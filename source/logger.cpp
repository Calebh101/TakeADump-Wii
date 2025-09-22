#include "logger.hpp"
#include "global.hpp"

void Logger::error(const int code, const char* input, ...) {
    va_list args;
    va_start(args, input);
    printf("ERR %01d: ", code);
    vprintf(input, args);
    va_end(args);
    putchar('\n');
    Global::resetAfterPause();
}