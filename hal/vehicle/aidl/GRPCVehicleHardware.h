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

#pragma once

#include <IVehicleHardware.h>
#include <VehicleHalTypes.h>
#include <VehicleUtils.h>
#include <android-base/result.h>

#include "VehicleServer.grpc.pb.h"
#include "VehicleServer.pb.h"

#include <memory>
#include <string>
#include <vector>

namespace android::hardware::automotive::vehicle::virtualization {

class GRPCVehicleHardware : public IVehicleHardware {
  public:
    explicit GRPCVehicleHardware(std::string service_addr);

    ~GRPCVehicleHardware();

    // Get all the property configs.
    std::vector<aidl::android::hardware::automotive::vehicle::VehiclePropConfig>
    getAllPropertyConfigs() const override;

    // Set property values asynchronously. Server could return before the property set requests
    // are sent to vehicle bus or before property set confirmation is received. The callback is
    // safe to be called after the function returns and is safe to be called in a different thread.
    aidl::android::hardware::automotive::vehicle::StatusCode setValues(
            std::shared_ptr<const SetValuesCallback> callback,
            const std::vector<aidl::android::hardware::automotive::vehicle::SetValueRequest>&
                    requests) override;

    // Get property values asynchronously. Server could return before the property values are ready.
    // The callback is safe to be called after the function returns and is safe to be called in a
    // different thread.
    aidl::android::hardware::automotive::vehicle::StatusCode getValues(
            std::shared_ptr<const GetValuesCallback> callback,
            const std::vector<aidl::android::hardware::automotive::vehicle::GetValueRequest>&
                    requests) const override;

    // Dump debug information in the server.
    DumpResult dump(const std::vector<std::string>& options) override;

    // Check whether the system is healthy, return {@code StatusCode::OK} for healthy.
    aidl::android::hardware::automotive::vehicle::StatusCode checkHealth() override;

    // Register a callback that would be called when there is a property change event from vehicle.
    void registerOnPropertyChangeEvent(
            std::unique_ptr<const PropertyChangeCallback> callback) override;

    // Register a callback that would be called when there is a property set error event from
    // vehicle.
    void registerOnPropertySetErrorEvent(
            std::unique_ptr<const PropertySetErrorCallback> callback) override;

    // Update the sample rate for the [propId, areaId] pair.
    aidl::android::hardware::automotive::vehicle::StatusCode updateSampleRate(
            int32_t propId, int32_t areaId, float sampleRate) override;

  private:
    std::string mServiceAddr;
    std::shared_ptr<::grpc::Channel> mGrpcChannel;
    std::unique_ptr<proto::VehicleServer::Stub> mGrpcStub;
};

}  // namespace android::hardware::automotive::vehicle::virtualization
