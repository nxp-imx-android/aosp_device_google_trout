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

#include <filesystem>
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

static void dumpDirAsText(int textFd, const std::filesystem::path& dirToDump) {
    for (const auto& fileEntry : std::filesystem::recursive_directory_iterator(dirToDump)) {
        if (!fileEntry.is_regular_file()) {
            continue;
        }

        DumpFileToFd(textFd, "Helper System Log", fileEntry.path());
    }
}

static void tryDumpDirAsTar(int textFd, int binFd, const std::filesystem::path& dirToDump) {
    if (!std::filesystem::is_directory(dirToDump)) {
        LOG(ERROR) << "'" << dirToDump << "'"
                   << " is not a valid directory to dump";
        return;
    }

    if (binFd < 0) {
        LOG(WARNING) << "No binary dumped file, fallback to text mode";
        return dumpDirAsText(textFd, dirToDump);
    }

    TemporaryFile tempTarFile;
    constexpr auto kTarTimeout = 20s;

    RunCommandToFd(
            textFd, "TAR LOG", {"/vendor/bin/tar", "cvf", tempTarFile.path, dirToDump.c_str()},
            CommandOptions::WithTimeout(duration_cast<seconds>(kTarTimeout).count()).Build());

    std::vector<uint8_t> buffer(65536);
    while (true) {
        ssize_t bytes_read = TEMP_FAILURE_RETRY(read(tempTarFile.fd, buffer.data(), buffer.size()));

        if (bytes_read == 0) {
            break;
        } else if (bytes_read < 0) {
            LOG(DEBUG) << "read temporary tar file(" << tempTarFile.path
                       << "): " << strerror(errno);
            break;
        }

        ssize_t result = TEMP_FAILURE_RETRY(write(binFd, buffer.data(), bytes_read));

        if (result != bytes_read) {
            LOG(DEBUG) << "Failed to write " << bytes_read
                       << " bytes, actually written: " << result;
            break;
        }
    }
}

static void dumpHelperSystem(int textFd, int binFd) {
    std::string helperSystemLogDir =
            android::base::GetProperty(VENDOR_HELPER_SYSTEM_LOG_LOC_PROPERTY, "");
    if (helperSystemLogDir.empty()) {
        LOG(ERROR) << "Helper system log location '" << VENDOR_HELPER_SYSTEM_LOG_LOC_PROPERTY
                   << "' not set";
        return;
    }

    tryDumpDirAsTar(textFd, binFd, helperSystemLogDir);
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
