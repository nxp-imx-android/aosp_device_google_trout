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

#include "AudioControl.h"

#include <hidl/HidlTransportSupport.h>

namespace android::hardware::automotive::audiocontrol::V2_0::implementation {

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;

Return<sp<ICloseHandle>> AudioControl::registerFocusListener(const sp<IFocusListener>&) {
    sp<ICloseHandle> closeHandle(nullptr);
    return closeHandle;
}

Return<void> AudioControl::setBalanceTowardRight(float) {
    return Void();
}

Return<void> AudioControl::setFadeTowardFront(float) {
    return Void();
}

Return<void> AudioControl::onAudioFocusChange(hidl_bitfield<AudioUsage>, int,
                                              hidl_bitfield<AudioFocusChange>) {
    return Void();
}

Return<void> AudioControl::debug(const hidl_handle&, const hidl_vec<hidl_string>&) {
    return Void();
}

}  // namespace android::hardware::automotive::audiocontrol::V2_0::implementation
