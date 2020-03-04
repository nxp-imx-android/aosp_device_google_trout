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

# Dumpstate HAL
LOCAL_DUMPSTATE_PRODUCT_PACKAGE := android.hardware.dumpstate@1.1-service.trout

DEVICE_MANIFEST_FILE += device/google/trout/manifest.xml

PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.vehiclehal.server.cid=2 \
	ro.vendor.vehiclehal.server.port=9210 \
	ro.vendor.helpersystem.log_loc=/data/bugreport_test \

# Disable Vulkan feature flag as it is not supported on trout
TARGET_VULKAN_SUPPORT := false

# Sensor HAL
LOCAL_SENSOR_PRODUCT_PACKAGE := android.hardware.sensors@2.0-service.multihal
LOCAL_SENSOR_PRODUCT_PACKAGE += android.hardware.sensors@2.0-service.multihal.rc
LOCAL_SENSOR_PRODUCT_PACKAGE += android.hardware.sensors@2.0-Google-IIO-Subhal

$(call inherit-product, device/google/cuttlefish/vsoc_arm64/auto/aosp_cf.mk)

PRODUCT_COPY_FILES += \
    device/google/trout/product_files/odm/ueventd.rc:$(TARGET_COPY_OUT_ODM)/ueventd.rc \

PRODUCT_COPY_FILES += \
    device/google/trout/product_files/vendor/etc/input-port-associations.xml:$(TARGET_COPY_OUT_VENDOR)/etc/input-port-associations.xml \

BOARD_SEPOLICY_DIRS += device/google/trout/sepolicy/vendor/google

PRODUCT_PROPERTY_OVERRIDES += \
	ro.hardware.type=automotive \

PRODUCT_COPY_FILES += device/google/trout/product_files/vendor/etc/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf

PRODUCT_NAME := aosp_trout_arm64
PRODUCT_DEVICE := vsoc_arm64
PRODUCT_MODEL := arm64 trout
