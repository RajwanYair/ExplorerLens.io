//==============================================================================
// ExplorerLens Engine — Multi-GPU Load Balancer
// Distribute thumbnail generation across multiple GPUs for throughput scaling.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// GPU load balancing strategy
enum class GPUBalanceStrategy : uint8_t {
    RoundRobin,         // Alternate between GPUs
    LeastLoaded,        // Route to least busy GPU
    MemoryAware,        // Route based on VRAM availability
    PowerEfficient,     // Prefer integrated GPU when possible
    PerformanceMax,     // Always use fastest GPU
    COUNT
};

/// GPU device type
enum class GPUDeviceType : uint8_t {
    Discrete,           // Dedicated GPU
    Integrated,         // CPU-integrated
    Software,           // Software renderer
    Virtual,            // Virtual GPU (cloud)
    COUNT
};

/// GPU device info
struct GPUDeviceInfo {
    uint32_t    deviceIndex     = 0;
    std::wstring deviceName;
    GPUDeviceType type          = GPUDeviceType::Discrete;
    uint64_t    vramTotal       = 0;
    uint64_t    vramAvailable   = 0;
    uint32_t    computeUnits    = 0;
    float       loadPercent     = 0.0f;
    bool        isAvailable     = true;
};

/// Load balancer statistics
struct GPUBalancerStats {
    uint32_t    totalGPUs       = 0;
    uint32_t    activeGPUs      = 0;
    uint64_t    totalDispatched = 0;
    double      avgLatencyMs    = 0;
    double      throughputPerSec = 0;
};

/// Multi-GPU load balancer config
struct MultiGPUConfig {
    GPUBalanceStrategy strategy = GPUBalanceStrategy::LeastLoaded;
    uint32_t maxGPUs            = 4;
    float    maxLoadPercent     = 90.0f;
    uint64_t minVRAMBytes       = 256 * 1024 * 1024;   // 256MB
    bool     enableFallback     = true;
    bool     preferDiscrete     = true;
};

/// Multi-GPU load balancer
class MultiGPULoadBalancer {
public:
    static const wchar_t* StrategyName(GPUBalanceStrategy s) {
        switch (s) {
            case GPUBalanceStrategy::RoundRobin:      return L"Round Robin";
            case GPUBalanceStrategy::LeastLoaded:     return L"Least Loaded";
            case GPUBalanceStrategy::MemoryAware:     return L"Memory Aware";
            case GPUBalanceStrategy::PowerEfficient:  return L"Power Efficient";
            case GPUBalanceStrategy::PerformanceMax:  return L"Performance Max";
            default: return L"Unknown";
        }
    }

    static const wchar_t* DeviceTypeName(GPUDeviceType t) {
        switch (t) {
            case GPUDeviceType::Discrete:   return L"Discrete";
            case GPUDeviceType::Integrated: return L"Integrated";
            case GPUDeviceType::Software:   return L"Software";
            case GPUDeviceType::Virtual:    return L"Virtual";
            default: return L"Unknown";
        }
    }

    static constexpr size_t StrategyCount() { return static_cast<size_t>(GPUBalanceStrategy::COUNT); }
    static constexpr size_t DeviceTypeCount() { return static_cast<size_t>(GPUDeviceType::COUNT); }

    static bool ValidateConfig(const MultiGPUConfig& cfg) {
        return cfg.maxGPUs > 0 && cfg.maxLoadPercent > 0.0f && cfg.maxLoadPercent <= 100.0f;
    }
};

}} // namespace ExplorerLens::Engine

