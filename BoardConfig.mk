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
TARGET_BOARD_HAS_SEC_AUDIO_HAL := true

ifeq ($(TARGET_BOARD_HAS_TFA_SEC_AUDIO_HAL),true)
TARGET_BOARD_HAS_TFA_AMP := true
endif

# sec audio hal
TARGET_BOARD_HAS_EXYNOS7870_SEC_AUDIOHAL := false

# TARGET_AUDIOHAL_VARIANT := samsung-exynos7870

# Audiohal
BOARD_USE_SPKAMP := false

# TFA
# TARGET_BOARD_TFA_MODEL := 9890

# LIBHWJPEG
TARGET_USES_EXYNOS7870_LIBHWJPEG_O := true

# Assert
TARGET_OTA_ASSERT_DEVICE := a6lte,a6ltexx,a6ltextc,a6lteub

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth
BOARD_CUSTOM_BT_CONFIG := $(DEVICE_PATH)/bluetooth/libbt_vndcfg.txt
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_HAVE_SAMSUNG_BLUETOOTH := true

# Kernel
TARGET_KERNEL_CONFIG := exynos7870-a6lte_defconfig

# Init
TARGET_INIT_VENDOR_LIB := //$(DEVICE_PATH):libinit_a6lte
TARGET_RECOVERY_DEVICE_MODULES := libinit_a6lte

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/configs/manifest.xml

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := $(DEVICE_PATH)/releasetools

# CAMERA
BOARD_BACK_CAMERA_ROTATION := 90
BOARD_FRONT_CAMERA_ROTATION := 270
BOARD_BACK_CAMERA_SENSOR := SENSOR_NAME_IMX258
BOARD_FRONT_CAMERA_SENSOR := SENSOR_NAME_3M3
BOARD_SECURE_CAMERA_SENSOR := SENSOR_NAME_NOTHING
BOARD_SECURE_CAMERA_ROTATION := 0

BOARD_SECURE_CAMERA_SUPPORT := true
BOARD_CAMERA_DUAL_SUPPORT := false
BOARD_CAMERA2_API_SUPPORT := true
BOARD_CAMERA_SAMSUNG_TN_FEATURE := true
BOARD_CAMERA_HAL3_FEATURE := true

# RIL
SIM_COUNT := 2

# Shims
TARGET_LD_SHIM_LIBS += \
    /vendor/lib/libbauthserver.so|/vendor/lib/libbauthtzcommon_shim.so

# Wifi
BOARD_WLAN_DEVICE                := bcmdhd
WPA_SUPPLICANT_VERSION           := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER      := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
WPA_SUPPLICANT_USE_HIDL          := true
BOARD_HOSTAPD_DRIVER             := NL80211
BOARD_HOSTAPD_PRIVATE_LIB        := lib_driver_cmd_bcmdhd
WIFI_DRIVER_FW_PATH_PARAM        := "/sys/module/dhd/parameters/firmware_path"
WIFI_DRIVER_NVRAM_PATH_PARAM     := "/sys/module/dhd/parameters/nvram_path"
WIFI_DRIVER_NVRAM_PATH           := "/vendor/etc/wifi/nvram_net.txt"
WIFI_DRIVER_FW_PATH_STA          := "/vendor/etc/wifi/bcmdhd_sta.bin"
WIFI_DRIVER_FW_PATH_AP           := "/vendor/etc/wifi/bcmdhd_apsta.bin"
WIFI_BAND                        := 802_11_ABG

# inherit from common
-include device/samsung/universal7870-common/BoardConfigCommon.mk

# inherit from the proprietary version
-include vendor/samsung/a6lte/BoardConfigVendor.mk
