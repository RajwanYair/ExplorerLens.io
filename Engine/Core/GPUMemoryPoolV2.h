//==============================================================================
// ExplorerLens Engine — GPU Memory Pool V2
// Defragmenting GPU heap allocator with tier-aware budgets, residency
// management, and background compaction for sustained rendering throughput.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// GPU memory heap type
enum class GPUHeapType : uint8_t {
    Default = 0,  // GPU-only (VRAM)
    Upload,       // CPU→GPU transfer
    Readback,     // GPU→CPU transfer
    Custom,       // UMA/custom heap
    COUNT
};

/// GPU residency priority
enum class GPUResidencyPriority : uint8_t {
    Low = 0,
    Normal,
    High,
    Critical,
    COUNT
};

/// Memory pool allocation strategy
enum class GPUAllocStrategy : uint8_t {
    BestFit = 0,
    FirstFit,
    PoolSlab,
    COUNT
};

/// GPU allocation record
struct GPUAllocation
{
    GPUHeapType heap = GPUHeapType::Default;
    GPUResidencyPriority priority = GPUResidencyPriority::Normal;
    uint64_t sizeBytes = 0;
    uint64_t offsetBytes = 0;
    bool resident = true;
};

/// Pool V2 statistics
struct GPUPoolV2Stats
{
    uint64_t totalAllocated = 0;
    uint64_t peakAllocated = 0;
    uint64_t budgetBytes = 0;
    uint32_t allocationCount = 0;
    uint32_t evictedCount = 0;
    float fragmentationPct = 0.0f;
};

/// GPU Memory Pool V2
class GPUMemoryPoolV2
{
  public:
    static const wchar_t* HeapTypeName(GPUHeapType t)
    {
        switch (t) {
            case GPUHeapType::Default:
                return L"Default (VRAM)";
            case GPUHeapType::Upload:
                return L"Upload (CPU→GPU)";
            case GPUHeapType::Readback:
                return L"Readback (GPU→CPU)";
            case GPUHeapType::Custom:
                return L"Custom (UMA)";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* ResidencyName(GPUResidencyPriority p)
    {
        switch (p) {
            case GPUResidencyPriority::Low:
                return L"Low";
            case GPUResidencyPriority::Normal:
                return L"Normal";
            case GPUResidencyPriority::High:
                return L"High";
            case GPUResidencyPriority::Critical:
                return L"Critical";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* StrategyName(GPUAllocStrategy s)
    {
        switch (s) {
            case GPUAllocStrategy::BestFit:
                return L"Best Fit";
            case GPUAllocStrategy::FirstFit:
                return L"First Fit";
            case GPUAllocStrategy::PoolSlab:
                return L"Pool Slab";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t HeapTypeCount()
    {
        return static_cast<size_t>(GPUHeapType::COUNT);
    }
    static constexpr size_t ResidencyCount()
    {
        return static_cast<size_t>(GPUResidencyPriority::COUNT);
    }
    static constexpr size_t AllocStrategyCount()
    {
        return static_cast<size_t>(GPUAllocStrategy::COUNT);
    }

    // Default VRAM budget: 512 MB
    static constexpr uint64_t DefaultBudgetBytes()
    {
        return 512ULL * 1024 * 1024;
    }

    static bool IsFragmented(const GPUPoolV2Stats& s)
    {
        return s.fragmentationPct > 20.0f;
    }

    static float ComputeUtilization(const GPUPoolV2Stats& s)
    {
        if (s.budgetBytes == 0)
            return 0.0f;
        return static_cast<float>(s.totalAllocated) / static_cast<float>(s.budgetBytes);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
