#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <gccore.h>

#define MEM_K0_TO_K1(x) ((u32*)((u32)(x) + (SYS_BASE_UNCACHED - SYS_BASE_CACHED)))

class Graphics {
public:
    static void init();

    static GXRModeObj* rmode;
    static u32* xfb;
};

#endif // GRAPHICS_H
