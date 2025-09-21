#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>

#ifndef GLOBAL_H
#define GLOBAL_H

class Global {
public:
    static u32 get_controller_buttons_pressed() {
        WPAD_ScanPads();
        u32 buttons = 0;
        WPADData* wiiPad;
        wiiPad = WPAD_Data(0);

        if (wiiPad->btns_h & WPAD_BUTTON_B) buttons |= PAD_BUTTON_B;
        if (wiiPad->btns_h & WPAD_BUTTON_A) buttons |= PAD_BUTTON_A;
        if (wiiPad->btns_h & WPAD_BUTTON_LEFT) buttons |= PAD_BUTTON_LEFT;
        if (wiiPad->btns_h & WPAD_BUTTON_RIGHT) buttons |= PAD_BUTTON_RIGHT;
        if (wiiPad->btns_h & WPAD_BUTTON_UP) buttons |= PAD_BUTTON_UP;
        if (wiiPad->btns_h & WPAD_BUTTON_DOWN) buttons |= PAD_BUTTON_DOWN;

        if (wiiPad->btns_h & WPAD_BUTTON_HOME) reset();
        return buttons;
    }

    static void reset(int code = 0) {
        Logger::newline();
        Logger::print("Exiting with code %02d...", code);
        WPAD_Shutdown();
        exit(code);
    }

    static void resetCallback() {
        reset();
    }
};

#endif // GLOBAL_H