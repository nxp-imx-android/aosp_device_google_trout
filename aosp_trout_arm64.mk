#
# Copyright (C) 2020 The Android Open Source Project
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

# Vehicle HAL
LOCAL_VHAL_PRODUCT_PACKAGE := android.hardware.automotive.vehicle@2.0-virtualization-service android.hardware.automotive.vehicle@2.0-virtualization-grpc-server
PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.vehiclehal.server.cid=2 \
	ro.vendor.vehiclehal.server.port=9210 \

# Disable Vulkan feature flag as it is not supported on trout
TARGET_VULKAN_SUPPORT := false

$(call inherit-product, device/google/cuttlefish/vsoc_arm64/auto/aosp_cf.mk)

PRODUCT_COPY_FILES += \
    device/google/trout/product_files/odm/ueventd.rc:$(TARGET_COPY_OUT_ODM)/ueventd.rc \

PRODUCT_COPY_FILES += \
    device/google/trout/product_files/vendor/etc/input-port-associations.xml:$(TARGET_COPY_OUT_VENDOR)/etc/input-port-associations.xml \

BOARD_SEPOLICY_DIRS += device/google/trout/sepolicy/vendor/google

PRODUCT_PROPERTY_OVERRIDES += \
	ro.hardware.type=automotive \

PRODUCT_NAME := aosp_trout_arm64
PRODUCT_DEVICE := vsoc_arm64
PRODUCT_MODEL := arm64 trout
