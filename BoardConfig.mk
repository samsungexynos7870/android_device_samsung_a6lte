#
# Copyright (C) 2019 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DEVICE_PATH := device/samsung/a6lte

# Audio

# audio type guard
TARGET_BOARD_HAS_TFA_SEC_AUDIO_HAL := false
TARGET_BOARD_HAS_SEC_AUDIO_HAL := false

ifeq ($(TARGET_BOARD_HAS_TFA_SEC_AUDIO_HAL),true)
TARGET_BOARD_HAS_TFA_AMP := true
endif

TARGET_AUDIOHAL_VARIANT := samsung-exynos7870

# secril
TARGET_SECRIL_VARIANT := samsung-exynos7870

# Audiohal (no smart amp HAL, CS35L40 is driven in-kernel)
BOARD_USE_SPKAMP := false

# Display
TARGET_SCREEN_DENSITY := 320

# build fixes
BUILD_BROKEN_CLANG_PROPERTY := true
BUILD_BROKEN_CLANG_CFLAGS := true
BUILD_BROKEN_CLANG_ASFLAGS := true

# Assert
TARGET_OTA_ASSERT_DEVICE := a6lte,a6ltexx,a6ltextc,a6lteub

# Bluetooth
BOARD_HAVE_BLUETOOTH := true

# FM Radio
TARGET_BOARD_HAS_BRCM_FM := true

SOONG_CONFIG_NAMESPACES += libfmjni
SOONG_CONFIG_libfmjni += vendor
SOONG_CONFIG_libfmjni_vendor := brcm

# Kernel
TARGET_KERNEL_CONFIG := exynos7870-a6lte_defconfig

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/configs/manifest.xml

# misc
BUILD_BROKEN_VINTF_PRODUCT_COPY_FILES := true

# Init
$(call soong_config_set,libinit,vendor_init_lib,//$(DEVICE_PATH):libinit_a6lte)

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := $(DEVICE_PATH)/releasetools

# CAMERA
BOARD_BACK_CAMERA_ROTATION := 90
BOARD_FRONT_CAMERA_ROTATION := 270
BOARD_BACK_CAMERA_SENSOR := SENSOR_NAME_S5K2P6
BOARD_FRONT_CAMERA_SENSOR := SENSOR_NAME_S5K2P6
BOARD_SECURE_CAMERA_SENSOR := SENSOR_NAME_NOTHING
BOARD_SECURE_CAMERA_ROTATION := 0

BOARD_SECURE_CAMERA_SUPPORT := true
BOARD_CAMERA_SAMSUNG_TN_FEATURE := true
BOARD_CAMERA_HAL3_FEATURE := true

# RIL
SIM_COUNT := 2

# A/B
AB_OTA_UPDATER := false

## USB
$(call soong_config_set,samsungUsbGadgetVars,gadget_name,13600000.dwc3)

# Shims
TARGET_LD_SHIM_LIBS += \
    /vendor/lib/libbauthserver.so|/vendor/lib/libbauthtzcommon_shim.so

# Sepolicy
BOARD_SEPOLICY_DIRS := $(DEVICE_PATH)/sepolicy

# Wifi
BOARD_WLAN_DEVICE                := bcmdhd
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_bcmdhd
WIFI_DRIVER_FW_PATH_PARAM        := "/sys/module/dhd/parameters/firmware_path"
WIFI_DRIVER_NVRAM_PATH_PARAM     := "/sys/module/dhd/parameters/nvram_path"
WIFI_DRIVER_NVRAM_PATH           := "/vendor/etc/wifi/nvram.txt_c5"
WIFI_DRIVER_FW_PATH_STA          := "/vendor/etc/wifi/bcmdhd_sta.bin_c5"
WIFI_DRIVER_FW_PATH_AP           := "/vendor/etc/wifi/bcmdhd_apsta.bin_c5"
WIFI_BAND                        := 802_11_ABG
WIFI_AVOID_IFACE_RESET_MAC_CHANGE := true
WIFI_HIDL_UNIFIED_SUPPLICANT_SERVICE_RC_ENTRY := true

# inherit from common
TARGET_USES_DEVICE_TOUCH_AIDL := true
-include device/samsung/universal7870-common/BoardConfigCommon.mk

# BoardConfigCommon assigns generic 0/1 camera sensors after this file is
# parsed, so restore the real a6lte sensor names here.
BOARD_BACK_CAMERA_SENSOR := SENSOR_NAME_S5K2P6
BOARD_FRONT_CAMERA_SENSOR := SENSOR_NAME_S5K2P6

# inherit from the proprietary version
-include vendor/samsung/a6lte/BoardConfigVendor.mk

# Samsung's fingerprint/bauth vendor blobs fail the strict prebuilt ELF checks
# (missing DT_NEEDED entries and unresolved libgcc helpers); skip that validation.
BUILD_BROKEN_PREBUILT_ELF_FILES := true
