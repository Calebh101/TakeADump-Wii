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
int driveFs = FS_FAT32;
int driveType = -1;

void menu() {
    Logger::print("1  Dump disc to %s USB", driveFs == FS_FAT32 ? "FAT32" : "NTFS");
    Logger::print("2  Dump disc to %s SD card", driveFs == FS_FAT32 ? "FAT32" : "NTFS");
    Logger::print("A  Toggle stop on error (currently: %s)", stopOnError ? "ON" : "OFF");
    Logger::print("B  ");
    Logger::print("+  Exit");
    Logger::print("-  Reprint menu");
}

void reset_callback(u32 a, void* b) {
    Global::reset();
}

void reset_callback_2() {
    Global::reset();
}

int main(int argc, char **argv) {
    Graphics::init();
    WPAD_Init();
    SYS_SetResetCallback(reset_callback);

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

    while (1) {
        u32 buttons = Global::get_controller_buttons_pressed();
        
        if (buttons & WPAD_BUTTON_A) {
            stopOnError = !stopOnError;
            Global::setCancelOnError(stopOnError);
            menu();
            Logger::newline();
        } else if (buttons & WPAD_BUTTON_B) {
            if (driveFs == FS_FAT32) {
                driveFs = FS_NTFS;
            } else if (driveFs = FS_NTFS) {
                driveFs = FS_FAT32;
            } else {
                driveFs = FS_FAT32;
            }

            menu();
            Logger::newline();
        } else if (buttons & WPAD_BUTTON_1) {
            Logger::print("Selected USB drive");
            driveType = DRIVE_USB;
            break;
        } else if (buttons & WPAD_BUTTON_2) {
            Logger::print("Selected SD card");
            driveType = DRIVE_SD;
            break;
        } else if (buttons & WPAD_BUTTON_MINUS) {
            menu();
            Logger::newline();
        } else if (buttons & WPAD_BUTTON_PLUS) {
            Global::reset();
        }

        if (buttons != 0) {
            Global::waitForButtonRelease();
        }
    }

    Logger::verbose("Selected drive of type %01d,%01d...", driveType, driveFs);
    Logger::print("Insert your %s %s now, and press A.", driveFs == FS_FAT32 ? "FAT32" : "NTFS", driveType == DRIVE_SD ? "SD card" : "USB drive");
    Global::waitForA();
    Global::waitForButtonRelease();

    if (driveFs == FS_FAT32) {
        Logger::verbose("Initializing FAT32...");
        int mountRet;
        int fsRet = true;
        //fsRet = fatInitDefault();

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
    } else if (driveFs == FS_NTFS) {
        Logger::verbose("Initializing NTFS...");
    } else {
        Logger::error(-1, "Invalid filesystem type: %01d", driveFs);
    }

    Logger::print("Insert your %s now, and press A.", "game disc");
    Global::waitForA();
    Global::waitForButtonRelease();

    int ret = DiskManager::initialize_disk();
    Logger::verbose("Initialized disk with code %01d.", ret);
    Global::resetAfterPause();
}