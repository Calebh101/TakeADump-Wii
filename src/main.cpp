#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>
#include "global.hpp"

bool stopOnError = true;

int main(int argc, char **argv) {
    Graphics::init();
    WPAD_Init();
    SYS_SetResetCallback(Global::resetCallback);

    Logger::print("TakeADump %s by Calebh101", Constants::version.c_str());
    Logger::print("Press Reset to exit.");
    Logger::newline();

    Logger::print("1. Dump disc to FAT32 USB");
    //Logger::print("2. Dump disc to FAT32 SD card");
    Logger::print("A. Toggle stop on error (currently: %s)", stopOnError ? "ON" : "OFF");
    Logger::print("B. Exit");

    while (1) {
        u32 buttons = Global::get_controller_buttons_pressed();
        
        if (buttons & WPAD_BUTTON_A) {
            stopOnError = !stopOnError;
            Logger::print("A. Toggle stop on error (currently: %s)", stopOnError ? "ON" : "OFF");
        } else if (buttons & WPAD_BUTTON_B) {
            Global::reset();
        } else if (buttons & WPAD_BUTTON_1) {
            Logger::newline();
            Logger::print("Selected: FAT32 USB drive");
        }
    }
}