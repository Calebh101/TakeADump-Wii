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
#include <iomanip>
#include <ogc/ipc.h>

#define MICROSECOND 1 // From microsecond
#define MILLISECOND 1000 // From microsecond
#define SECOND 1000000 // From microsecond
#define MINUTE 60000000 // From microsecond

#define DVD_NO_HW_ACCESS -1000
#define DVD_NO_DISC      -1001
#define DVD_NORMAL 0xA8000000
#define DVD_DVDR   0xD0000000

#define WII_MAGIC 0x5D1C9EA3
#define NGC_MAGIC 0xC2339F3D

#define IS_NGC_DISC 0
#define IS_WII_DISC 1
#define IS_UNK_DISC 2

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

typedef struct {
    char gameCode[4];     // 0x00
    char makerCode[2];    // 0x04
    u8 discNumber;        // 0x06
    u8 gameVersion;       // 0x07
    u8 audioStreaming;    // 0x08
    u8 streamBufferSize;  // 0x09
    u8 reserved[14];      // 0x0A
    u32 magicWord;        // 0x18
    u8 padding[8];        // 0x1C

    std::string toString() const {
        std::string delim = ", ";
        std::ostringstream oss;

        oss << "GameCode: " << std::string(gameCode, 4)
            << delim << "Maker: " << std::string(makerCode, 2)
            << delim << "Disc Number: " << static_cast<int>(discNumber)
            << delim << "Version: " << static_cast<int>(gameVersion)
            << delim << "Magic: 0x" << std::hex << std::setw(8) << std::setfill('0') << magicWord;

        return oss.str();
    }
} DiscID;

static union {
	u32 inbuffer[0x10];
	ioctlv iovector[0x08];
} ATTRIBUTE_ALIGN(32);

extern u32 read_cmd;
extern volatile u32* dvd;
extern u32 dvd_hard_init;
extern int dumpCounter;
extern char gameName[32];
extern char internalName[512];
extern int di_fd;
extern DiscID* globalDiskId;

class DiskManager {
public:
    static volatile u32* get_dvd() {
        return dvd;
    }

    static char* getGameName() {
        return gameName;
    }

    static char* getGameInternalName() {
        return internalName;
    }

    static int initialize_disk() {
        Logger::print("Initializing DVD...");
        STACK_ALIGN(u8, id, 32, 32);
        u32 error;

        mask32(0x0D8000E0, 0x10, 0); // Enable GPIO for spin-up on drive reset (active low)
        mask32(0x0D800194, 0x400, 0); // Assert DI reset (active low)
        usleep(1 * MICROSECOND); // Deassert DI reset
        mask32(0x0D800194, 0, 0x400);

        error = dvd_get_error();
        if ((error >> 24) == 1) return DVD_NO_DISC;
        int idRet = -1;

        if ((!dvd_hard_init) || (dvd_get_error())) {
            dvd[0] = 0x54;
            dvd[2] = 0xA8000040;
            dvd[3] = 0;
            dvd[4] = 0x20;
            dvd[5] = (u32)id & 0x1FFFFFFF;
            dvd[6] = 0x20;
            dvd[7] = 3;

            while (dvd[7] & 1) usleep(1 * MILLISECOND);
            Logger::verbose("Got past DVD hard initialize");
            dvd_hard_init = 1;
        }

        if ((dvd_get_error() & 0xFFFFFF) == 0x053000) {
            read_cmd = DVD_DVDR;
        } else {
            read_cmd = DVD_NORMAL;
        }

        while (idRet != 0) {
            idRet = WDVD_LowReadDiskId();
            Logger::verbose("Disk identification returned code of %01d.", idRet);
            usleep(1 * SECOND);
        }

        DiscID* discId = read_dvd_id();
        globalDiskId = discId;
        if (discId == nullptr) return -1;
        return 0;
    }

