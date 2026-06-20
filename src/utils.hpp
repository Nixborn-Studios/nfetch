#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include "globals.hpp"
#include <chafa.h>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <filesystem>
#include <fstream>
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
    }

    static std::string CleanGPU(const std::string& s) {
        // "Lexa PRO [Radeon RX 550]" -> "Radeon RX 550"
        size_t open = s.find('[');
        size_t close = s.find(']', open);

        if (open == std::string::npos || close == std::string::npos || close <= open)
            return s;

        std::string result = s.substr(open + 1, close - open - 1);
        std::string trailing = s.substr(close + 1);

        size_t FirstNonSpace = trailing.find_first_not_of(" \t");
        if (FirstNonSpace != std::string::npos)
            result += " " + trailing.substr(FirstNonSpace);

        return result;
    }

    enum class InfoColor {
        Cpu, Gpu, Ram, Disk, Meta
    };

    static const char* ColorCode(InfoColor c) {
        switch (c) {
            case InfoColor::Cpu:  return "\033[38;2;255;85;85m";   // czerwony
            case InfoColor::Gpu:  return "\033[38;2;85;220;255m";  // cyan
            case InfoColor::Ram:  return "\033[38;2;120;220;120m"; // zielony
            case InfoColor::Disk: return "\033[38;2;255;210;80m";  // żółty
            default:              return "\033[38;2;180;180;180m"; // szary (Meta)
        }
    }

    static constexpr const char* ResetColor = "\033[0m";

