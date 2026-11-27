#
# Copyright (C) 2019 The LineageOS Project
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

DEVICE_PATH := device/samsung/a6lte

# audio type guard
TARGET_DEVICE_HAS_TFA_SEC_AUDIO_HAL := false
TARGET_DEVICE_HAS_SEC_AUDIO_HAL := false
TARGET_DEVICE_HAS_OSS_AUDIO_HAL := true
TARGET_DEVICE_HAS_TFA_AMP := false

# radio type guard
TARGET_DEVICE_HAS_SEC_RIL := true

# gnss type guard
TARGET_DEVICE_HAS_SEC_GNSS := true

# prebuilt slsi
TARGET_DEVICE_HAS_SAMSUNG_SLSI_EXYNOS7870 := false

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    hardware/broadcom/libbt

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.ese.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml

# Custom mixer_paths
ifeq ($(TARGET_DEVICE_HAS_OSS_AUDIO_HAL),true)
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/audio/mixer_paths_oss_a600fn.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml
else
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/audio/mixer_gains.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_gains.xml \
    $(DEVICE_PATH)/configs/audio/mixer_paths.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths.xml
endif

# Launch Android API level
PRODUCT_SHIPPING_API_LEVEL := 26

# Bootanimation
TARGET_SCREEN_HEIGHT := 1480
TARGET_SCREEN_WIDTH := 720

# Graphics
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi
PRODUCT_AAPT_PREBUILT_DPI := xhdpi hdpi

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.3-service.a6lte

# Bluetooth
$(call soong_config_set,brcm_libbt,bdroid_buildcfg_include_dir,$(DEVICE_PATH)/bluetooth)
$(call soong_config_set,brcm_libbt,custom_bt_config,//$(DEVICE_PATH):vnd_a6lte.txt)
$(call soong_config_set,brcm_libbt,fm_hci,true)

PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl \
    android.hardware.bluetooth@1.0.vendor \
    android.hardware.bluetooth@1.0-service \
    android.hardware.bluetooth.audio-impl \
    android.hardware.bluetooth.audio@2.0-impl \
    audio.bluetooth.default \
    libbt-vendor

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/bluetooth/bt_vendor.conf:vendor/etc/bluetooth/bt_vendor.conf

# NFC
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec-vendor.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-sec-vendor.conf \
    $(DEVICE_PATH)/configs/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nci.conf \
    $(DEVICE_PATH)/configs/nfc/nfcee_access.xml:$(TARGET_COPY_OUT_VENDOR)/etc/nfcee_access.xml

# NFC
PRODUCT_PACKAGES += \
    libnfc-nci \
    libnfc_nci_jni \
    Tag \
    com.android.nfc_extras \
    android.hardware.nfc@1.2.vendor \
    android.hardware.nfc@1.2-service.samsung

# Overlays
DEVICE_PACKAGE_OVERLAYS += \
    $(DEVICE_PATH)/overlay

# Static biometric sensor RRO (fingerprint strength)
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/prebuilt/overlay/biometric-rro.apk:$(TARGET_COPY_OUT_PRODUCT)/overlay/biometric-rro.apk

# Power
# power-libperfmgr's HintManager aborts at boot if the hint file is missing.
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/power/powerhint.json:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.json

# Ramdisk
PRODUCT_PACKAGES += \
    mobicore.rc \
    wifi_device.rc

# Shims
PRODUCT_PACKAGES += \
    libbauthtzcommon_shim

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi-service \
    hostapd \
    libwifi-hal \
    libwpa_client \
    wificond \
    wpa_supplicant \
    wpa_supplicant.conf

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(DEVICE_PATH)/configs/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf

# FM radio
# Bluetooth owns the combo chip HCI/UART. FMRadio talks to the device-gated
# Broadcom FM endpoint in the Bluetooth process.
PRODUCT_PACKAGES += \
    FMRadio

# Properties
-include $(DEVICE_PATH)/vendor_prop.mk

# Inherit from common
$(call inherit-product, device/samsung/universal7870-common/device-common.mk)

$(call inherit-product-if-exists, vendor/samsung/a6lte/a6lte-vendor.mk)

# Eng-only boot property dump for boot diagnostics.
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/debug/a6lte-getprop.sh:$(TARGET_COPY_OUT_VENDOR)/bin/a6lte-getprop.sh \
    $(DEVICE_PATH)/debug/a6lte-getprop.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/a6lte-getprop.rc
