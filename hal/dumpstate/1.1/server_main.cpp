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
#include "ServiceSupplier.h"

#include <getopt.h>

#include <iostream>
#include <string>

static ServiceDescriptor kDmesgService("dmesg", "/bin/dmesg -kuPT");

static ServiceDescriptor SystemdService(const std::string& name) {
    return ServiceDescriptor{name, std::string("/bin/journalctl --no-pager -t ") + name};
}

// clang-format off
static const std::vector<ServiceDescriptor> kAvailableServices {
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

class CoqosLvSystemdServices : public ServiceSupplier {
  public:
    std::optional<ServiceDescriptor> GetSystemLogsService() const override { return kDmesgService; }

    std::vector<ServiceDescriptor> GetServices() const override { return kAvailableServices; }

    void dump(std::ostream& os) {
        if (auto dmesg = GetSystemLogsService()) {
            os << "system logs service: [name=" << dmesg->name() << ", command=" << dmesg->command()
               << "]" << std::endl;
        }
        for (auto svc : GetServices()) {
            os << "service " << svc.name() << " runs command " << svc.command() << std::endl;
        }
    }
};

int main(int argc, char** argv) {
    std::string serverAddr;

    // unique values to identify the options
    constexpr int OPT_SERVER_ADDR = 1001;

    struct option longOptions[] = {
            {"server_addr", 1, 0, OPT_SERVER_ADDR},
            {},
    };

    int optValue;
    while ((optValue = getopt_long_only(argc, argv, ":", longOptions, 0)) != -1) {
        switch (optValue) {
            case OPT_SERVER_ADDR:
                serverAddr = optarg;
                break;
            default:
                // ignore other options
                break;
        }
    }

    if (serverAddr.empty()) {
        std::cerr << "Dumpstate server addreess is missing" << std::endl;
        return 1;
    } else {
        std::cerr << "Dumpstate server addreess: " << serverAddr << std::endl;
    }

    CoqosLvSystemdServices servicesSupplier;
    servicesSupplier.dump(std::cerr);

    DumpstateGrpcServer server(serverAddr, servicesSupplier);
    server.Start();
    return 0;
}