#ifdef __linux__
    // pci.ids, same db lspci uses. skips vendor name on purpose,
    // AMD's vendor line has its own [AMD/ATI] bracket that breaks CleanGPU
    static bool LookupPciName(const std::string& VendorHex, const std::string& DeviceHex, std::string& OutDevice) {
        static const char* paths[] = {
            "/usr/share/hwdata/pci.ids",
            "/usr/share/misc/pci.ids",
            "/usr/share/pci.ids"
        };

        for (const char* path : paths) {
            std::ifstream f(path);
            if (!f.is_open()) continue;

            std::string line;
            bool InVendor = false;

            while (std::getline(f, line)) {
                if (line.empty() || line[0] == '#') continue;

                if (line[0] != '\t') {
                    InVendor = (line.size() >= 4 && line.substr(0, 4) == VendorHex);
                    continue;
                }

                if (InVendor && line.size() > 5 && line[1] != '\t') {
                    std::string id = line.substr(1, 4);
                    if (id == DeviceHex) {
                        size_t sp = line.find(' ', 6);
                        OutDevice = sp != std::string::npos ? line.substr(sp + 1) : "";
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // vfio-pci (passthrough) GPUs are invisible to hwinfo, read sysfs directly
    static void AppendPassthroughGPUs(std::vector<GPUInfo>& gpus) {
        const std::filesystem::path base = "/sys/bus/pci/devices";
        if (!std::filesystem::exists(base)) return;

        static const std::vector<std::string> DisplayDrivers = {
            "amdgpu", "nouveau", "nvidia", "nvidia_drm", "i915", "xe", "radeon"
        };

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(base, ec)) {
            std::ifstream ClassFile(entry.path() / "class");
            std::string ClassId;
            if (!ClassFile.is_open() || !(ClassFile >> ClassId)) continue;

            // 0x03xxxx = display controller
            if (ClassId.size() < 4 || ClassId.substr(0, 4) != "0x03") continue;

            std::ifstream VendorFile(entry.path() / "vendor");
            std::ifstream DeviceFile(entry.path() / "device");
            std::string VendorId, DeviceId;
            if (!(VendorFile >> VendorId) || !(DeviceFile >> DeviceId)) continue;

            if (VendorId.size() > 2) VendorId = VendorId.substr(2);
            if (DeviceId.size() > 2) DeviceId = DeviceId.substr(2);

            std::string driver = "unbound";
            std::error_code linkEc;
            auto DriverLink = std::filesystem::read_symlink(entry.path() / "driver", linkEc);
            if (!linkEc) driver = DriverLink.filename().string();

            bool OwnedByDisplayDriver = false;
            for (const auto& d : DisplayDrivers) {
                if (driver == d) { OwnedByDisplayDriver = true; break; }
            }
            if (OwnedByDisplayDriver) continue;

            std::string deviceName;
            bool found = LookupPciName(VendorId, DeviceId, deviceName);

            GPUInfo gpui;
            gpui.model = (found ? deviceName : (VendorId + ":" + DeviceId)) + " (" + driver + ")";
            gpui.vram = 0;
            gpui.shared = 0;
            gpui.cores = 0;
            gpui.frequency = 0;

            gpus.push_back(gpui);
        }
    }
#endif

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

        info.push_back(std::string(ColorCode(InfoColor::Meta)) + "OS: " + (sysi.os.empty() ? "Unknown" : sysi.os) + ResetColor);
        info.push_back(std::string(ColorCode(InfoColor::Meta)) + "Kernel: " + (sysi.kernel.empty() ? "Unknown" : sysi.kernel) + ResetColor);
        info.push_back(std::string(ColorCode(InfoColor::Meta)) + "Arch: " + std::to_string(sysi.bits) + "-bit" + ResetColor);

        if (!sysi.cpu_array.empty()) {
            const auto& cpu = sysi.cpu_array[0];
            info.push_back(std::string(ColorCode(InfoColor::Cpu)) + "CPU: " + cpu.model + ResetColor);
            info.push_back(std::string(ColorCode(InfoColor::Cpu)) + " > Cores: " + std::to_string(cpu.cores) + ResetColor);
            info.push_back(std::string(ColorCode(InfoColor::Cpu)) + " > Threads: " + std::to_string(cpu.threads) + ResetColor);
            info.push_back(std::string(ColorCode(InfoColor::Cpu)) + " > Clock: " + std::to_string(cpu.clock) + " MHz" + ResetColor);
        }

        if (!sysi.memory_array.empty()) {
            const auto& mem = sysi.memory_array[0];

            double TotalGB = mem.total / (1024.0 * 1024 * 1024);
            double UsedGB  = mem.used  / (1024.0 * 1024 * 1024);
            double FreeGB  = mem.free  / (1024.0 * 1024 * 1024);

            info.push_back(std::string(ColorCode(InfoColor::Ram)) + "RAM Total: " + std::to_string(TotalGB) + " GB" + ResetColor);
            info.push_back(std::string(ColorCode(InfoColor::Ram)) + "RAM Used: "  + std::to_string(UsedGB)  + " GB" + ResetColor);
            info.push_back(std::string(ColorCode(InfoColor::Ram)) + "RAM Free: "  + std::to_string(FreeGB)  + " GB" + ResetColor);
        }

        double TotalDiskGB = 0.0;
        for (const auto& d : sysi.disk_array)
            TotalDiskGB += d.total;

        info.push_back(std::string(ColorCode(InfoColor::Disk)) + "Disk: " + std::to_string(TotalDiskGB) + " GB" + ResetColor);

        if (!sysi.gpu_array.empty()) {
            for (const auto& gpu : sysi.gpu_array) {
                std::string name = CleanGPU(gpu.model);

                if (name.size() > 50)
                    name = name.substr(0, 47) + "...";

                double VRAMGB = gpu.vram / (1024.0 * 1024 * 1024);
                double SharedGB = gpu.shared / (1024.0 * 1024 * 1024);

                info.push_back(std::string(ColorCode(InfoColor::Gpu)) + "GPU: " + name + ResetColor);

                if (VRAMGB > 0)
                    info.push_back(std::string(ColorCode(InfoColor::Gpu)) + "VRAM: " + std::to_string(VRAMGB) + " GB" + ResetColor);

                if (SharedGB > 0)
                    info.push_back(std::string(ColorCode(InfoColor::Gpu)) + "Shared: " + std::to_string(SharedGB) + " GB" + ResetColor);
            }
        }

        const int LEFT_COL = 10;
        int max = std::max(len, (int)info.size());

        for (int i = 0; i < max; i++) {

            std::string left = (i < len && rows[i]) ? rows[i]->str : "";
            std::string right = (i < info.size()) ? info[i] : "";

            std::cout << "  ";

            if (!left.empty())
                std::cout << left;
            else
                std::cout << std::string(LEFT_COL, ' ');

            std::cout << std::string(LEFT_COL, ' ') << "☰    " << right << "\n";
        }

        for (int i = 0; i < len; i++)
            g_string_free(rows[i], TRUE);

        g_free(rows);

        chafa_term_info_unref(ti);
        chafa_canvas_config_unref(cfg);
    }

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

        for (int i = 0; i < len; i++) {
            std::cout << rows[i]->str << "\n";
            g_string_free(rows[i], TRUE);
        }

        g_free(rows);

        chafa_term_info_unref(ti);
        chafa_canvas_config_unref(cfg);
    }

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
            for (const auto& c : cpu.cores())
                sum += c.regular_frequency_hz;

            size_t cores = cpu.cores().size();
            cpui.clock = cores ? (sum / cores) / 1'000'000 : 0;
            cpui.threads = std::thread::hardware_concurrency();

            sysi.cpu_array.push_back(cpui);
        }

        for (const auto& gpu : hwinfo::getAllGPUs()) {
            GPUInfo gpui;
            gpui.model = gpu.name();
            gpui.vram = gpu.dedicated_memory_Bytes();
            gpui.cores = gpu.num_cores();
            gpui.shared = gpu.shared_memory_Bytes();
            gpui.frequency = gpu.frequency_hz();

            sysi.gpu_array.push_back(gpui);
        }

#ifdef __linux__
        AppendPassthroughGPUs(sysi.gpu_array);
#endif

        hwinfo::Memory mem;
        MemoryInfo memi;

        memi.free  = mem.available();
        memi.total = mem.size();
        memi.used  = mem.size() - mem.available();

        sysi.memory_array.push_back(memi);

        try {
            auto all = hwinfo::getAllDisks();

            for (const auto& d : all) {
                if (d.size() <= 0) continue;

                DiskInfo di;
                di.total = d.size() / (1024.0 * 1024 * 1024);

                sysi.disk_array.push_back(di);
            }
        } catch (...) {}

        // hwinfo sometimes returns nothing here
        if (sysi.disk_array.empty()) {
            try {
                std::error_code ec;
                auto space = std::filesystem::space("/", ec);

                if (!ec && space.capacity > 0) {
                    DiskInfo di;
                    di.total = static_cast<double>(space.capacity) / (1024.0 * 1024 * 1024);
                    sysi.disk_array.push_back(di);
                }
            } catch (...) {}
        }

        hwinfo::OS os;
        sysi.os = os.name();
        sysi.kernel = os.kernel();
        sysi.bits = os.is32bit() ? 32 : 64;

        return sysi;
    }

}