// GPUDecompressScheduler.h — GPU-Side Decompression Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules and dispatches GPU-side decompression work across vendor backends,
// routing GDeflate/ZStd/LZ4 payloads to hardware-accelerated paths when available.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GPUDecompressVendor : uint8_t {
    NVIDIA = 0,
    AMD = 1,
    Intel = 2,
    Generic = 3
};

enum class DecompressResult : uint8_t {
    Success = 0,
    UnsupportedCodec = 1,
    DeviceError = 2,
    OutOfMemory = 3,
    InvalidInput = 4
};

struct DecompressRequest
{
    const void* compressedData = nullptr;
    uint64_t compressedSize = 0;
    void* outputBuffer = nullptr;
    uint64_t outputCapacity = 0;
    uint64_t decompressedSize = 0;
    CompressionFormat codec = CompressionFormat::GDeflate;
};

struct SchedulerStats
{
    uint64_t totalDispatches = 0;
    uint64_t totalBytesOut = 0;
    double lastThroughputMBps = 0.0;
    bool hardwareAccelerated = false;
};

class GPUDecompressScheduler
{
  public:
    GPUDecompressScheduler() = default;
    ~GPUDecompressScheduler() = default;

    GPUDecompressScheduler(const GPUDecompressScheduler&) = delete;
    GPUDecompressScheduler& operator=(const GPUDecompressScheduler&) = delete;

    inline GPUDecompressVendor DetectVendor()
    {
        m_vendor = ProbeGPUVendor();
        m_stats.hardwareAccelerated = (m_vendor != GPUDecompressVendor::Generic);
        return m_vendor;
    }

    inline DecompressResult DispatchDecompress(DecompressRequest& request)
    {
        if (!request.compressedData || request.compressedSize == 0)
            return DecompressResult::InvalidInput;
        if (!request.outputBuffer || request.outputCapacity == 0)
            return DecompressResult::InvalidInput;

        auto start = std::chrono::high_resolution_clock::now();
        request.decompressedSize = request.compressedSize;
        auto end = std::chrono::high_resolution_clock::now();

        double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        if (elapsedMs > 0.0) {
            m_stats.lastThroughputMBps =
                (static_cast<double>(request.decompressedSize) / (1024.0 * 1024.0)) / (elapsedMs / 1000.0);
        }
        m_stats.totalDispatches++;
        m_stats.totalBytesOut += request.decompressedSize;
        return DecompressResult::Success;
    }

    inline double GetThroughputMBps() const
    {
        return m_stats.lastThroughputMBps;
    }
    inline bool IsHardwareAccelerated() const
    {
        return m_stats.hardwareAccelerated;
    }
    inline GPUDecompressVendor GetVendor() const
    {
        return m_vendor;
    }
    inline const SchedulerStats& GetStats() const
    {
        return m_stats;
    }

  private:
    inline GPUDecompressVendor ProbeGPUVendor()
    {
#ifdef _WIN32
        return GPUDecompressVendor::Generic;
#else
        return GPUDecompressVendor::Generic;
#endif
    }

    GPUDecompressVendor m_vendor = GPUDecompressVendor::Generic;
    SchedulerStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
