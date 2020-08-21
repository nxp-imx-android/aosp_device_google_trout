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
#pragma once

#include <android/hardware/automotive/vehicle/2.0/types.h>

namespace android::hardware::automotive::vehicle::V2_0::impl {

class VirtualizedGarageModeHandler {
  public:
    using heartbeat_sender_t = std::function<bool()>;

    virtual ~VirtualizedGarageModeHandler() = default;

    virtual void HandlePowerStateChange(const VehiclePropValue& value) = 0;
};

std::unique_ptr<VirtualizedGarageModeHandler> makeVirtualizedGarageModeHandler(
        VirtualizedGarageModeHandler::heartbeat_sender_t&& heartbeatSender);

}  // namespace android::hardware::automotive::vehicle::V2_0::impl
