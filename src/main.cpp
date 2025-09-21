#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"

int main() {
    Graphics::init();

    Logger::newline();
    Logger::newline();
    Logger::newline();
    Logger::newline();

    Logger::print("TakeADump %s by Calebh101", Constants::version.c_str());
    Logger::print("Press Home to exit.");

    while (1) {}
}