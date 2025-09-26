#include "disk.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <gccore.h>
#include "global.hpp"
#include <chrono>
#include <cmath>

#define SECTOR_SIZE 0x800 // 2 kilobytes
#define CHUNK_SIZE (2ULL*1024*1024*1024) // 2 gigabytes
#define MAX_READ_SIZE 128 * 1024

u32 read_cmd = DVD_NORMAL;
volatile u32* dvd = (volatile u32*)0xCC006000;
u32 dvd_hard_init = 0;
int dumpCounter = 0;
char gameName[32] = {0};
char internalName[512] = {0};
int di_fd = -1;
DiskID* globalDiskId = nullptr;
int errorCount = 0;

int DiskManager::dump(DiskID* diskId, int diskType, u64 diskSize) {
    void* buffer = memalign(32, SECTOR_SIZE);

    if (!buffer) {
        Logger::print("Failed to allocate buffer.");
        return 1;
    }

    FILE* fp = nullptr;
    u64 offset = 0;
    int chunk_index = 1;
    u64 chunk_offset = 0;
    u64 remaining = diskSize;
    u64 lba = 0;

    Logger::print("Taking a dump...");
    Logger::print("Dumped 0 bytes");
    auto start = std::chrono::high_resolution_clock::now();

    while (remaining > 0) {
        if (remaining <= 0) break;
        Logger::moveUp();

        if (!fp || chunk_offset >= CHUNK_SIZE) {
            if (fp) fclose(fp);
            char filename[256];
            snprintf(filename, sizeof(filename), "%s.part%d.iso", diskId->gameCode, chunk_index++);
            fp = fopen(filename, "wb");

            if (!fp) {
                Logger::newline();
                Logger::print("Failed to create file: %s", filename);
                free(buffer);
                return -2;
            }

            Logger::verbose("New chunk created at file: %s", filename);
            chunk_offset = 0;
        }

        u32 read_size = (remaining > SECTOR_SIZE) ? SECTOR_SIZE : (u32)remaining;
        int ret = -1;
        int tries = 0;

        while (ret != 0 && tries < 3) {
            ret = DVD_LowRead64(buffer, read_size, lba << 11);
            tries++;
        }

        if (ret != 0) {
            errorCount++;
            Logger::print("Error reading disk at offset 0x%08llX.", offset);

            if (Global::cancelOnError) {
                Logger::error(ret, "Unrecovered read error.");
            } else {
                memset(buffer, 0, read_size);
            }
        }

        size_t written = fwrite(buffer, 1, read_size, fp);

        if (written != read_size) {
            Logger::print("Error writing file. Bytes written: %d. Bytes expected: %d.", written, read_size);
            fclose(fp);
            free(buffer);
            return -2;
        }

        lba += read_size >> 11;
        offset += read_size;
        chunk_offset += read_size;
        remaining -= read_size;

        Logger::print("Dumped %.2f MB / %.2f MB (%d%%) (%d errors)", offset / 1024.0 / 1024.0, diskSize / 1024.0 / 1024.0, static_cast<int>(round(((double)offset / (double)diskSize) * 100)), errorCount);
        if (Global::get_controller_buttons_pressed() & WPAD_BUTTON_HOME) Global::reset();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (fp) fclose(fp);
    free(buffer);
    Logger::print("Dump complete after %d minutes!", duration.count() / 1000 / 60);
    return 0;
}