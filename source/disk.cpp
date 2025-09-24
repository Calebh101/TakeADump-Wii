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
#define MAX_READ 2ULL*1024*1024*1024;

u32 read_cmd = DVD_NORMAL;
volatile u32* dvd = (volatile u32*)0xCC006000;
u32 dvd_hard_init = 0;
int dumpCounter = 0;
char gameName[32] = {0};
char internalName[512] = {0};
int di_fd = -1;
DiscID* globalDiskId = nullptr;

int DiskManager::dump(DiscID* diskId, int diskType, u64 diskSize) {
    void* buffer = memalign(32, SECTOR_SIZE);

    if (!buffer) {
        Logger::print("Failed to allocate buffer.");
        return 1;
    }

    FILE* fp = nullptr;
    u64 offset = 0;
    int chunk_index = 0;
    u64 chunk_offset = 0;

    Logger::print("Taking a dump...");
    Logger::print("Dumping will start shortly.");
    auto start = std::chrono::high_resolution_clock::now();

    while (offset < diskSize) {
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

        u32 read_size = SECTOR_SIZE;
        if ((chunk_offset + read_size) > CHUNK_SIZE) read_size = (u32)(CHUNK_SIZE - chunk_offset);
        if ((offset + read_size) > diskSize) read_size = (u32)(diskSize - offset);
        int ret = DVD_LowRead64(buffer, read_size, (u32)(offset & 0xFFFFFFFF));

        if (ret != 0) {
            Logger::print("Error reading disk at offset 0x%08llX.", offset);

            if (Global::cancelOnError) {
                fclose(fp);
                free(buffer);
                return -1;
            }
        }

        size_t written = fwrite(buffer, 1, SECTOR_SIZE, fp);

        if (written != SECTOR_SIZE) {
            Logger::print("Error writing file. Bytes written: %d. Bytes expected: %d.", written, SECTOR_SIZE);
            fclose(fp);
            free(buffer);
            return -2;
        }

        offset += SECTOR_SIZE;
        chunk_offset += SECTOR_SIZE;
        Logger::print("Dumped %.2f MB / %.2f MB (%d%%)", offset / 1024.0 / 1024.0, diskSize / 1024.0 / 1024.0, static_cast<int>(round(((double)offset / (double)diskSize) * 100)));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (fp) fclose(fp);
    free(buffer);
    Logger::print("Dump complete after %d minutes!", duration.count() / 1000 / 60);
    return 0;
}