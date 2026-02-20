//==============================================================================
// DarkThumbs Engine — Sprint 274: Vulkan Compute Backend
// Complete VulkanComputePipeline for Linux/Wine compatibility path.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Vulkan feature requirements
enum class VulkanFeature : uint8_t {
    ComputeShader,      // Required — compute pipeline
    StorageImage,       // Required — image write
    SampledImage,       // Required — image read
    PushConstants,      // Optional — fast small data
    Descriptor,         // Required — resource binding
    Timeline,           // Optional — timeline semaphores
    DynamicRendering,   // Optional — Vulkan 1.3
    COUNT
};

/// Vulkan queue family types
enum class VulkanQueueType : uint8_t {
    Graphics,
    Compute,
    Transfer,
    SparseBinding,
    Present,
    COUNT
};

/// Vulkan adapter info
struct VulkanAdapterInfo {
    std::wstring deviceName;
    uint32_t     apiVersion      = 0;
    uint32_t     driverVersion   = 0;
    uint32_t     vendorId        = 0;
    uint32_t     deviceId        = 0;
    uint64_t     deviceMemory    = 0;
    bool         computeSupport  = false;
    bool         isDiscrete      = false;
};

/// Vulkan pipeline configuration
struct VulkanPipelineConfig {
    uint32_t workGroupSizeX     = 16;
    uint32_t workGroupSizeY     = 16;
    uint32_t maxImageDimension  = 16384;
    uint32_t maxDescriptorSets  = 4;
    bool     enableValidation   = false;
    bool     preferCompute      = true;
};

/// Vulkan compute backend activation
class VulkanComputeActivation {
public:
    /// Feature name
    static const wchar_t* FeatureName(VulkanFeature f) {
        switch (f) {
            case VulkanFeature::ComputeShader:    return L"Compute Shader";
            case VulkanFeature::StorageImage:     return L"Storage Image";
            case VulkanFeature::SampledImage:     return L"Sampled Image";
            case VulkanFeature::PushConstants:    return L"Push Constants";
            case VulkanFeature::Descriptor:       return L"Descriptor Binding";
            case VulkanFeature::Timeline:         return L"Timeline Semaphore";
            case VulkanFeature::DynamicRendering: return L"Dynamic Rendering";
            default: return L"Unknown";
        }
    }

    /// Queue type name
    static const wchar_t* QueueName(VulkanQueueType q) {
        switch (q) {
            case VulkanQueueType::Graphics:      return L"Graphics";
            case VulkanQueueType::Compute:       return L"Compute";
            case VulkanQueueType::Transfer:      return L"Transfer";
            case VulkanQueueType::SparseBinding: return L"Sparse Binding";
            case VulkanQueueType::Present:       return L"Present";
            default: return L"Unknown";
        }
    }

    /// Feature count
    static constexpr size_t FeatureCount() { return static_cast<size_t>(VulkanFeature::COUNT); }

    /// Queue type count
    static constexpr size_t QueueTypeCount() { return static_cast<size_t>(VulkanQueueType::COUNT); }

    /// Check if adapter meets minimum requirements
    static bool MeetsMinimumRequirements(const VulkanAdapterInfo& info) {
        return info.computeSupport && info.deviceMemory >= 256 * 1024 * 1024ULL;
    }

    /// Validate config
    static bool ValidateConfig(const VulkanPipelineConfig& cfg) {
        if (cfg.workGroupSizeX == 0 || cfg.workGroupSizeY == 0) return false;
        if (cfg.maxImageDimension == 0 || cfg.maxImageDimension > 65536) return false;
        return true;
    }
};

}} // namespace DarkThumbs::Engine
