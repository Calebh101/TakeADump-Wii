#include <gccore.h>
#include "logger.hpp"

int main() {
    VIDEO_Init(); // Initialize video // Setup console
    Logger::print("Hello!");
}