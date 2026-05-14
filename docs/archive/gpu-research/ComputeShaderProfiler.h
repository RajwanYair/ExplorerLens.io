// ComputeShaderProfiler.h — GPU Compute Dispatch Profiling
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks GPU compute shader dispatch timings, resource utilization, and
// occupancy to guide adaptive shader selection and load balancing.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DispatchRecord
{
    std::string shaderName;
    uint32_t groupCountX = 0;
    uint32_t groupCountY = 0;
    uint32_t groupCountZ = 0;
    double gpuTimeMs = 0.0;
    double cpuTimeMs = 0.0;
    uint64_t inputBytes = 0;
    uint64_t outputBytes = 0;
};

struct ShaderProfileSummary
{
    std::string shaderName;
    uint64_t dispatchCount = 0;
    double totalGpuMs = 0.0;
    double avgGpuMs = 0.0;
    double minGpuMs = 1e9;
    double maxGpuMs = 0.0;
    uint64_t totalInputBytes = 0;
    uint64_t totalOutputBytes = 0;
    double throughputMBps = 0.0;
};

struct ProfilerConfig
{
    bool enabled = true;
    uint32_t maxRecords = 1000;
    bool captureTimestamps = true;
    bool capturePipelineStats = false;
};

class ComputeShaderProfiler
{
  public:
    void Configure(const ProfilerConfig& config)
    {
        m_config = config;
    }

    void RecordDispatch(const DispatchRecord& record)
    {
        if (!m_config.enabled)
            return;
        auto& summary = m_summaries[record.shaderName];
        summary.shaderName = record.shaderName;
        summary.dispatchCount++;
        summary.totalGpuMs += record.gpuTimeMs;
        summary.avgGpuMs = summary.totalGpuMs / summary.dispatchCount;
        if (record.gpuTimeMs < summary.minGpuMs)
            summary.minGpuMs = record.gpuTimeMs;
        if (record.gpuTimeMs > summary.maxGpuMs)
            summary.maxGpuMs = record.gpuTimeMs;
        summary.totalInputBytes += record.inputBytes;
        summary.totalOutputBytes += record.outputBytes;
        if (summary.totalGpuMs > 0) {
            summary.throughputMBps = (summary.totalInputBytes + summary.totalOutputBytes) / (1024.0 * 1024.0)
                                     / (summary.totalGpuMs / 1000.0);
        }
    }

    ShaderProfileSummary GetSummary(const std::string& shaderName) const
    {
        auto it = m_summaries.find(shaderName);
        return it != m_summaries.end() ? it->second : ShaderProfileSummary{};
    }

    std::vector<ShaderProfileSummary> GetAllSummaries() const
    {
        std::vector<ShaderProfileSummary> result;
        result.reserve(m_summaries.size());
        for (const auto& [k, v] : m_summaries)
            result.push_back(v);
        return result;
    }

    void Reset()
    {
        m_summaries.clear();
    }

  private:
    ProfilerConfig m_config;
    std::unordered_map<std::string, ShaderProfileSummary> m_summaries;
};

}  // namespace Engine
}  // namespace ExplorerLens
