#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <globals.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include "hwinfo/hwinfo.h"

SystemInfo sysi;

namespace NFETCH {
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

        int indent = 50;

        std::vector<std::string> info = {
            "OS: Linux",
            "CPU: Ryzen",
            "RAM: 16GB"
        };

        int max = std::max(len, (int)info.size());

        const int LEFT_COL = 10;

        for (int i = 0; i < max; i++)
        {
            std::string left  = (i < len) ? rows[i]->str : "";
            std::string right = (i < info.size()) ? info[i] : "";

            std::cout << "  " << left;

            int pad = LEFT_COL;

            std::cout << std::string(pad, ' ')
                    << "☰    "
                    << right
                    << "\n";
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
    
    SystemInfo GetSpecs(){
        // CPU
        for (const auto& cpu : hwinfo::getAllCPUs()) {
            CPUInfo cpui;
            cpui.cores = cpu.cores().size();
            cpui.model = cpu.modelName();

            // ------------------------------------------------------- //

            double sum = 0;
            size_t count = 0;

            for (const auto& core : cpu.cores()) {
                sum += core.regular_frequency_hz;
                count++;
            }

            cpui.clock = (count > 0) ? (sum / count) : 0;

            // ------------------------------------------------------- //

            cpui.threads = cpu.numLogicalCores();

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
        
        return sysi;
    }

    
}//  TUTAJ PATRZ ja pierdole
// - Zrob kurwa jebane funkcje to getowania tego (GetCPU, GetGPU, GetRAM, GetDisk, GetSystemInfo) ktore
// zwraca odpowiedni struct, np GetCPU zwraca struct CPUInfo* i wtedy z tego sobie bierzemy np cpu->model i printujemy

// dobra pokaze ci jak