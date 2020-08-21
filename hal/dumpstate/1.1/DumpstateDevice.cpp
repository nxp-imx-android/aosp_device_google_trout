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

#include "DumpstateDevice.h"

#include <DumpstateUtil.h>
#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

#include <string>

using android::os::dumpstate::CommandOptions;
using android::os::dumpstate::DumpFileToFd;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::literals::chrono_literals::operator""s;

static constexpr const char* VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY =
        "persist.vendor.verbose_logging_enabled";

static constexpr const char* VENDOR_HELPER_SYSTEM_LOG_LOC_PROPERTY =
        "ro.vendor.helpersystem.log_loc";

namespace android::hardware::dumpstate::V1_1::implementation {

static void dumpHelperSystem(int /*textFd*/, int /*binFd*/) {
    std::string helperSystemLogDir =
            android::base::GetProperty(VENDOR_HELPER_SYSTEM_LOG_LOC_PROPERTY, "");
    if (helperSystemLogDir.empty()) {
        LOG(ERROR) << "Helper system log location '" << VENDOR_HELPER_SYSTEM_LOG_LOC_PROPERTY
                   << "' not set";
        return;
    }

    LOG(ERROR) << "this build does not support actually getting any work done, sorry!";
}

// Methods from ::android::hardware::dumpstate::V1_0::IDumpstateDevice follow.
Return<void> DumpstateDevice::dumpstateBoard(const hidl_handle& handle) {
    // Ignore return value, just return an empty status.
    dumpstateBoard_1_1(handle, DumpstateMode::DEFAULT, 30 * 1000 /* timeoutMillis */);
    return Void();
}

// Methods from ::android::hardware::dumpstate::V1_1::IDumpstateDevice follow.
Return<DumpstateStatus> DumpstateDevice::dumpstateBoard_1_1(const hidl_handle& handle,
                                                            const DumpstateMode /* mode */,
                                                            const uint64_t /* timeoutMillis */) {
    if (handle == nullptr || handle->numFds < 1) {
        LOG(ERROR) << "No FDs";
        return DumpstateStatus::ILLEGAL_ARGUMENT;
    }

    const int textFd = handle->data[0];
    const int binFd = handle->numFds >= 2 ? handle->data[1] : -1;

    dumpHelperSystem(textFd, binFd);

    return DumpstateStatus::OK;
}

Return<void> DumpstateDevice::setVerboseLoggingEnabled(const bool enable) {
    android::base::SetProperty(VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY, enable ? "true" : "false");
    return Void();
}

Return<bool> DumpstateDevice::getVerboseLoggingEnabled() {
    return android::base::GetBoolProperty(VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY, false);
}

}  // namespace android::hardware::dumpstate::V1_1::implementation
