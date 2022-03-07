#
# Copyright 2020 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Enable sparse on all filesystem images
TARGET_USERIMAGES_SPARSE_EROFS_DISABLED ?= false
TARGET_USERIMAGES_SPARSE_EXT_DISABLED ?= false
TARGET_USERIMAGES_SPARSE_F2FS_DISABLED ?= false

#
# arm64 target for Trout
#

BOARD_BOOT_HEADER_VERSION := 3

-include device/google/cuttlefish/vsoc_arm64/BoardConfig.mk

# Android Bluetooth stack configuration
LOCAL_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR ?= device/google/trout/product_files/bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(LOCAL_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR)
