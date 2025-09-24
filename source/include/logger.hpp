#include "wiiuse.hpp"
#include <stdio.h>
#include <cstdio>
#include <gccore.h>
#include "graphics.hpp"

#ifndef LOGGER_H
#define LOGGER_H

class Logger {
public:
    static void init() {
        Logger::verbose("Logger initialized");
    }

    static void newline() {
        printf("\n");
    }

    static void print(const char* input, ...) {
        va_list args;
        va_start(args, input);
        vprintf(input, args);
        va_end(args);
        putchar('\n');
    }

    static void moveUp(int lines = 1) {
        for (int i = 0; i < lines; i++) printf("\x1b[1A");
    }

    static void error(const int code, const char* input, ...);
    static void verbose(const char* input, ...);
};

#endif // LOGGER_H
