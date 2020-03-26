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

#include "GarageModeHandler.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <android-base/logging.h>

#include "vhal_v2_0/DefaultConfig.h"

namespace android::hardware::automotive::vehicle::V2_0::impl {

using std::chrono::duration_cast;
using std::chrono::seconds;
using std::literals::chrono_literals::operator""s;

class VirtualizedGarageModeHandlerImpl : public VirtualizedGarageModeHandler {
  public:
    explicit VirtualizedGarageModeHandlerImpl(heartbeat_sender_t&& heartbeatSender)
        : mHeartbeatSender(std::move(heartbeatSender)),
          mThread(std::bind(&VirtualizedGarageModeHandlerImpl::HeartbeatSenderLoop, this)) {}

    virtual ~VirtualizedGarageModeHandlerImpl() {
        mShuttingDownFlag.store(true);
        mHeartbeatCV.notify_all();
        if (mThread.joinable()) {
            mThread.join();
        }
    }

    void HandlePowerStateChange(const VehiclePropValue& value) override;

  private:
    void HeartbeatSenderLoop();

    void EnteringGarageMode();

    void ExitingGarageMode();

    heartbeat_sender_t mHeartbeatSender;
    std::atomic<bool> mHeartbeatFlag{false};
    std::atomic<bool> mShuttingDownFlag{false};
    std::thread mThread;
    std::condition_variable mHeartbeatCV;
    std::mutex mHeartbeatMutex;
};

void VirtualizedGarageModeHandlerImpl::HandlePowerStateChange(const VehiclePropValue& value) {
    auto powerStateValue = value.value.int32Values[0];
    LOG(INFO) << __func__ << ": change the power state to "
              << toString<VehicleApPowerStateReport>(powerStateValue);

    switch (powerStateValue) {
        case toInt(VehicleApPowerStateReport::SHUTDOWN_PREPARE):
            EnteringGarageMode();
            break;
        case toInt(VehicleApPowerStateReport::DEEP_SLEEP_ENTRY):
        case toInt(VehicleApPowerStateReport::SHUTDOWN_CANCELLED):
        case toInt(VehicleApPowerStateReport::SHUTDOWN_START):
            ExitingGarageMode();
            break;
        default:
            break;
    }
}

void VirtualizedGarageModeHandlerImpl::EnteringGarageMode() {
    LOG(INFO) << __func__ << ": start sending garage mode heartbeats.";
    mHeartbeatFlag.store(true);
    mHeartbeatCV.notify_all();
}

void VirtualizedGarageModeHandlerImpl::ExitingGarageMode() {
    LOG(INFO) << __func__ << ": stop sending garage mode heartbeats.";
    mHeartbeatFlag.store(false);
}

void VirtualizedGarageModeHandlerImpl::HeartbeatSenderLoop() {
    constexpr auto kHeartbeatInterval = 1s;
    LOG(DEBUG) << "Garage mode heartbeat sender launched, "
               << "heartbeat interval " << duration_cast<seconds>(kHeartbeatInterval).count()
               << " s";
    while (!mShuttingDownFlag.load()) {
        if (!mHeartbeatFlag.load()) {
            std::unique_lock<std::mutex> heartbeatLock(mHeartbeatMutex);
            mHeartbeatCV.wait(heartbeatLock, [this]() {
                return mHeartbeatFlag.load() || mShuttingDownFlag.load();
            });
        }

        if (!mHeartbeatSender()) {
            LOG(WARNING) << __func__ << ": failed to send heartbeat!";
        }

        std::this_thread::sleep_for(kHeartbeatInterval);
    }
    LOG(DEBUG) << "Garage mode heartbeat sender exited.";
}

std::unique_ptr<VirtualizedGarageModeHandler> makeVirtualizedGarageModeHandler(
        VirtualizedGarageModeHandler::heartbeat_sender_t&& heartbeatSender) {
    return std::make_unique<VirtualizedGarageModeHandlerImpl>(std::move(heartbeatSender));
}

}  // namespace android::hardware::automotive::vehicle::V2_0::impl
