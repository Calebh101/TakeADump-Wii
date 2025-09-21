#include <gccore.h>
#include "logger.hpp"
#include "wiiuse.hpp"
#include <unistd.h>
#include "graphics.hpp"
#include "constants.hpp"
#include <ogc/system.h>
#include "iso9660.h"
#include "processor.h"
#include <cstring>

#define DVD_NO_HW_ACCESS -1000
#define DVD_NO_DISC      -1001
#define DVD_NORMAL 0xA8000000
#define DVD_DVDR   0xD0000000

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

static u32 read_cmd = DVD_NORMAL;
volatile u32* dvd = (volatile u32*)0xCC006000;
u32 dvd_hard_init = 0;

class DiskManager {
public:
    static int initialise_dvd() {
        Logger::print("Initializing DVD...");
        STACK_ALIGN(u8, id, 32, 32);
        u32 error;

        mask32(0x0D8000E0, 0x10, 0); // Enable GPIO for spin-up on drive reset (active low)
        mask32(0x0D800194, 0x400, 0); // Assert DI reset (active low)
        usleep(1000); // Deassert DI reset
        mask32(0x0D800194, 0, 0x400);

        error = dvd_get_error();
        if ((error >> 24) == 1) return DVD_NO_DISC;

        if ((!dvd_hard_init) || (dvd_get_error())) {
            dvd[0] = 0x54;
            dvd[2] = 0xA8000040;
            dvd[3] = 0;
            dvd[4] = 0x20;
            dvd[5] = (u32)id & 0x1FFFFFFF;
            dvd[6] = 0x20;
            dvd[7] = 3;

            while (dvd[7] & 1) usleep(20000);
            dvd_hard_init = 1;
        }

        if ((dvd_get_error() & 0xFFFFFF) == 0x053000) {
            read_cmd = DVD_DVDR;
        } else {
            read_cmd = DVD_NORMAL;
        }

        dvd_read_id();
        return 0;
    }

    static int DVD_LowRead64(void* dst, u32 len, uint64_t offset) {
	if (offset >> 2 > 0xFFFFFFFF)
		return -1;

	if ((((u32) dst) & 0xC0000000) == 0x80000000) // cached?
		dvd[0] = 0x2E;
        dvd[1] = 0;
        dvd[2] = read_cmd;
        dvd[3] = read_cmd == DVD_DVDR ? offset >> 11 : offset >> 2;
        dvd[4] = read_cmd == DVD_DVDR ? len >> 11 : len;
        dvd[5] = (u32) dst & 0x1FFFFFFF;
        dvd[6] = len;
        dvd[7] = 3; // enable reading!
        DCInvalidateRange(dst, len);
        while (dvd[7] & 1)
            LWP_YieldThread();

        if (dvd[0] & 0x4)
            return 1;
        return 0;
    }

private:
    static u32 dvd_get_error(void) {
        dvd[2] = 0xE0000000;
        dvd[8] = 0;
        dvd[7] = 1; // IMM

        while (dvd[7] & 1);
        return dvd[8];
    }

    static int dvd_read_id() {
        char readbuf[2048] __attribute__((aligned(32)));
        DVD_LowRead64(readbuf, 2048, 0ULL);
        memcpy((void*)0x80000000, readbuf, 32);
        return 0;
    }
};

#endif