#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <globals.hpp>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <limits>
#include "hwinfo/hwinfo.h"

namespace NFETCH {
    SystemInfo GetSpecs();

    STBImageReturn* LoadImage(const char* path) {
        STBImageReturn* stb = new STBImageReturn;
        stb->stbdata = stbi_load(path, &stb->w, &stb->h, &stb->c, 4);
        if (!stb->stbdata) {
            std::cerr << "> Couldn't load image: " << path << "\n";
            exit(-1);
        }

        return stb;
    };

    void PrintNixbornStyle(STBImageReturn* stbir, int width, int height) {
        ChafaCanvasConfig* cfg = chafa_canvas_config_new();
        chafa_canvas_config_set_geometry(cfg, width, height);

        ChafaCanvas* canvas = chafa_canvas_new(cfg);
        

        chafa_canvas_draw_all_pixels(
            canvas,
            CHAFA_PIXEL_RGBA8_UNASSOCIATED,
            stbir->stbdata,
            stbir->w,
            stbir->h,
            stbir->w * 4
        );

        ChafaTermInfo* ti = chafa_term_info_new();

        GString **rows = nullptr;
        gint len = 0;

        chafa_canvas_print_rows(canvas, ti, &rows, &len);

        SystemInfo sysi;
        try {
            sysi = GetSpecs();
        } catch (const std::exception& e) {
            std::cerr << "> Warning: " << e.what() << "\n";
        }

        std::vector<std::string> info;

        // ---------------- OS ----------------
        info.push_back("OS: " + (sysi.os.empty() ? "Unknown" : sysi.os));
        info.push_back("Kernel: " + (sysi.kernel.empty() ? "Unknown" : sysi.kernel));
        info.push_back("Arch: " + std::to_string(sysi.bits) + "-bit");

        // ---------------- CPU ----------------
        if (!sysi.cpu_array.empty()) {
            const auto& cpu = sysi.cpu_array[0];

            info.push_back("CPU: " + cpu.model);
            info.push_back("Cores: " + std::to_string(cpu.cores));
            info.push_back("Threads: " + std::to_string(cpu.threads));
            info.push_back("Clock: " + std::to_string(cpu.clock) + " MHz");
        }

        // ---------------- RAM ----------------
        if (!sysi.memory_array.empty()) {
            const auto& mem = sysi.memory_array[0];

            double totalGB = mem.total / (1024.0 * 1024 * 1024);
            double usedGB  = mem.used  / (1024.0 * 1024 * 1024);
            double freeGB  = mem.free  / (1024.0 * 1024 * 1024);

            info.push_back("RAM Total: " + std::to_string(totalGB) + " GB");
            info.push_back("RAM Used: "  + std::to_string(usedGB)  + " GB");
            info.push_back("RAM Free: "  + std::to_string(freeGB)  + " GB");
        }

        // ---------------- GPU ----------------
        if (!sysi.gpu_array.empty()) {
            for (const auto& gpu : sysi.gpu_array) {
                double vramGB = gpu.vram / (1024.0 * 1024 * 1024);
                double sharedGB = gpu.shared / (1024.0 * 1024 * 1024);

                info.push_back("GPU: " + gpu.model);
                info.push_back("VRAM: " + std::to_string(vramGB) + " GB");
                info.push_back("Shared: " + std::to_string(sharedGB) + " GB");
            }
        }

        // ---------------- DISK ----------------

        double totalDiskGB = 0.0;
        for (const auto& disk : sysi.disk_array) {
            totalDiskGB += disk.total;
        }
        info.push_back("Disk: " + std::to_string(totalDiskGB) + " GB");

        int max = std::max(len, (int)info.size());

        const int LEFT_COL = 10;

        for (int i = 0; i < max; i++) {

            std::string left;
            if (i < len && rows[i])
                left = rows[i]->str;
            else
                left = "";

            std::string right = (i < info.size()) ? info[i] : "";

            std::cout << "  ";

            if (!left.empty()) {
                std::cout << left;
            } else {
                std::cout << std::string(LEFT_COL, ' ');
            }

            std::cout << std::string(LEFT_COL, ' ')
                    << "☰    ";

            if (!right.empty()) {
                std::cout << right;
            } else {
                std::cout << "";
            }

            std::cout << "\n";
        }

        for (int i = 0; i < len; i++) {
            g_string_free(rows[i], TRUE);
        }

        g_free(rows);

        chafa_term_info_unref(ti);
        chafa_canvas_config_unref(cfg);
    };

    void PrintImage(STBImageReturn* stbir, int width, int height) {
        ChafaCanvasConfig* cfg = chafa_canvas_config_new();
        chafa_canvas_config_set_geometry(cfg, width, height);

        ChafaCanvas* canvas = chafa_canvas_new(cfg);

        chafa_canvas_draw_all_pixels(
            canvas,
            CHAFA_PIXEL_RGBA8_UNASSOCIATED,
            stbir->stbdata,
            stbir->w,
            stbir->h,
            stbir->w * 4
        );

        ChafaTermInfo* ti = chafa_term_info_new();

        GString **rows = nullptr;
        gint len = 0;

        chafa_canvas_print_rows(canvas, ti, &rows, &len);

        for (int i = 0; i < len; i++)
        {
            std::cout << rows[i]->str << "\n";
            g_string_free(rows[i], TRUE);
        }

        g_free(rows);

        chafa_term_info_unref(ti);
        chafa_canvas_config_unref(cfg);
    };
    
    SystemInfo GetSpecs() {
        SystemInfo sysi;

        sysi.cpu_array.clear();
        sysi.gpu_array.clear();
        sysi.memory_array.clear();
        sysi.disk_array.clear();

        for (const auto& cpu : hwinfo::getAllCPUs()) {
            CPUInfo cpui;
            cpui.cores = cpu.cores().size();
            cpui.model = cpu.modelName();

            double sum = 0;
            for (const auto& core : cpu.cores())
                sum += core.regular_frequency_hz;

            auto cores = cpu.cores().size();
            cpui.clock = (cores ? (sum / cores) : 0) / 1'000'000;
            cpui.threads = std::thread::hardware_concurrency();

            sysi.cpu_array.push_back(cpui);
        }

        // GPU
        for (const auto& gpu : hwinfo::getAllGPUs()) {
            GPUInfo gpui;
            gpui.model = gpu.name();
            gpui.vram = gpu.dedicated_memory_Bytes();
            gpui.cores = gpu.num_cores();
            gpui.shared = gpu.shared_memory_Bytes();
            gpui.frequency = gpu.frequency_hz();

            sysi.gpu_array.push_back(gpui);
        }

        MemoryInfo memi;
        hwinfo::Memory mem;

        memi.free = mem.free();
        memi.total = mem.size();
        memi.used = mem.size() - mem.available();

        sysi.memory_array.push_back(memi);

        std::vector<hwinfo::Disk> disks;

        try {
            
        } catch (const std::exception& e) {
            std::cerr << "> Disk scan failed: " << e.what() << "\n";
        }

        hwinfo::OS os;
        sysi.os = os.name();
        sysi.kernel = os.kernel();
        sysi.bits = os.is32bit() ? 32 : 64;

        return sysi;
    }

    
}