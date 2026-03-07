// SystemInfoCollector.h — System Environment Discovery
// Copyright (c) 2026 ExplorerLens Project
//
// Collects comprehensive system information including OS version, hardware
// specs, driver versions, and display configuration for diagnostics.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CPUInfo {
    std::string brand;
    uint32_t physicalCores = 0;
    uint32_t logicalCores = 0;
    uint32_t baseClockMHz = 0;
    bool hasSSE42 = false;
    bool hasAVX2 = false;
    bool hasAVX512 = false;
    std::string architecture; // "x64", "ARM64"
};

struct SystemGPUInfo {
    std::string name;
    std::string driverVersion;
    uint64_t dedicatedMemoryMB = 0;
    std::string vendor;
    uint32_t driverDate = 0; // YYYYMMDD
};

struct DisplayInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t refreshRate = 0;
    uint32_t dpiScale = 100;
    bool isPrimary = false;
};

struct SystemSnapshot {
    // OS
    std::string osName;
    std::string osBuild;
    bool isServer = false;
    bool isARM64 = false;

    // Hardware
    CPUInfo cpu;
    uint64_t totalRAM_MB = 0;
    uint64_t availableRAM_MB = 0;

    // GPU
    std::vector<SystemGPUInfo> gpus;

    // Displays
    std::vector<DisplayInfo> displays;

    // Storage
    uint64_t systemDiskFreeMB = 0;
    bool isSSD = false;

    // Power
    bool isOnBattery = false;
    uint32_t batteryPercent = 100;
};

class SystemInfoCollector {
public:
    bool IsLowEndSystem(const SystemSnapshot& info) const {
        return info.cpu.logicalCores <= 2 ||
            info.totalRAM_MB < 4096 ||
            info.gpus.empty();
    }

    bool HasDiscreteGPU(const SystemSnapshot& info) const {
        for (const auto& gpu : info.gpus) {
            if (gpu.dedicatedMemoryMB > 512) return true;
        }
        return false;
    }

    uint32_t RecommendedThreadCount(const SystemSnapshot& info) const {
        uint32_t cores = info.cpu.logicalCores;
        if (cores <= 2) return 1;
        if (cores <= 4) return 2;
        if (cores <= 8) return 4;
        return cores / 2;
    }

    uint64_t RecommendedCacheSizeMB(const SystemSnapshot& info) const {
        uint64_t ram = info.totalRAM_MB;
        if (ram < 4096) return 64;
        if (ram < 8192) return 128;
        if (ram < 16384) return 256;
        return 512;
    }

    std::string GetSummaryString(const SystemSnapshot& info) const {
        return info.osName + " | " + info.cpu.brand + " (" +
            std::to_string(info.cpu.logicalCores) + " threads) | " +
            std::to_string(info.totalRAM_MB) + "MB RAM" +
            (info.gpus.empty() ? "" : " | " + info.gpus[0].name);
    }
};

} // namespace Engine
} // namespace ExplorerLens
