#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>

#define DRIVE_USB 0
#define DRIVE_SD 1
#define FS_FAT32 0

#ifndef GLOBAL_H
#define GLOBAL_H

class Global {
public:
    static void setCancelOnError(bool status) {
        cancelOnError = status;
    }

    static u32 get_controller_buttons_pressed() {
        WPAD_ScanPads();
        WPADData* wiiPad;
        wiiPad = WPAD_Data(0);
        u32 buttons = wiiPad->btns_h;
        //if (buttons & WPAD_BUTTON_HOME) reset();
        return buttons;
    }

    static void reset(int code = 0) {
        Logger::print("Exiting with code %02d...", code);
        WPAD_Shutdown();
        exit(code);
    }

    static void waitForA() {
        while (true) {
            u32 buttons = get_controller_buttons_pressed();
            if (buttons & WPAD_BUTTON_A) break;
            VIDEO_WaitVSync();
        }
    }

    static void waitForButtonRelease() {
        while (true) {
            if (Global::get_controller_buttons_pressed() == 0) break;
            VIDEO_WaitVSync();
        }
    }

    static void resetAfterPause(int code = 0) {
        Logger::print("Press A to exit.");
        waitForButtonRelease();
        waitForA();
        waitForButtonRelease();
        reset(0);
    }

    static bool cancelOnError;
};

#endif // GLOBAL_H