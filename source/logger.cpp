#include "logger.hpp"
#include "global.hpp"

int verboseCount = 0;

void Logger::error(const int code, const char* input, ...) {
    va_list args;
    va_start(args, input);
    printf("ERR %01d: ", code);
    vprintf(input, args);
    va_end(args);
    putchar('\n');
    Global::resetAfterPause();
}

void Logger::verbose(const char* input, ...) {
#ifndef DEBUG
    return;
#endif

    va_list args;
    va_start(args, input);
    printf("VBS %01d: ", verboseCount);
    vprintf(input, args);
    va_end(args);
    putchar('\n');
    verboseCount++;
}