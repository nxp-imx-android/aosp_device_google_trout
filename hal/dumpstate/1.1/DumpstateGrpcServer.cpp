/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DumpstateGrpcServer.h"
#include "ServiceDescriptor.h"

#include <array>
#include <iostream>
#include <unordered_set>

#include <grpc++/grpc++.h>

struct GrpcServiceOutputConsumer : public ServiceDescriptor::OutputConsumer {
    using Dest = ::grpc::ServerWriter<dumpstate_proto::DumpstateBuffer>*;

    explicit GrpcServiceOutputConsumer(Dest s) : stream(s) {}

    void Write(char* ptr, size_t len) override {
        dumpstate_proto::DumpstateBuffer dumpstateBuffer;
        dumpstateBuffer.set_buffer(ptr, len);
        stream->Write(dumpstateBuffer);
    }

    Dest stream;
};

static ServiceDescriptor kDmesgService("dmesg", "/bin/dmesg -kuPT");

static std::pair<std::string, ServiceDescriptor> SystemdService(const std::string& name) {
    return {name, ServiceDescriptor{name, std::string("/bin/journalctl --no-pager -t ") + name}};
}

// clang-format off
static const std::unordered_map<std::string, ServiceDescriptor> kAvailableServices{
        SystemdService("coqos-virtio-blk"),
        SystemdService("coqos-virtio-net"),
        SystemdService("coqos-virtio-video"),
        SystemdService("coqos-virtio-console"),
        SystemdService("coqos-virtio-rng"),
        SystemdService("coqos-virtio-vsock"),
        SystemdService("coqos-virtio-gpu-virgl"),
        SystemdService("coqos-virtio-scmi"),
        SystemdService("coqos-virtio-input"),
        SystemdService("coqos-virtio-snd"),
        SystemdService("dumpstate_grpc_server"),
        SystemdService("systemd"),
        SystemdService("vehicle_hal_grpc_server"),
};
// clang-format on

static std::shared_ptr<::grpc::ServerCredentials> getServerCredentials() {
    // TODO(chenhaosjtuacm): get secured credentials here
    return ::grpc::InsecureServerCredentials();
}

grpc::Status DumpstateGrpcServer::GetSystemLogs(
        ::grpc::ServerContext*, const ::google::protobuf::Empty*,
        ::grpc::ServerWriter<dumpstate_proto::DumpstateBuffer>* stream) {
    GrpcServiceOutputConsumer consumer(stream);

    const auto ok = kDmesgService.GetOutput(&consumer);
    if (ok == std::nullopt)
        return ::grpc::Status::OK;
    else
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, *ok);
}

grpc::Status DumpstateGrpcServer::GetAvailableServices(
        ::grpc::ServerContext*, const ::google::protobuf::Empty*,
        dumpstate_proto::ServiceNameList* serviceList) {
    static const dumpstate_proto::ServiceNameList kProtoAvailableServices = []() {
        dumpstate_proto::ServiceNameList serviceNameList;
        for (auto& svc : kAvailableServices) {
            if (svc.second.IsAvailable()) serviceNameList.add_service_names(svc.first);
        }
        return serviceNameList;
    }();

    *serviceList = kProtoAvailableServices;
    return ::grpc::Status::OK;
}

grpc::Status DumpstateGrpcServer::GetServiceLogs(
        ::grpc::ServerContext*, const dumpstate_proto::ServiceLogRequest* request,
        ::grpc::ServerWriter<dumpstate_proto::DumpstateBuffer>* stream) {
    const auto& serviceName = request->service_name();
    if (serviceName.empty()) {
        return ::grpc::Status::OK;
    }
    auto iter = kAvailableServices.find(serviceName);
    if (iter == kAvailableServices.end()) {
        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                              std::string("Bad service name: ") + serviceName);
    }
    GrpcServiceOutputConsumer consumer(stream);
    const auto ok = iter->second.GetOutput(&consumer);
    if (ok == std::nullopt)
        return ::grpc::Status::OK;
    else
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, *ok);
}

void DumpstateGrpcServer::Start() {
    ::grpc::ServerBuilder builder;
    builder.RegisterService(this);
    builder.AddListeningPort(mServiceAddr, getServerCredentials());
    std::unique_ptr<::grpc::Server> server(builder.BuildAndStart());

    if (!server) {
        std::cerr << __func__ << ": failed to create the GRPC server, "
                  << "please make sure the configuration and permissions are correct" << std::endl;
        std::abort();
    }

    server->Wait();
}
