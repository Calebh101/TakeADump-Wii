#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>

void reset()
{
    Logger::newline();
    Logger::print("Exiting...");
    WPAD_Shutdown();
    exit(0);
}

u32 get_controller_buttons_pressed() {
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

int main(int argc, char **argv) {
    Graphics::init();
    WPAD_Init();
    SYS_SetResetCallback(reset);

    Logger::print("TakeADump %s by Calebh101", Constants::version.c_str());
    Logger::print("Press Home or Reset to exit.");

    while (1) {
        u32 buttons = get_controller_buttons_pressed();
    }
}