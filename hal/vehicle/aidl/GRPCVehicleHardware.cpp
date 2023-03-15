/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <GRPCVehicleHardware.h>

#include <android-base/logging.h>
#include <grpc++/grpc++.h>

#include <cstdlib>
#include <utility>

namespace android::hardware::automotive::vehicle::virtualization {

static std::shared_ptr<::grpc::ChannelCredentials> getChannelCredentials() {
    // TODO(chenhaosjtuacm): get secured credentials here
    return ::grpc::InsecureChannelCredentials();
}

GRPCVehicleHardware::GRPCVehicleHardware(std::string service_addr)
    : mServiceAddr(std::move(service_addr)),
      mGrpcChannel(::grpc::CreateChannel(mServiceAddr, getChannelCredentials())),
      mGrpcStub(proto::VehicleServer::NewStub(mGrpcChannel)) {}

GRPCVehicleHardware::~GRPCVehicleHardware() {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
}

std::vector<aidl::android::hardware::automotive::vehicle::VehiclePropConfig>
GRPCVehicleHardware::getAllPropertyConfigs() const {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
    std::abort();
}

aidl::android::hardware::automotive::vehicle::StatusCode GRPCVehicleHardware::setValues(
        std::shared_ptr<const SetValuesCallback> callback,
        const std::vector<aidl::android::hardware::automotive::vehicle::SetValueRequest>&
                requests) {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
    std::abort();
}

aidl::android::hardware::automotive::vehicle::StatusCode GRPCVehicleHardware::getValues(
        std::shared_ptr<const GetValuesCallback> callback,
        const std::vector<aidl::android::hardware::automotive::vehicle::GetValueRequest>& requests)
        const {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
    std::abort();
}

void GRPCVehicleHardware::registerOnPropertyChangeEvent(
        std::unique_ptr<const PropertyChangeCallback> callback) {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
}

void GRPCVehicleHardware::registerOnPropertySetErrorEvent(
        std::unique_ptr<const PropertySetErrorCallback> callback) {
    LOG(FATAL) << __func__ << ": Not implemented yet.";
}

DumpResult GRPCVehicleHardware::dump(const std::vector<std::string>& /* options */) {
    return {};
}

aidl::android::hardware::automotive::vehicle::StatusCode GRPCVehicleHardware::checkHealth() {
    return aidl::android::hardware::automotive::vehicle::StatusCode::OK;
}

aidl::android::hardware::automotive::vehicle::StatusCode GRPCVehicleHardware::updateSampleRate(
        int32_t /* propId */, int32_t /* areaId */, float /* sampleRate */) {
    return aidl::android::hardware::automotive::vehicle::StatusCode::OK;
}

}  // namespace android::hardware::automotive::vehicle::virtualization
