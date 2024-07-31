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

BOARD_BOOT_HEADER_VERSION := 4

# Kernel - prefer version 6.1 by default for trout
TARGET_KERNEL_USE ?= 6.1

# Select the prebuilt trout kernel if 5.10 or 5.4 is in use
TROUT_KERNEL_DIR ?= $(wildcard device/google/trout-kernel/$(TARGET_KERNEL_USE)-arm64)

# The trout kernel is provided as source to AOSP,
# and thus we cannot rely on it existing outside of Google-internal builds. Make sure not to try
# and include a missing kernel image.
ifndef TARGET_KERNEL_PATH
TARGET_KERNEL_PATH := vendor/nxp-opensource/imx_virt_prebuilts/kernel/prebuilts/Image
KERNEL_MODULES_PATH := vendor/nxp-opensource/imx_virt_prebuilts/kernel/prebuilts/
SYSTEM_DLKM_SRC := vendor/nxp-opensource/imx_virt_prebuilts/kernel/prebuilts/
endif

TARGET_BOARD_PLATFORM := vsoc_arm64
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a53

ifneq ($(TROUT_KERNEL_DIR),)
KERNEL_MODULES_PATH ?= $(TROUT_KERNEL_DIR)
TARGET_KERNEL_PATH ?= $(TROUT_KERNEL_DIR)/Image

# For local builds of the android12-5.10 kernel, this directory doesn't exist.
# The system_dlkm partition won't have any kernel modules in it, which matches
# how this kernel was originally used.
#
# For prebuilts of the android12-5.10 kernel, the result is the same.
#
# For local builds of the android14-6.1 kernel and later, this directory should
# be created by extracting the system_dlkm_staging_archive.tar.gz file in the
# build directory of the kernel before building the android image.
#
# For prebuilts of the android14-6.1 kernel and later, TROUT_KERNEL_DIR should
# not be specified, in which case it will follow whatever the upstream
# cuttlefish device specifies.
SYSTEM_DLKM_SRC ?= $(TROUT_KERNEL_DIR)/flatten/lib/modules
endif

# The list of modules strictly/only required either to reach second stage
# init, OR for recovery. Do not use this list to workaround second stage
# issues.
RAMDISK_KERNEL_MODULES ?= \
    failover.ko \
    nd_virtio.ko \
    net_failover.ko \
    virtio_blk.ko \
    virtio_console.ko \
    virtio_dma_buf.ko \
    virtio-gpu.ko \
    virtio_input.ko \
    virtio_net.ko \
    virtio_mmio.ko \
    virtio_pci.ko \
    virtio-rng.ko \
    vmw_vsock_virtio_transport.ko \

-include device/google/trout/shared/BoardConfig.mk

AUDIOSERVER_MULTILIB := first

HOST_CROSS_OS := linux_bionic
HOST_CROSS_ARCH := arm64
HOST_CROSS_2ND_ARCH :=

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

# Prevent mac80211_hwsim from simulating any radios
BOARD_KERNEL_CMDLINE += mac80211_hwsim.radios=0

BOARD_KERNEL_CMDLINE += androidboot.boot_device=51712
