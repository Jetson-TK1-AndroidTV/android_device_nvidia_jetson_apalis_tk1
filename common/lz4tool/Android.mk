LOCAL_PATH := $(call my-dir)
include $(NVIDIA_DEFAULTS)

LOCAL_PREBUILT_EXECUTABLES := lz4c

include $(NVIDIA_HOST_PREBUILT)

