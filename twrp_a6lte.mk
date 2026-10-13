#
# Copyright 2017 The Android Open Source Project
# Copyright (C) 2026 samsungexynos7870
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

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/base.mk)

# Inherit some common TWRP stuff
$(call inherit-product, vendor/twrp/config/common.mk)

PRODUCT_RELEASE_NAME := a6lte

PRODUCT_PACKAGES += \
    charger \
    charger_res_images

# DTBH packer used by build/tasks/dtb.mk
PRODUCT_HOST_PACKAGES += dtbhtoolExynos

# Device identifier. This must come after all inclusions
PRODUCT_DEVICE := a6lte
PRODUCT_NAME := twrp_a6lte
PRODUCT_BRAND := samsung
PRODUCT_MODEL := Samsung Galaxy A6 2018
PRODUCT_MANUFACTURER := samsung
PRODUCT_GMS_CLIENTID_BASE := android-samsung

# Galaxy A6 2018 launched with Android 8.0 (API 26).
PRODUCT_SHIPPING_API_LEVEL := 26
