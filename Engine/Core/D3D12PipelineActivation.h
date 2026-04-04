//==============================================================================
// ExplorerLens Engine — D3D12 Pipeline Activation
// Activate D3D12ComputePipeline for real GPU workloads with D3D11 fallback.
// Benchmark comparison and runtime selection logic.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// GPU backend type
enum class GPUBackend : uint8_t {
    Auto,      // Runtime selection based on hardware
    D3D12,     // Direct3D 12 — compute pipeline, higher throughput
    D3D11,     // Direct3D 11 — primary, widely supported
    CPU,       // Force CPU / software fallback
    None,      // No backend selected (initial state)
    Disabled,  // GPU disabled by policy
    GDI,       // GDI+ — legacy CPU fallback
    Vulkan,    // Vulkan — cross-platform path
    Software   // Pure software renderer — last resort
};

/// GPU feature level requirements
enum class D3DFeatureLevel : uint8_t {
    Level_11_0,  // D3D11 minimum
    Level_11_1,  // D3D11.1 with compute
    Level_12_0,  // D3D12 base
    Level_12_1,  // D3D12 with bindless
    Level_12_2,  // D3D12 Ultimate (mesh shaders, RT)
    Unknown
};

/// GPU adapter info
struct GPUAdapterInfo
{
    std::wstring name;
    uint64_t dedicatedVideoMemory = 0;
    uint64_t dedicatedSystemMemory = 0;
    uint64_t sharedSystemMemory = 0;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
    D3DFeatureLevel featureLevel = D3DFeatureLevel::Unknown;
    GPUBackend selectedBackend = GPUBackend::Auto;
    bool supportsD3D12 = false;
    bool supportsCompute = false;
};

/// D3D12 pipeline activation settings
struct D3D12ActivationConfig
{
    uint64_t minVRAM = 512 * 1024 * 1024ULL;  // 512 MB minimum
    bool preferD3D12 = true;
    bool allowFallback = true;
    uint32_t computeThreadGroupX = 16;
    uint32_t computeThreadGroupY = 16;
    uint32_t maxConcurrentDispatches = 4;
};

/// D3D12 pipeline activation manager
class D3D12PipelineActivation
{
  public:
    /// Backend display name
    static const wchar_t* BackendName(GPUBackend b)
    {
        switch (b) {
            case GPUBackend::Auto:
                return L"Auto";
            case GPUBackend::D3D12:
                return L"Direct3D 12";
            case GPUBackend::D3D11:
                return L"Direct3D 11";
            case GPUBackend::CPU:
                return L"CPU";
            case GPUBackend::None:
                return L"None";
            case GPUBackend::Disabled:
                return L"Disabled";
            case GPUBackend::GDI:
                return L"GDI+ (CPU)";
            case GPUBackend::Vulkan:
                return L"Vulkan";
            case GPUBackend::Software:
                return L"Software";
            default:
                return L"Unknown";
        }
    }

    /// Feature level name
    static const wchar_t* FeatureLevelName(D3DFeatureLevel fl)
    {
        switch (fl) {
            case D3DFeatureLevel::Level_11_0:
                return L"11.0";
            case D3DFeatureLevel::Level_11_1:
                return L"11.1";
            case D3DFeatureLevel::Level_12_0:
                return L"12.0";
            case D3DFeatureLevel::Level_12_1:
                return L"12.1";
            case D3DFeatureLevel::Level_12_2:
                return L"12.2";
            default:
                return L"Unknown";
        }
    }

    /// Select best backend based on adapter info
    static GPUBackend SelectBackend(const GPUAdapterInfo& adapter, const D3D12ActivationConfig& cfg)
    {
        if (cfg.preferD3D12 && adapter.supportsD3D12 && adapter.dedicatedVideoMemory >= cfg.minVRAM)
            return GPUBackend::D3D12;
        if (adapter.featureLevel >= D3DFeatureLevel::Level_11_0)
            return GPUBackend::D3D11;
        if (cfg.allowFallback)
            return GPUBackend::GDI;
        return GPUBackend::Software;
    }

    /// Backend count
    static constexpr size_t BackendCount()
    {
        return 9;
    }

    /// Feature level count
    static constexpr size_t FeatureLevelCount()
    {
        return 5;
    }

    /// Validate activation config
    static bool ValidateConfig(const D3D12ActivationConfig& cfg)
    {
        if (cfg.minVRAM < 128 * 1024 * 1024ULL)
            return false;
        if (cfg.computeThreadGroupX == 0 || cfg.computeThreadGroupY == 0)
            return false;
        if (cfg.maxConcurrentDispatches == 0 || cfg.maxConcurrentDispatches > 16)
            return false;
        return true;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