    static int DVD_LowRead64(void* dst, u32 len, uint64_t offset) {
        if (offset >> 2 > 0xFFFFFFFF) return -1;

        dvd[0] = 0x2E;
        dvd[1] = 0;
        dvd[2] = read_cmd;
        dvd[3] = (read_cmd == DVD_DVDR) ? (offset >> 11) : (offset >> 2);
        dvd[4] = (read_cmd == DVD_DVDR) ? (len >> 11) : len;
        dvd[5] = (u32) dst & 0x1FFFFFFF;
        dvd[6] = len;
        dvd[7] = 3;

        DCInvalidateRange(dst, len);
        while (dvd[7] & 1) LWP_YieldThread();
        if (dvd[0] & 0x4) return 1;
        return 0;
    }

    static int identify_disc() {
        char readbuf[2048] __attribute__((aligned(32)));
        memset(&internalName[0],0,512);
        DVD_LowRead64(readbuf, 2048, 0ULL);

        if (readbuf[0]) {
            strncpy(&gameName[0], readbuf, 6);
            gameName[6] = 0;
            
            if (readbuf[6]) {
                size_t lastPos = strlen(gameName);
                sprintf(&gameName[lastPos], "-disc%i", (readbuf[6]) + 1);
            }

            strncpy(&internalName[0],&readbuf[32],512);
            internalName[511] = '\0';
        } else {
            sprintf(&gameName[0], "disc%i", dumpCounter);
        }

        if ((*(volatile u32*)(readbuf + 0x1C)) == NGC_MAGIC) {
            Logger::verbose("Found GameCube disc");
            return IS_NGC_DISC;
        } else if ((*(volatile u32*)(readbuf + 0x18)) == WII_MAGIC) {
            Logger::verbose("Found Wii disc");
            return IS_WII_DISC;
        } else {
            Logger::verbose("Found unknown disc");
            return IS_UNK_DISC;
        }
    }

    static int dump(DiscID* diskId, u32 diskSize);

private:
    static u32 dvd_get_error(void) {
        dvd[2] = 0xE0000000;
        dvd[8] = 0;
        dvd[7] = 1; // IMM

        while (dvd[7] & 1);
        return dvd[8];
    }

    /*static int WDVD_LowReadDiskId() {
        int result;
        void* outbuf = (void*)0x80000000;

        if (di_fd < 0) return -1;
        memset(outbuf, 0, 0x20);
        inbuffer[0] = 0x70000000;

        result = IOS_Ioctl(di_fd, 0x70, inbuffer, 0x20, outbuf, 0x20);
        return (result == 1) ? 0 : -result;
    }*/

    static int WDVD_LowReadDiskId() {
        int result;

        if (di_fd < 0) {
            di_fd = IOS_Open("/dev/di", 0);
            Logger::verbose("IOS_Open(\"/dev/di\") -> %d", di_fd);
            if (di_fd < 0) {
                return -1;
            }
        }

        void* outbuf = (void*)0x80000000;
        memset(outbuf, 0, 0x20);
        inbuffer[0] = 0x70000000;
        inbuffer[1] = 0;

        result = IOS_Ioctl(di_fd, 0x70, inbuffer, 0x20, outbuf, 0x20);
        Logger::verbose("IOS_Ioctl(di_fd,0x70) -> %d", result);
        DCInvalidateRange(outbuf, 0x20);

        if (result < 0) return -result;
        DiscID* d = (DiscID*)outbuf;

        if (d->magicWord == WII_MAGIC || d->magicWord == NGC_MAGIC) {
            return 0;
        } else {
            Logger::verbose("WDVD_LowReadDiskId: invalid magic 0x%08X", d->magicWord);
            return -2;
        }
    }

    static DiscID* read_dvd_id() {
        if (WDVD_LowReadDiskId() == 0) {
            DCInvalidateRange((void*)0x80000000, sizeof(DiscID));
            DiscID localId;
            memcpy(&localId, (void*)0x80000000, sizeof(DiscID));

            std::string gameCode(localId.gameCode, 4);
            Logger::verbose("Found disc ID: %s", localId.toString().c_str());
            return &localId;
        } else {
            Logger::verbose("Failed to read Disc ID");
            return nullptr;
        }
    }

    /*static int dvd_read_id() {
        char readbuf[2048] __attribute__((aligned(32)));
        int ret = DVD_LowRead64(readbuf, 2048, 0ULL);
        if (ret != 0) return ret;
        memcpy(&dvdId, readbuf, sizeof(DiscID));
        return 0;
    }*/
};

#endif