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
ifndef TARGET_BOOTLOADER_BOARD_NAME
TARGET_BOOTLOADER_BOARD_NAME := trout
endif

BOARD_BOOT_HEADER_VERSION := 3

-include device/google/cuttlefish/shared/BoardConfig.mk

TARGET_BOARD_PLATFORM := vsoc_arm64
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a53

AUDIOSERVER_MULTILIB := first
BOARD_VENDOR_RAMDISK_KERNEL_MODULES += $(wildcard kernel/prebuilts/common-modules/virtual-device/$(TARGET_KERNEL_USE)/arm64/*.ko)

HOST_CROSS_OS := linux_bionic
HOST_CROSS_ARCH := arm64
HOST_CROSS_2ND_ARCH :=

# Reset this variable to re-enable ramdisk.
BOARD_INIT_BOOT_IMAGE_PARTITION_SIZE :=

# Android Bluetooth stack configuration
LOCAL_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR ?= device/google/trout/product_files/bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(LOCAL_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR)

# Turn off AVB so that trout can boot
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS += --flag 2
BOARD_KERNEL_CMDLINE += androidboot.verifiedbootstate=orange

# Set SELinux to permissive mode for trout
BOARD_KERNEL_CMDLINE += androidboot.selinux=permissive
BOARD_KERNEL_CMDLINE += enforcing=0

# Declare trout as a Cuttlefish HW
BOARD_KERNEL_CMDLINE += androidboot.hardware=cutf_cvm
BOARD_KERNEL_CMDLINE += androidboot.serialno=CUTTLEFISHCVD01
BOARD_KERNEL_CMDLINE += androidboot.cf_devcfg=1

# Set GPU properties
BOARD_KERNEL_CMDLINE += androidboot.cpuvulkan.version=0
BOARD_KERNEL_CMDLINE += androidboot.hardware.gralloc=minigbm
BOARD_KERNEL_CMDLINE += androidboot.hardware.hwcomposer=ranchu
BOARD_KERNEL_CMDLINE += androidboot.hardware.egl=mesa
BOARD_KERNEL_CMDLINE += androidboot.hardware.hwcomposer.mode=client
BOARD_KERNEL_CMDLINE += androidboot.hardware.hwcomposer.display_finder_mode=drm
BOARD_KERNEL_CMDLINE += androidboot.lcd_density=160

# Add WiFi configuration for VirtWifi network
BOARD_KERNEL_CMDLINE += androidboot.wifi_mac_prefix=5554

# Add default fstab settings
BOARD_KERNEL_CMDLINE += androidboot.fstab_name=fstab androidboot.fstab_suffix=trout
