// GPUPowerStateManager.h — GPU Power-Aware Routing
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors GPU power state via DXGI adapter enumeration and routes
// simple thumbnail operations to the integrated GPU, avoiding discrete
// GPU wake-ups that waste power for trivial work.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

namespace ExplorerLens {
namespace Engine {

/// GPU adapter type classification
enum class GPUAdapterType : uint8_t {
    Unknown = 0,
    Integrated,     // iGPU (Intel UHD, AMD Vega, etc.)
    Discrete,       // dGPU (NVIDIA GeForce, AMD Radeon, etc.)
    Software        // WARP / software renderer
};

/// Routing decision for thumbnail work
enum class GPURouteTarget : uint8_t {
    Integrated,     // Use iGPU to save power
    Discrete,       // Use dGPU for complex work
    CPU,            // Fall back to CPU
    Auto            // Let driver decide
};

/// Per-adapter information for power-aware routing
struct PowerAdapterInfo {
    std::wstring   description;
    uint32_t       vendorId = 0;
    uint32_t       deviceId = 0;
    uint64_t       dedicatedVideoMB = 0;
    uint64_t       sharedSystemMB = 0;
    GPUAdapterType type = GPUAdapterType::Unknown;
    bool           available = false;
    uint32_t       adapterIndex = 0;
};

/// Power-aware routing statistics
struct GPUPowerStats {
    uint64_t integratedRouted = 0;
    uint64_t discreteRouted = 0;
    uint64_t cpuFallbackRouted = 0;
    uint64_t discreteWakeAvoided = 0;
    uint32_t adapterCount = 0;
};

/// GPU power state manager for energy-efficient thumbnail rendering
class GPUPowerStateManager {
public:
    static GPUPowerStateManager& Instance() {
        static GPUPowerStateManager inst;
        return inst;
    }

    /// Enumerate all GPU adapters via DXGI
    bool EnumerateAdapters() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_adapters.clear();

        IDXGIFactory* factory = nullptr;
        HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory),
            reinterpret_cast<void**>(&factory));
        if (FAILED(hr) || !factory) return false;

        IDXGIAdapter* adapter = nullptr;
        for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC desc{};
            adapter->GetDesc(&desc);

            PowerAdapterInfo info;
            info.description = desc.Description;
            info.vendorId = desc.VendorId;
            info.deviceId = desc.DeviceId;
            info.dedicatedVideoMB = desc.DedicatedVideoMemory / (1024 * 1024);
            info.sharedSystemMB = desc.SharedSystemMemory / (1024 * 1024);
            info.adapterIndex = i;
            info.available = true;
            info.type = ClassifyAdapter(desc);

            m_adapters.push_back(info);
            adapter->Release();
        }
        factory->Release();
        m_stats.adapterCount = static_cast<uint32_t>(m_adapters.size());
        return !m_adapters.empty();
    }

    /// Decide which GPU to route a thumbnail operation to
    GPURouteTarget RouteWork(uint32_t imageSizeBytes, bool requiresCompute) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_adapters.empty()) {
            m_stats.cpuFallbackRouted++;
            return GPURouteTarget::CPU;
        }

        bool hasIntegrated = false, hasDiscrete = false;
        for (const auto& a : m_adapters) {
            if (a.type == GPUAdapterType::Integrated) hasIntegrated = true;
            if (a.type == GPUAdapterType::Discrete) hasDiscrete = true;
        }

        // Small thumbnails (< 4 MB) → prefer integrated GPU to save power
        constexpr uint32_t IGPU_THRESHOLD = 4 * 1024 * 1024;
        if (!requiresCompute && imageSizeBytes < IGPU_THRESHOLD && hasIntegrated) {
            m_stats.integratedRouted++;
            if (hasDiscrete) m_stats.discreteWakeAvoided++;
            return GPURouteTarget::Integrated;
        }

        // Large images or compute-heavy work → use discrete GPU
        if (hasDiscrete) {
            m_stats.discreteRouted++;
            return GPURouteTarget::Discrete;
        }

        if (hasIntegrated) {
            m_stats.integratedRouted++;
            return GPURouteTarget::Integrated;
        }

        m_stats.cpuFallbackRouted++;
        return GPURouteTarget::CPU;
    }

    /// Get adapter info
    std::vector<PowerAdapterInfo> GetAdapters() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_adapters;
    }

    GPUPowerStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

private:
    GPUPowerStateManager() = default;

    static GPUAdapterType ClassifyAdapter(const DXGI_ADAPTER_DESC& desc) {
        // Intel: 0x8086, AMD: 0x1002, NVIDIA: 0x10DE, Microsoft (WARP): 0x1414
        if (desc.VendorId == 0x1414) return GPUAdapterType::Software;

        // Heuristic: < 512 MB dedicated VRAM → integrated
        uint64_t dedicatedMB = desc.DedicatedVideoMemory / (1024 * 1024);
        if (dedicatedMB < 512) return GPUAdapterType::Integrated;

        // Intel GPUs are almost always integrated
        if (desc.VendorId == 0x8086 && dedicatedMB < 2048)
            return GPUAdapterType::Integrated;

        return GPUAdapterType::Discrete;
    }

    mutable std::mutex              m_mutex;
    std::vector<PowerAdapterInfo>     m_adapters;
    mutable GPUPowerStats           m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
