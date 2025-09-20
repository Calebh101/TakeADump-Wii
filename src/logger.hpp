#ifdef GEKKO
#include <stdio.h>
#include <cstdio>
#include <wiiuse/wpad.h>
#include <gccore.h>
#endif

#ifndef LOGGER_H   // Include guard start
#define LOGGER_H

class Logger {
public:
    static void init() {
        console_init(NULL, 0, 0, 640, 480, 640 * VI_DISPLAY_PIX_SZ);
    }

    static void print(const char* input) {
        printf("%s\n", input);
        VIDEO_Flush();
        VIDEO_WaitVSync();
        VIDEO_WaitVSync();
    }
    
    static void clear() {
        for(int i = 0; i < 30; i++) printf("\n");
        VIDEO_Flush();
        VIDEO_WaitVSync();
        VIDEO_WaitVSync();
    }
};

#endif // LOGGER_H
