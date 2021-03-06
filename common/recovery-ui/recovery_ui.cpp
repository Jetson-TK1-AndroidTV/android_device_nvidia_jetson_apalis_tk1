/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (c) 2015-2016 NVIDIA Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/input.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <cutils/properties.h>

#include <fs_mgr.h>
#include <fscheck_api.h>
#include "roots.h"
#include "common.h"
#include "device.h"
#include "screen_ui.h"
#include "ui.h"
#include "make_ext4fs.h"
extern "C" {
#include "wipe.h"
}
#include "recovery_ui.h"

enum USBMode {
    HOST_MODE,
    DEVICE_MODE,
};

Device::BuiltinAction APPLY_USB_STORAGE = static_cast<Device::BuiltinAction>(12);
Device::BuiltinAction COPY_RECOVERY_LOGS_USB = static_cast<Device::BuiltinAction>(13);

static char usb_mode[64] = "/sys/class/extcon/ID/connect";

const char* HEADERS[] = { "Volume up/down to move highlight;",
                          "enter/power button to select.",
                          "",
                          NULL };

const char* HEADERS_loki[] = { "Short press of Power button to move highlight;",
                               "Press Power button for 4-6s to select.",
                               "If you are using SHIELD controller, press X/Y button to move highlight;",
                               "Press button A or Power to select.",
                               NULL };

const char* HEADERS_foster[] = { "Short press of Power button to move highlight;",
                               "Press Power button for 4-6s to select.",
                               "FAT32 formatted USB drive is required to apply update or copy logs.",
                               NULL };

const char* HEADERS_darcy[] = { "Connect SHIELD Controller to the USB port near HDMI port.",
                                "The other USB port can be connected to PC for sideload,",
                                "or to a FAT32 formatted USB drive to apply update or copy logs.",
                                "Press X/Y button on the controller to move highlight in the below menu;",
                                "Press button A to select.",
                                NULL };

static const char* ITEMS[] =  {"reboot system now",
                               "apply update from ADB",
                               "wipe data/factory reset",
                               "wipe cache partition",
                               "reboot to bootloader",
                               "power down",
                               NULL };

static const char* ITEMS_foster[] =  {"reboot system now",
                               "apply update from ADB",
                               "apply update from SD card",
                               "apply update from USB storage",
                               "wipe data/factory reset",
                               "wipe cache partition",
                               "view recovery logs",
                               "copy recovery logs to USB",
                               NULL };

static const char* ITEMS_loki[] =  {"reboot system now",
                               "apply update from ADB",
                               "apply update from SD card",
                               "wipe data/factory reset",
                               "wipe cache partition",
                               "view recovery logs",
                               NULL };

static const char* ITEMS_darcy[] =  {"reboot system now",
                               "apply update from ADB",
                               "apply update from USB storage",
                               "wipe data/factory reset",
                               "wipe cache partition",
                               "view recovery logs",
                               "copy recovery logs to USB",
                               NULL };

static const char* ITEMS_hawkeye[] = {
        "Reboot system now",
        "Reboot to bootloader",
        "Apply update from ADB",
        "Apply update from SD card",
        "Apply update from USB storage",
        "Wipe data/factory reset",
        "Wipe cache partition",
        "Mount /system",
        "View recovery logs",
        "Power off",
         NULL
};

#if (PLATFORM_IS_AFTER_LOLLIPOP == 1)
RecoveryUI::KeyAction DefaultRecoveryUI::CheckKey(int key, bool long_press) {
    if (IsKeyPressed(KEY_POWER) && key == KEY_VOLUMEUP) {
        return TOGGLE;
    }

    if (key == KEY_DISPLAYTOGGLE || key == BTN_EAST) {
        return TOGGLE;
    }

    if (key == KEY_POWER) {
        ++consecutive_power_keys;
        if (consecutive_power_keys >= 7) {
            return REBOOT;
        }
    } else {
        consecutive_power_keys = 0;
    }
    return ENQUEUE;
}
#endif

