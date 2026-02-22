//==============================================================================
// DarkThumbs Engine — Sprint 300: GPU Pipeline V3
// Enhanced DirectX 12 pipeline with mesh shaders, enhanced barriers, and
// GPU-driven dispatch for high-throughput thumbnail rendering.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// GPU Pipeline V3 features
enum class GPUV3Feature : uint8_t {
    MeshShaders = 0,
    EnhancedBarriers,
    GPUDrivenDispatch,
    DirectStorage,
    PipelineLibrary,
    WorkGraphs,
    COUNT
};

/// Pipeline V3 queue type
enum class PipelineV3Queue : uint8_t {
    DirectGraphics = 0,
    Compute,
    Copy,
    VideoDecode,
    COUNT
};

/// GPU V3 performance tier
enum class GPUV3PerfTier : uint8_t {
    Tier0_Legacy = 0,   // D3D11 fallback
    Tier1_Basic,        // D3D12 basic
    Tier2_Enhanced,     // D3D12 + mesh shaders
    Tier3_Advanced,     // Full D3D12 + work graphs
    COUNT
};

/// Pipeline V3 descriptor
struct PipelineV3Descriptor {
    GPUV3PerfTier   tier            = GPUV3PerfTier::Tier1_Basic;
    uint32_t        maxFramesInFlight = 3;
    bool            enableMeshShaders = false;
    bool            enableWorkGraphs  = false;
    bool            enableDirectStorage = false;
};

/// GPU Pipeline V3 manager
class GPUPipelineV3 {
public:
    static const wchar_t* FeatureName(GPUV3Feature f) {
        switch (f) {
            case GPUV3Feature::MeshShaders:        return L"Mesh Shaders";
            case GPUV3Feature::EnhancedBarriers:   return L"Enhanced Barriers";
            case GPUV3Feature::GPUDrivenDispatch:  return L"GPU-Driven Dispatch";
            case GPUV3Feature::DirectStorage:      return L"DirectStorage";
            case GPUV3Feature::PipelineLibrary:    return L"Pipeline Library";
            case GPUV3Feature::WorkGraphs:         return L"Work Graphs";
            default: return L"Unknown";
        }
    }

    static const wchar_t* QueueName(PipelineV3Queue q) {
        switch (q) {
            case PipelineV3Queue::DirectGraphics: return L"Direct/Graphics";
            case PipelineV3Queue::Compute:        return L"Compute";
            case PipelineV3Queue::Copy:           return L"Copy";
            case PipelineV3Queue::VideoDecode:    return L"Video Decode";
            default: return L"Unknown";
        }
    }

    static const wchar_t* PerfTierName(GPUV3PerfTier t) {
        switch (t) {
            case GPUV3PerfTier::Tier0_Legacy:   return L"Legacy (D3D11)";
            case GPUV3PerfTier::Tier1_Basic:    return L"Basic (D3D12)";
            case GPUV3PerfTier::Tier2_Enhanced: return L"Enhanced (Mesh Shaders)";
            case GPUV3PerfTier::Tier3_Advanced: return L"Advanced (Work Graphs)";
            default: return L"Unknown";
        }
    }

    static constexpr size_t FeatureCount() { return static_cast<size_t>(GPUV3Feature::COUNT); }
    static constexpr size_t QueueCount()   { return static_cast<size_t>(PipelineV3Queue::COUNT); }
    static constexpr size_t TierCount()    { return static_cast<size_t>(GPUV3PerfTier::COUNT); }

    static PipelineV3Descriptor DefaultDescriptor() {
        PipelineV3Descriptor d;
        d.tier = GPUV3PerfTier::Tier1_Basic;
        d.maxFramesInFlight = 3;
        return d;
    }

    static bool ValidateDescriptor(const PipelineV3Descriptor& d) {
        return d.maxFramesInFlight >= 1 && d.maxFramesInFlight <= 8;
    }
};

}} // namespace DarkThumbs::Engine
