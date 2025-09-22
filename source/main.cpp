#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>
#include "global.hpp"
#include "disk.hpp"
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <ogc/disc_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>

bool stopOnError = true;

void menu() {
    Logger::print("1  Dump disc to FAT32 USB");
    Logger::print("2  Dump disc to FAT32 SD card");
    Logger::print("A  Toggle stop on error (currently: %s)", stopOnError ? "ON" : "OFF");
    Logger::print("B  Exit");
    Logger::print("-  Reprint menu");
}

int main(int argc, char **argv) {
    Graphics::init();
    WPAD_Init();
    SYS_SetResetCallback(Global::resetCallback);

    Logger::print("TakeADump %s by Calebh101", Constants::version.c_str());
    Logger::print("Please note: We are NOT responsible for any damage to your console, game discs, or external media.");
    Logger::newline();
    Logger::print("Press A to continue.");
    Logger::print("Press Reset to exit any time.");
    Logger::newline();

    Global::waitForA();
    Global::waitForButtonRelease();
    menu();
    Logger::newline();

    int driveType = -1;
    int driveFs = -1;

    while (1) {
        u32 buttons = Global::get_controller_buttons_pressed();
        
        if (buttons & WPAD_BUTTON_A) {
            stopOnError = !stopOnError;
            Global::setCancelOnError(stopOnError);
            Logger::print("A. Toggle stop on error (currently: %s)", stopOnError ? "ON" : "OFF");
        } else if (buttons & WPAD_BUTTON_B) {
            Global::reset();
        } else if (buttons & WPAD_BUTTON_1) {
            Logger::print("Selected: FAT32 USB drive");
            driveType = DRIVE_USB;
            driveFs = FS_FAT32;
            break;
        } else if (buttons & WPAD_BUTTON_2) {
            Logger::print("Selected: FAT32 SD card");
            driveType = DRIVE_SD;
            driveFs = FS_FAT32;
            break;
        } else if (buttons & WPAD_BUTTON_MINUS) {
            Logger::newline();
            menu();
            Logger::newline();
        }

        if (buttons != 0) {
            Global::waitForButtonRelease();
        }
    }

    Logger::verbose("Selected drive of type %01d,%01d...", driveType, driveFs);
    Logger::print("Insert your %s now, and press A.", driveType == DRIVE_SD ? "SD card" : "USB drive");
    Global::waitForA();
    Global::waitForButtonRelease();

    int mountRet;
    int fsRet = 1;

    if (fsRet <= 0) {
        Logger::error(fsRet, "Unable to initialize FAT devices.");
    }

    const DISC_INTERFACE* sdcard = &__io_wiisd;
    const DISC_INTERFACE* usb = &__io_usbstorage;

    try {
        if (driveType == DRIVE_SD) {
            Logger::verbose("Mounting DRIVE_SD now...");
            usleep(500);
            mountRet = fatMountSimple("target", sdcard);
        } else if (driveType == DRIVE_USB) {
            Logger::verbose("Mounting DRIVE_USB now...");
            usleep(500);
            mountRet = fatMountSimple("target", usb);
        } else {
            Logger::error(-1, "Invalid drive type: %01d", driveType);
        }
    } catch (const std::exception& e) {
        Logger::error(1, "Unhandled FAT exception: %s", e.what());
    } catch (...) {
        Logger::error(2, "Unhandled FAT exception");
    }

    if (!mountRet) {
        Logger::error(mountRet, "Unable to mount drive.");
    }

    Logger::print("Insert your %s now, and press A.", "game disc");
    Global::waitForA();
    Global::waitForButtonRelease();

    int ret = DiskManager::initialize_disk();
    Logger::verbose("Initialized disk with code %01d.", ret);
    Global::resetAfterPause();
}