class DefaultDevice : public Device {
  public:
#if (PLATFORM_IS_AFTER_LOLLIPOP != 1)
    DefaultDevice() :
        ui(new ScreenRecoveryUI) {
#else
    DefaultDevice(RecoveryUI *UI) :
        Device(UI), ui(UI) {
            property_get("ro.hardware", platform, "");
#endif
    }

#if (PLATFORM_IS_AFTER_LOLLIPOP != 1)
    RecoveryUI* GetUI() { return ui; }

    int HandleMenuKey(int key, int visible) {
        if (visible) {
            switch (key) {
              case KEY_DOWN:
              case KEY_VOLUMEDOWN:
                return kHighlightDown;

              case KEY_UP:
              case KEY_VOLUMEUP:
                return kHighlightUp;

              case KEY_ENTER:
              case KEY_POWER:
                return kInvokeItem;
            }
        }

        return kNoAction;
    }

    BuiltinAction InvokeMenuItem(int menu_position) {
        switch (menu_position) {
          case 0: return REBOOT;
          case 1: return APPLY_ADB_SIDELOAD;
          case 2: return WIPE_DATA;
          case 3: return WIPE_CACHE;
          case 4: return REBOOT_BOOTLOADER;
          case 5: return SHUTDOWN;
          default: return NO_ACTION;
        }
    }

    int WipeData() {
        erase_usercalibration_partition();
        return 0;
    }

    const char* const* GetMenuHeaders() { return HEADERS; }
    const char* const* GetMenuItems() { return ITEMS; }

#else
    const char* const* GetMenuHeaders() {
        if (!strncmp(platform, "loki_e", 6)) {
            return HEADERS_loki;
        }
        else if (!strncmp(platform, "foster_e", 8)) {
            return HEADERS_foster;
        }
        else if (!strncmp(platform, "darcy", 5)) {
            return HEADERS_darcy;
        }

        return HEADERS;
    }

    const char* const* GetMenuItems() {
        if (!strncmp(platform, "foster_e", 8)) {
            return ITEMS_foster;
        }
        else if (!strncmp(platform, "loki_e", 6)) {
            return ITEMS_loki;
        }
        else if (!strncmp(platform, "he2290", 6)) {
            return ITEMS_hawkeye;
        }
        else if (!strncmp(platform, "darcy", 5)) {
            return ITEMS_darcy;
        }

        return Device::GetMenuItems();
    }

    int HandleMenuKey(int key, int visible) {
        if (visible) {
            switch (key) {
              case KEY_DOWN:
              case KEY_VOLUMEDOWN:
              case BTN_NORTH:
                return kHighlightDown;

              case KEY_UP:
              case BTN_WEST:
              case KEY_VOLUMEUP:
                return kHighlightUp;

              case KEY_ENTER:
              case KEY_POWER:
              case BTN_GAMEPAD:
                return kInvokeItem;
            }
        }

        return kNoAction;
    }

    BuiltinAction InvokeMenuItem(int menu_position) {
        if (!strncmp(platform, "foster_e", 8)) {
            switch (menu_position) {
              case 0: return REBOOT;
              case 1: return APPLY_ADB_SIDELOAD;
              case 2: return APPLY_SDCARD;
              case 3: return APPLY_USB_STORAGE;
              case 4: return WIPE_DATA;
              case 5: return WIPE_CACHE;
              case 6: return VIEW_RECOVERY_LOGS;
              case 7: return COPY_RECOVERY_LOGS_USB;
              default: return NO_ACTION;
            }
        }
        else if (!strncmp(platform, "loki_e", 6)) {
            switch (menu_position) {
              case 0: return REBOOT;
              case 1: return APPLY_ADB_SIDELOAD;
              case 2: return APPLY_SDCARD;
              case 3: return WIPE_DATA;
              case 4: return WIPE_CACHE;
              case 5: return VIEW_RECOVERY_LOGS;
              default: return NO_ACTION;
            }
        }
        else if (!strncmp(platform, "darcy", 5)) {
            switch (menu_position) {
              case 0: return REBOOT;
              case 1: return APPLY_ADB_SIDELOAD;
              case 2: return APPLY_USB_STORAGE;
              case 3: return WIPE_DATA;
              case 4: return WIPE_CACHE;
              case 5: return VIEW_RECOVERY_LOGS;
              case 6: return COPY_RECOVERY_LOGS_USB;
              default: return NO_ACTION;
            }
        }
        else if (!strncmp(platform, "he2290", 6)) {
            switch (menu_position) {
                case 0: return REBOOT;
                case 1: return REBOOT_BOOTLOADER;
                case 2: return APPLY_ADB_SIDELOAD;
                case 3: return APPLY_SDCARD;
                case 4: return APPLY_USB_STORAGE;
                case 5: return WIPE_DATA;
                case 6: return WIPE_CACHE;
                case 7: return MOUNT_SYSTEM;
                case 8: return VIEW_RECOVERY_LOGS;
                case 9: return SHUTDOWN;
                default: return NO_ACTION;
            }
        }
        else
            return Device::InvokeMenuItem(menu_position);
    }

    bool PostWipeData() {
         int rc = erase_usercalibration_partition() ||
                  erase_fscheck_partition();
         if (!rc)
             return true;
         else
             return false;
    }

    bool ChangeUSBMode(USBMode target_mode) {
        if (!strncmp(platform, "darcy", 5)) {
            return change_usb_mode(target_mode);
        }
        return true;
    }

#endif

  private:
    int erase_usercalibration_partition() {
        const char* USERCALIB_PATH = "/usercalib";


        Volume *v = volume_for_path(USERCALIB_PATH);
        if (v == NULL) {
            // most devices won't have /usercalib, so this is not an error.
            return 0;
        }

        ui->SetBackground(RecoveryUI::ERASING);
        ui->SetProgressType(RecoveryUI::INDETERMINATE);
        ui->Print("Formatting %s...\n", USERCALIB_PATH);

        int fd = open(v->blk_device, O_RDWR);
        uint64_t size = get_file_size(fd);
        if (size != 0) {
            if (wipe_block_device(fd, size)) {
                LOGE("error wiping /usercalib: %s\n", strerror(errno));
                close(fd);
                return -1;
            }
        }

        close(fd);

        return 0;
    }

    int erase_fscheck_partition() {
        ui->Print("Formatting fscheck partition ...\n");
        return fscheck_clear_blob();
    }

  private:
    RecoveryUI* ui;
#if (PLATFORM_IS_AFTER_LOLLIPOP == 1)
    char platform[PROPERTY_VALUE_MAX+1];
#endif

    bool change_usb_mode(USBMode target_mode) {
        int fd;
        const char *host_mode = "USB-HOST";
        const char *device_mode = "none";
        char buf[16];

        if (target_mode == DEVICE_MODE) {
            if((fd = open(usb_mode, O_RDWR)) < 0) {
                ui->Print("Device mode switching fail ...\n");
                return false;
            }

            read(fd, buf, strlen(device_mode));
            if ((strncmp(buf, device_mode, strlen(device_mode))) != 0)
                 write(fd, device_mode, strlen(device_mode));

        } else {
            if((fd = open(usb_mode, O_RDWR)) < 0) {
                ui->Print("Host mode switching fail ...\n");
                return false;
            }

            read(fd, buf, strlen(host_mode));
            if ((strncmp(buf, host_mode, strlen(host_mode))) != 0)
                write(fd, host_mode, strlen(host_mode));
        }
        close(fd);
        return true;
    }
};

Device* make_device(int default_action) {
#if (PLATFORM_IS_AFTER_LOLLIPOP != 1)
    return new DefaultDevice();
#else
#ifdef BOARD_SUPPORTS_BILLBOARD
    if (default_action == Device::UPDATE_PACKAGE ||
        default_action == Device::APPLY_ADB_SIDELOAD)
        return new DefaultDevice(new BillboardRecoveryUI);
    else
#endif
        return new DefaultDevice(new DefaultRecoveryUI);
#endif
}

