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

int Global::driveFs = FS_FAT32;
int Global::driveType = -1;

void menu() {
    Logger::print("1  Dump disk to %s USB", Global::driveFs == FS_FAT32 ? "FAT32" : "NTFS");
    Logger::print("2  Dump disk to %s SD card", Global::driveFs == FS_FAT32 ? "FAT32" : "NTFS");
    Logger::print("A  Toggle stop on read error (currently: %s)", Global::cancelOnError ? "ON" : "OFF");
    Logger::print("B  Change filesystem (currently: %s)", Global::driveFs == FS_FAT32 ? "FAT32" : "NTFS");
    Logger::print("+  Exit");
    Logger::print("-  Reprint menu");
}

void reset_callback(u32 a, void* b) {
    Logger::verbose("Received reset callback of type 1");
    Global::reset();
}

void reset_callback_2() {
    Logger::verbose("Received reset callback of type 2");
    Global::reset();
}

int main(int argc, char **argv) {
    Graphics::init();
    WPAD_Init();
    SYS_SetResetCallback(reset_callback);

    Logger::print("TakeADump %s by Calebh101", Constants::version.c_str());
    Logger::print("Please note: We are NOT responsible for any damage to your console, game disks, or external media.");
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
            Global::cancelOnError = !Global::cancelOnError;
            Global::setCancelOnError(Global::cancelOnError);
            menu();
            Logger::newline();
        } else if (buttons & WPAD_BUTTON_B) {
            if (Global::driveFs == FS_FAT32) {
                Global::driveFs = FS_NTFS;
            } else if (Global::driveFs = FS_NTFS) {
                Global::driveFs = FS_FAT32;
            } else {
                Global::driveFs = FS_FAT32;
            }

            menu();
            Logger::newline();
        } else if (buttons & WPAD_BUTTON_1) {
            Global::driveType = DRIVE_USB;
            break;
        } else if (buttons & WPAD_BUTTON_2) {
            Global::driveType = DRIVE_SD;
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

    Logger::verbose("Selected drive of type %01d,%01d...", Global::driveType, Global::driveFs);
    Logger::print("Insert your %s %s now, and press A.", Global::driveFs == FS_FAT32 ? "FAT32" : "NTFS", Global::driveType == DRIVE_SD ? "SD card" : "USB drive");
    Global::waitForA();
    Global::waitForButtonRelease();

    if (Global::driveFs == FS_FAT32) {
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
            if (Global::driveType == DRIVE_SD) {
                Logger::verbose("Mounting DRIVE_SD now...");
                usleep(500);
                mountRet = fatMountSimple("target", sdcard);
            } else if (Global::driveType == DRIVE_USB) {
                Logger::verbose("Mounting DRIVE_USB now...");
                usleep(500);
                mountRet = fatMountSimple("target", usb);
            } else {
                Logger::error(-1, "Invalid drive type: %01d", Global::driveType);
            }
        } catch (const std::exception& e) {
            Logger::error(1, "Unhandled FAT exception: %s", e.what());
        } catch (...) {
            Logger::error(2, "Unhandled FAT exception");
        }

        if (!mountRet) {
            Logger::error(mountRet, "Unable to mount drive.");
        }
    } else if (Global::driveFs == FS_NTFS) {
        Logger::verbose("Initializing NTFS...");
        Logger::error(-1, "NTFS is currently unsupported. Sorry for the inconvenience. (Feel free to make a pull request!)");
    } else {
        Logger::error(-1, "Invalid filesystem type: %01d", Global::driveFs);
    }

    Logger::print("Insert your %s now, and press A.", "game disk");
    Global::waitForA();
    Global::waitForButtonRelease();

    int initRet = DiskManager::initialize_disk();
    Logger::verbose("Initialized disk with code %01d.", initRet);

    if (initRet != 0) {
        Logger::error(initRet, "Unable to initialize disk.");
    }

    DiskID* disk = DiskManager::get_disk_id();
    int type = DiskManager::identify_disk(disk);
    Logger::verbose("Found disk type of %d", type);
    if (type == IS_UNK_DISK) Logger::error(-1, "Disk is unknown.");
    u64 size = DiskManager::get_disk_size_bytes(type);
    Logger::verbose("Found disk size of %u bytes (%.2f MB)", size, size / 1024.0f / 1024.0f);

    Logger::print("Press A to start dumping.");
    Logger::print("Press Reset to exit.");
    Global::waitForA();
    Global::waitForButtonRelease();
    int dumpRet = DiskManager::dump(disk, type, size);

    if (dumpRet != 0) {
        Logger::error(dumpRet, "Dump failed.");
    } else {
        Logger::verbose("Dump complete.");
        Global::resetAfterPause();
    }
}