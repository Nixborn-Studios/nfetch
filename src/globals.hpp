#pragma once
#include <chafa.h>

const char* nixborn_logo = "./assets/nixborn.png";
const char* windows_logo = "./assets/windows.png";
const char* linux_logo = "./assets/linux.png";

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