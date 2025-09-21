#include "wiiuse.hpp"
#include <stdio.h>
#include <cstdio>
#include <gccore.h>
#include "graphics.hpp"

#ifndef LOGGER_H
#define LOGGER_H

class Logger {
public:
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
    
    static void clear() {
        for(int i = 0; i < 30; i++) newline();
    }
};

#endif // LOGGER_H
