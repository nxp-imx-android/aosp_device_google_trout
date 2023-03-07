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

#include "DefaultVehicleHal.h"
#include "GRPCVehicleHardware.h"
#include "vsockinfo.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <memory>
#include <utility>

using ::android::hardware::automotive::vehicle::DefaultVehicleHal;
using ::android::hardware::automotive::vehicle::virtualization::GRPCVehicleHardware;

int main(int /* argc */, char* /* argv */[]) {
    LOG(INFO) << "Starting thread pool...";
    if (!ABinderProcess_setThreadPoolMaxThreadCount(4)) {
        LOG(ERROR) << "Failed to set thread pool max thread count.";
        return 1;
    }
    ABinderProcess_startThreadPool();

    auto vsock = android::hardware::automotive::utils::VsockConnectionInfo::fromRoPropertyStore(
            {
                    "ro.boot.vendor.vehiclehal.server.cid",
                    "ro.vendor.vehiclehal.server.cid",
            },
            {
                    "ro.boot.vendor.vehiclehal.server.port",
                    "ro.vendor.vehiclehal.server.port",
            });
    CHECK(vsock.has_value()) << "Cannot read VHAL server address.";

    LOG(INFO) << "Connecting to vsock server at " << vsock->str();

    auto hardware = std::make_unique<GRPCVehicleHardware>(vsock->str());
    auto vhal = ::ndk::SharedRefBase::make<DefaultVehicleHal>(std::move(hardware));

    LOG(INFO) << "Registering as service...";
    binder_exception_t err = AServiceManager_addService(
            vhal->asBinder().get(), "android.hardware.automotive.vehicle.IVehicle/default");
    if (err != EX_NONE) {
        LOG(ERROR) << "Failed to register android.hardware.automotive.vehicle service, exception: "
                   << err << ".";
        return 1;
    }

    LOG(INFO) << "Vehicle Service Ready.";

    ABinderProcess_joinThreadPool();

    LOG(INFO) << "Vehicle Service Exiting.";

    return 0;
}
