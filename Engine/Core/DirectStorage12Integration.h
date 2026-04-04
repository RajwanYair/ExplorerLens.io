#pragma once
// DirectStorage12Integration.h — DirectStorage 1.2 Integration
// GPU-direct file I/O pipeline using DirectStorage API for
// bypassing CPU staging when loading textures to GPU memory.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// DirectStorage availability
enum class DStorageSupport : uint8_t {
    Unavailable = 0,  // OS or GPU doesn't support
    Version1_0,       // Basic GPU decompression
    Version1_1,       // Enhanced metadata queries
    Version1_2,       // GPU decompression + bulk submit
    Emulated,         // CPU fallback emulation
    COUNT
};

/// Decompression target
enum class DStorageDecompTarget : uint8_t {
    CPU = 0,      // Traditional CPU decompress
    GPU_Compute,  // GPU compute shader decompress
    GPU_Copy,     // GPU copy engine decompress
    Auto,         // Runtime selection based on load
    COUNT
};

struct DStorageQueueStats
{
    uint64_t submittedRequests = 0;
    uint64_t completedRequests = 0;
    uint64_t errorCount = 0;
    uint64_t bytesTransferred = 0;
    double avgLatencyMs = 0.0;
    double peakBandwidthMBps = 0.0;
    bool gpuDecompActive = false;
};

struct DStorageConfig
{
    bool enabled = false;
    DStorageDecompTarget decompTarget = DStorageDecompTarget::Auto;
    uint32_t queueDepth = 64;
    uint32_t stagingBufferMB = 32;
    bool bypassCPUStaging = true;
    bool enableCompression = true;
    bool forceEmulation = false;
};

class DirectStorage12Integration
{
  public:
    static constexpr size_t SupportCount()
    {
        return static_cast<size_t>(DStorageSupport::COUNT);
    }
    static constexpr size_t DecompTargetCount()
    {
        return static_cast<size_t>(DStorageDecompTarget::COUNT);
    }

    static const wchar_t* SupportName(DStorageSupport s)
    {
        switch (s) {
            case DStorageSupport::Unavailable:
                return L"Unavailable";
            case DStorageSupport::Version1_0:
                return L"DirectStorage 1.0";
            case DStorageSupport::Version1_1:
                return L"DirectStorage 1.1";
            case DStorageSupport::Version1_2:
                return L"DirectStorage 1.2";
            case DStorageSupport::Emulated:
                return L"Emulated (CPU)";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* DecompTargetName(DStorageDecompTarget t)
    {
        switch (t) {
            case DStorageDecompTarget::CPU:
                return L"CPU";
            case DStorageDecompTarget::GPU_Compute:
                return L"GPU Compute";
            case DStorageDecompTarget::GPU_Copy:
                return L"GPU Copy Engine";
            case DStorageDecompTarget::Auto:
                return L"Auto";
            default:
                return L"Unknown";
        }
    }

    /// Estimate bandwidth improvement vs traditional I/O (multiplier)
    static constexpr double EstimatedSpeedup(DStorageSupport support)
    {
        switch (support) {
            case DStorageSupport::Version1_2:
                return 3.5;
            case DStorageSupport::Version1_1:
                return 2.8;
            case DStorageSupport::Version1_0:
                return 2.0;
            case DStorageSupport::Emulated:
                return 1.0;
            default:
                return 1.0;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
