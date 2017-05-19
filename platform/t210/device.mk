# NVIDIA Tegra5 "T124" development system
#
# Copyright (c) 2013-2017 NVIDIA Corporation.  All rights reserved.
#
# 32-bit specific product settings

$(call inherit-product-if-exists, frameworks/base/data/videos/VideoPackage2.mk)
$(call inherit-product, device/nvidia/platform/t210/device-common.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/t210/nvflash.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/raydium.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/sharp.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/touch/nvtouch.mk)
$(call inherit-product, device/nvidia/platform/t210/motionq/motionq.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/multimedia/base.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/multimedia/firmware.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/camera/full.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/services/nvcpl.mk)
$(call inherit-product-if-exists, vendor/nvidia/tegra/core/android/services/analyzer.mk)
$(call inherit-product, vendor/nvidia/tegra/core/android/services/edid.mk)
$(call inherit-product-if-exists, vendor/nvidia/tegra/core/android/t124/full.mk)
$(call inherit-product-if-exists, vendor/nvidia/tegra/core/nvidia-tegra-vendor.mk)

#enable Widevine drm
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libdrmwvmplugin \
    libwvm \
    libWVStreamControlAPI_L1 \
    libwvdrm_L1

# Need AppWidget permission to prevent from Launcher's crash.
# TODO(pattjin): Remove this when the TV Launcher is used, which does not support AppWidget.
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.software.app_widgets.xml:system/etc/permissions/android.software.app_widgets.xml

# Boot Animation
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bootanimation.zip:system/media/bootanimation.zip

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayouts/AliTV_Remote_V1_Airmouse.kl:system/usr/keylayout/AliTV_Remote_V1_Airmouse.kl \
    $(LOCAL_PATH)/keylayouts/AliTV_Remote_V1_Airmouse.idc:system/usr/idc/AliTV_Remote_V1_Airmouse.idc

PRODUCT_COPY_FILES += \
   device/nvidia/platform/loki/t210/pbc.conf:system/etc/pbc.conf

PRODUCT_PACKAGES += \
    bpmp \
    tegra_xusb_firmware \
    tegra12x_xusb_firmware

PRODUCT_PACKAGES += \
        tos \
        keystore.tegra \
        gatekeeper.tegra \
        icera_host_test \
        setup_fs \
        e2fsck \
        make_ext4fs \
        hdmi_cec.tegra \
        lights.tegra \
        pbc.tegra \
        pbc.hawkeye \
        power.tegra \
        power.loki_e \
        power.loki_e_lte \
        power.loki_e_wifi \
        power.darcy \
        power.foster_e \
        power.foster_e_hdd \
        libnvglsi \
        libnvwsi

# HDCP SRM Support
PRODUCT_PACKAGES += \
        hdcp1x.srm \
        hdcp2x.srm \
        hdcp2xtest.srm

#enable Widevine drm
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += \
    liboemcrypto \
    libdrmdecrypt

PRODUCT_RUNTIMES := runtime_libart_default

PRODUCT_PACKAGES += \
    gpload \
    ctload \
    c2debugger

#TegraOTA
PRODUCT_PACKAGES += \
    TegraOTA

ifneq ($(wildcard vendor/nvidia/tegra/core-private),)
PRODUCT_PACKAGES += \
    track.sh
endif

ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
PRODUCT_PACKAGES += \
    Stats
endif

# LED service
PRODUCT_PACKAGES += \
    NvShieldService \
    NvGamepadMonitorService

# Application for sending feedback to NVIDIA
PRODUCT_PACKAGES += \
    nvidiafeedback

PRODUCT_PACKAGES += \
    libwpa_client \
    hostapd \
    wpa_supplicant \
    wpa_supplicant.conf

PRODUCT_PACKAGES += \
    badblocks \
    e2fsck \
    fsck.exfat \
    fsck.f2fs \
    mke2fs \
    make_f2fs \
    mkfs.exfat \
    mkntfs \
    mount.exfat \
    ntfs-3g \
    ntfsfix \
    resize2fs \
    tune2fs

# Bluetooth USB
PRODUCT_PACKAGES += \
    btattach \
    hciattach \
    hciconfig \
    hcitool

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/bt_vendor.conf:system/etc/bluetooth

# Leanback Launcher
PRODUCT_PACKAGES += \
    AppDrawer \
    LeanbackLauncher \
    LeanbackCustomizer \
    LeanbackIme \
    LiveTv \
    TvProvider \
    TvSettings \
    tv_input.default \
    TV

# Leanback Gapps
$(call inherit-product, vendor/google/jetson/jetson-vendor.mk)
$(call inherit-product-if-exists, vendor/google/atv/atv-vendor.mk)

#for SMD partition
PRODUCT_PACKAGES += \
    slot_metadata

# touch screen and camera don't apply to Foster/Darcy
ifeq ($(filter foster_e% darcy%, $(TARGET_PRODUCT)),)
# Sharp touch
PRODUCT_COPY_FILES += \
    device/nvidia/drivers/touchscreen/lr388k7_ts.idc:system/usr/idc/lr388k7_ts.idc \
    device/nvidia/common/init.sharp_touch.rc:root/init.sharp_touch.rc

# Nvidia touch
PRODUCT_COPY_FILES += \
    device/nvidia/common/init.nv_touch.rc:root/init.nv_touch.rc

# Nvidia Camera app
PRODUCT_PACKAGES += NvCamera
endif

# bmp.blob
PRODUCT_PACKAGES += bmp

#symlinks
PRODUCT_PACKAGES += gps.symlink
