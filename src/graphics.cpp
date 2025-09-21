#include "graphics.hpp"

GXRModeObj* Graphics::rmode = nullptr;
u32* Graphics::xfb = nullptr;

void Graphics::init() {
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    CON_Init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE); VIDEO_Flush(); VIDEO_WaitVSync(); VIDEO_WaitVSync();
}
