#pragma once
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <cstdlib>
#endif

namespace fs = std::filesystem;

static fs::path GetExecutableDir()
{
#ifdef _WIN32
    char Buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, Buffer, MAX_PATH);
    return fs::path(Buffer).parent_path();

#else
    char Buffer[4096];

    ssize_t Length = readlink("/proc/self/exe", Buffer, sizeof(Buffer) - 1);

    if (Length == -1)
        return fs::current_path();

    Buffer[Length] = '\0';

    return fs::path(Buffer).parent_path();
#endif
}

static fs::path GetAssetsPath()
{
#ifdef _WIN32
    return GetExecutableDir() / "assets";
#else
    return fs::path("/usr/share/nfetch/assets");
#endif
}

auto assets = GetAssetsPath();
auto nixborn_logo = (assets / "nixborn.png").string();

typedef struct {
    int w, h, c;
    unsigned char* stbdata;
} STBImageReturn;

struct CPUInfo{
    std::string model;
    int cores;
    int threads;
    float clock;
};

struct GPUInfo{
    std::string model;
    uint64_t vram;
    uint64_t shared;
    uint64_t cores;
    uint64_t frequency;
};

struct MemoryInfo{
    uint64_t total;
    uint64_t used;
    uint64_t free;
}; 

struct DiskInfo{
    uint64_t total;
};

struct SystemInfo{
    std::string os;
    std::string kernel;
    int bits;

    std::vector<CPUInfo> cpu_array;
    std::vector<GPUInfo> gpu_array;
    std::vector<MemoryInfo> memory_array;
    std::vector<DiskInfo> disk_array;
};