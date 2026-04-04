// HardwareAcceleratedPipeline.h — Hardware-Accelerated Pipeline Stage
// Copyright (c) 2026 ExplorerLens Project
//
// Pipeline stage that routes decode and AI inference workloads to optimal
// silicon at runtime (NPU/GPU/CPU) with transparent CPU fallback.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HWPipelineStage : uint8_t {
    Decode = 0,
    Infer,
    Encode,
    Composite
};

struct HWPipelineConfig
{
    bool preferNPUForInfer = true;
    bool preferGPUForDecode = true;
    bool enableCPUFallback = true;
};

struct HWPipelineStats
{
    uint64_t stagesProcessed = 0;
    uint64_t npuRoutings = 0;
    uint64_t gpuRoutings = 0;
    uint64_t cpuFallbacks = 0;
};

class HardwareAcceleratedPipeline
{
  public:
    HardwareAcceleratedPipeline() = default;

    bool Initialize(const HWPipelineConfig& cfg = {})
    {
        m_config = cfg;
        m_ready = true;
        return true;
    }

    bool IsReady() const
    {
        return m_ready;
    }

    std::vector<uint8_t> Process(HWPipelineStage stage, const std::vector<uint8_t>& input)
    {
        ++m_stats.stagesProcessed;
        if (stage == HWPipelineStage::Infer && m_config.preferNPUForInfer) {
            ++m_stats.npuRoutings;
        } else if (stage == HWPipelineStage::Decode && m_config.preferGPUForDecode) {
            ++m_stats.gpuRoutings;
        } else {
            ++m_stats.cpuFallbacks;
        }
        return input;  // pass-through stub
    }

    bool HasCPUFallback() const
    {
        return m_config.enableCPUFallback;
    }

    const HWPipelineStats& GetStats() const
    {
        return m_stats;
    }
    void Reset()
    {
        m_stats = {};
    }

  private:
    bool m_ready = false;
    HWPipelineConfig m_config;
    HWPipelineStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
