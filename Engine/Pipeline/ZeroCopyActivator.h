// ZeroCopyActivator.h — Activate Zero-Copy Decode to GPU Upload
// Copyright (c) 2026 ExplorerLens Project
//
// Manages zero-copy transfer from decode output directly to GPU staging buffers,
// eliminating intermediate CPU copies for improved throughput.
//
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ZCStage : uint8_t {
    Inactive = 0,
    DecodeReady = 1,
    Mapped = 2,
    Uploading = 3,
    GPUReady = 4,
    Failed = 5
};

struct ZeroCopyMetrics
{
    uint64_t totalTransfers = 0;
    uint64_t totalBytes = 0;
    uint64_t zeroCopyTransfers = 0;
    uint64_t fallbackCopies = 0;
    double avgTransferTimeUs = 0.0;
    double savedCopyTimeUs = 0.0;
    float zeroCopyRatio = 0.0f;
};

struct ZeroCopyConfig
{
    uint64_t maxStagingBufferSize = 64 * 1024 * 1024;  // 64 MB
    uint32_t minImageSizeForZC = 256 * 256 * 4;        // Min image bytes for zero-copy
    uint32_t stagingBufferCount = 3;                   // triple buffer
    bool gpuMappingEnabled = true;
    bool fallbackToMemcpy = true;
};

class ZeroCopyActivator
{
  public:
    static ZeroCopyActivator& Instance()
    {
        static ZeroCopyActivator s;
        return s;
    }

    void SetConfig(const ZeroCopyConfig& config)
    {
        m_config = config;
    }
    const ZeroCopyConfig& GetConfig() const
    {
        return m_config;
    }

    bool Activate(uint64_t imageSizeBytes)
    {
        if (m_active)
            return true;
        if (!m_config.gpuMappingEnabled)
            return false;
        if (imageSizeBytes > m_config.maxStagingBufferSize)
            return false;

        m_currentStage = ZCStage::DecodeReady;
        m_active = true;
        m_currentBufferIndex = 0;
        m_activeSizeBytes = imageSizeBytes;
        return true;
    }

    bool Deactivate()
    {
        m_active = false;
        m_currentStage = ZCStage::Inactive;
        m_activeSizeBytes = 0;
        return true;
    }

    bool IsActive() const
    {
        return m_active;
    }
    ZCStage GetCurrentStage() const
    {
        return m_currentStage;
    }

    bool AdvanceStage()
    {
        if (!m_active)
            return false;
        switch (m_currentStage) {
            case ZCStage::DecodeReady:
                m_currentStage = ZCStage::Mapped;
                return true;
            case ZCStage::Mapped:
                m_currentStage = ZCStage::Uploading;
                return true;
            case ZCStage::Uploading:
                m_currentStage = ZCStage::GPUReady;
                RecordTransfer(true);
                return true;
            default:
                return false;
        }
    }

    bool FallbackCopy(uint64_t sizeBytes)
    {
        if (!m_config.fallbackToMemcpy)
            return false;
        m_metrics.fallbackCopies++;
        m_metrics.totalTransfers++;
        m_metrics.totalBytes += sizeBytes;
        UpdateRatio();
        return true;
    }

    ZeroCopyMetrics GetMetrics() const
    {
        return m_metrics;
    }

    uint32_t GetCurrentBufferIndex() const
    {
        return m_currentBufferIndex;
    }

    void RotateBuffer()
    {
        m_currentBufferIndex = (m_currentBufferIndex + 1) % m_config.stagingBufferCount;
    }

    bool ShouldUseZeroCopy(uint64_t imageSizeBytes) const
    {
        if (!m_config.gpuMappingEnabled)
            return false;
        if (imageSizeBytes < m_config.minImageSizeForZC)
            return false;
        if (imageSizeBytes > m_config.maxStagingBufferSize)
            return false;
        return true;
    }

    void ResetMetrics()
    {
        m_metrics = ZeroCopyMetrics{};
    }

    bool Validate() const
    {
        if (m_config.maxStagingBufferSize == 0)
            return false;
        if (m_config.stagingBufferCount == 0 || m_config.stagingBufferCount > 16)
            return false;
        if (m_config.minImageSizeForZC > m_config.maxStagingBufferSize)
            return false;
        if (m_metrics.zeroCopyRatio < 0.0f || m_metrics.zeroCopyRatio > 1.0f)
            return false;
        return true;
    }

  private:
    ZeroCopyActivator() = default;
    ~ZeroCopyActivator() = default;
    ZeroCopyActivator(const ZeroCopyActivator&) = delete;
    ZeroCopyActivator& operator=(const ZeroCopyActivator&) = delete;

    void RecordTransfer(bool zeroCopy)
    {
        m_metrics.totalTransfers++;
        m_metrics.totalBytes += m_activeSizeBytes;
        if (zeroCopy)
            m_metrics.zeroCopyTransfers++;
        UpdateRatio();
        RotateBuffer();
    }

    void UpdateRatio()
    {
        if (m_metrics.totalTransfers > 0) {
            m_metrics.zeroCopyRatio =
                static_cast<float>(m_metrics.zeroCopyTransfers) / static_cast<float>(m_metrics.totalTransfers);
        }
    }

    ZeroCopyConfig m_config{};
    ZeroCopyMetrics m_metrics{};
    ZCStage m_currentStage = ZCStage::Inactive;
    uint32_t m_currentBufferIndex = 0;
    uint64_t m_activeSizeBytes = 0;
    bool m_active = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
