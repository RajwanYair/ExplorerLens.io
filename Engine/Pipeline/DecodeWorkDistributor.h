// DecodeWorkDistributor.h — CPU/GPU Decode Work Distribution
// Copyright (c) 2026 ExplorerLens Project
//
// Distributes thumbnail decode workloads across CPU cores and GPU compute
// units based on format requirements and hardware capabilities.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ComputeTarget : uint8_t {
    CPUSingle,
    CPUMulti,
    GPUCompute,
    GPUDecode,
    Hybrid
};

struct WorkUnit
{
    uint64_t unitId = 0;
    std::wstring filePath;
    ComputeTarget target = ComputeTarget::CPUSingle;
    uint32_t estimatedComplexity = 1;
    bool completed = false;
};

struct DistributionPlan
{
    uint32_t cpuWorkUnits = 0;
    uint32_t gpuWorkUnits = 0;
    uint32_t hybridWorkUnits = 0;
    float gpuUtilizationTarget = 0.7f;
    float cpuUtilizationTarget = 0.8f;
};

struct DistributorMetrics
{
    uint64_t totalDistributed = 0;
    uint64_t cpuDispatched = 0;
    uint64_t gpuDispatched = 0;
    double avgDistributionTimeUs = 0.0;
};

class DecodeWorkDistributor
{
  public:
    DecodeWorkDistributor() = default;

    ComputeTarget SelectTarget(const std::wstring& /*filePath*/, uint32_t fileSize) const
    {
        if (!m_gpuAvailable)
            return ComputeTarget::CPUMulti;
        if (fileSize > m_gpuThresholdBytes)
            return ComputeTarget::GPUDecode;
        if (fileSize > m_hybridThresholdBytes)
            return ComputeTarget::Hybrid;
        return ComputeTarget::CPUMulti;
    }

    DistributionPlan PlanDistribution(const std::vector<WorkUnit>& units) const
    {
        DistributionPlan plan;
        for (const auto& unit : units) {
            switch (unit.target) {
                case ComputeTarget::GPUCompute:
                case ComputeTarget::GPUDecode:
                    plan.gpuWorkUnits++;
                    break;
                case ComputeTarget::Hybrid:
                    plan.hybridWorkUnits++;
                    break;
                default:
                    plan.cpuWorkUnits++;
                    break;
            }
        }
        return plan;
    }

    void SetGPUAvailable(bool available)
    {
        m_gpuAvailable = available;
    }
    bool IsGPUAvailable() const
    {
        return m_gpuAvailable;
    }
    void SetGPUThreshold(uint32_t bytes)
    {
        m_gpuThresholdBytes = bytes;
    }
    void SetHybridThreshold(uint32_t bytes)
    {
        m_hybridThresholdBytes = bytes;
    }

    DistributorMetrics GetMetrics() const
    {
        return m_metrics;
    }

  private:
    bool m_gpuAvailable = true;
    uint32_t m_gpuThresholdBytes = 1024 * 1024;
    uint32_t m_hybridThresholdBytes = 256 * 1024;
    DistributorMetrics m_metrics;
};

}  // namespace Engine
}  // namespace ExplorerLens
