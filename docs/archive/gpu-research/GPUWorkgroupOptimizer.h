// GPUWorkgroupOptimizer.h — Dynamic Compute Shader Workgroup Sizing
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically sizes compute shader workgroups based on hardware limits,
// texture dimensions, and measured occupancy for optimal GPU throughput.
// Supports DirectX 12 and Vulkan dispatch optimization.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// GPU vendor for workgroup optimization heuristics
enum class WGVendor : uint8_t {
    Unknown = 0,
    Intel,
    NVIDIA,
    AMD
};

/// Workgroup size recommendation
struct WorkgroupSize
{
    uint32_t x = 8;  // Threads in X dimension
    uint32_t y = 8;  // Threads in Y dimension
    uint32_t z = 1;  // Threads in Z dimension

    uint32_t TotalThreads() const
    {
        return x * y * z;
    }
};

/// Dispatch dimensions for compute shader launch
struct DispatchDimensions
{
    uint32_t groupsX = 1;
    uint32_t groupsY = 1;
    uint32_t groupsZ = 1;
    WorkgroupSize localSize;
    uint32_t totalThreads = 0;
    float occupancy = 0.0f;  // Estimated occupancy (0.0 - 1.0)
};

/// Hardware limits queried from the GPU
struct GPUComputeLimits
{
    uint32_t maxWorkgroupSize = 1024;
    uint32_t maxWorkgroupDimX = 1024;
    uint32_t maxWorkgroupDimY = 1024;
    uint32_t maxWorkgroupDimZ = 64;
    uint32_t maxDispatchX = 65535;
    uint32_t maxDispatchY = 65535;
    uint32_t maxDispatchZ = 65535;
    uint32_t warpSize = 32;  // 32 for NVIDIA/Intel, 64 for AMD
    uint32_t maxSharedMemoryBytes = 32768;
    uint32_t computeUnits = 16;
    WGVendor vendor = WGVendor::Unknown;
};

/// Workgroup optimization statistics
struct WorkgroupStats
{
    uint64_t optimizationsRun = 0;
    uint64_t totalDispatches = 0;
    double avgOccupancy = 0.0;
    double avgThreadsPerGroup = 0.0;
};

/// Dynamic workgroup optimizer for compute shader dispatches
class GPUWorkgroupOptimizer
{
  public:
    explicit GPUWorkgroupOptimizer(const GPUComputeLimits& limits = {}) : m_limits(limits)
    {
        DetectVendorHeuristics();
    }

    /// Calculate optimal workgroup size for a 2D image operation
    DispatchDimensions Optimize2D(uint32_t imageWidth, uint32_t imageHeight, uint32_t sharedMemPerThread = 0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.optimizationsRun++;

        WorkgroupSize local = ChooseLocalSize2D(imageWidth, imageHeight, sharedMemPerThread);
        DispatchDimensions result;
        result.localSize = local;
        result.groupsX = (imageWidth + local.x - 1) / local.x;
        result.groupsY = (imageHeight + local.y - 1) / local.y;
        result.groupsZ = 1;
        result.totalThreads = result.groupsX * result.groupsY * local.TotalThreads();

        // Clamp to dispatch limits
        result.groupsX = (std::min)(result.groupsX, m_limits.maxDispatchX);
        result.groupsY = (std::min)(result.groupsY, m_limits.maxDispatchY);

        // Estimate occupancy
        result.occupancy = EstimateOccupancy(local, sharedMemPerThread);

        m_stats.totalDispatches++;
        m_stats.avgOccupancy = m_stats.avgOccupancy * 0.95 + result.occupancy * 0.05;
        m_stats.avgThreadsPerGroup = m_stats.avgThreadsPerGroup * 0.95 + local.TotalThreads() * 0.05;

        return result;
    }

    /// Calculate optimal workgroup for 1D data processing
    DispatchDimensions Optimize1D(uint32_t elementCount)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.optimizationsRun++;

        uint32_t groupSize = m_limits.warpSize * 4;  // 4 warps per group baseline
        groupSize = (std::min)(groupSize, m_limits.maxWorkgroupSize);

        DispatchDimensions result;
        result.localSize = {groupSize, 1, 1};
        result.groupsX = (elementCount + groupSize - 1) / groupSize;
        result.groupsY = 1;
        result.groupsZ = 1;
        result.groupsX = (std::min)(result.groupsX, m_limits.maxDispatchX);
        result.totalThreads = result.groupsX * groupSize;
        result.occupancy = EstimateOccupancy(result.localSize, 0);

        m_stats.totalDispatches++;
        return result;
    }

    void SetLimits(const GPUComputeLimits& limits)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_limits = limits;
        DetectVendorHeuristics();
    }

    GPUComputeLimits GetLimits() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_limits;
    }

    WorkgroupStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

  private:
    void DetectVendorHeuristics()
    {
        if (m_limits.vendor == WGVendor::AMD) {
            m_limits.warpSize = 64;  // AMD wavefront = 64
        } else {
            m_limits.warpSize = 32;  // NVIDIA warp = 32, Intel EU = 32 SIMD8×4
        }
    }

    WorkgroupSize ChooseLocalSize2D(uint32_t w, uint32_t h, uint32_t sharedPerThread) const
    {
        // Start with common 8×8 = 64 threads
        uint32_t targetThreads = m_limits.warpSize * 2;  // 2 waves per group

        // If image is very wide, prefer wider groups
        if (w >= 4 * h)
            targetThreads = m_limits.warpSize * 4;

        // Shared memory constraint
        if (sharedPerThread > 0) {
            uint32_t maxThreads = m_limits.maxSharedMemoryBytes / sharedPerThread;
            targetThreads = (std::min)(targetThreads, maxThreads);
        }

        targetThreads = (std::min)(targetThreads, m_limits.maxWorkgroupSize);
        targetThreads = (std::max)(targetThreads, m_limits.warpSize);

        // Find best square-ish factorization
        uint32_t bestX = targetThreads, bestY = 1;
        float bestRatio = 1e10f;
        for (uint32_t x = 1; x <= targetThreads; x *= 2) {
            uint32_t y = targetThreads / x;
            if (x * y != targetThreads)
                continue;
            if (x > m_limits.maxWorkgroupDimX || y > m_limits.maxWorkgroupDimY)
                continue;
            // Prefer square-ish workgroups for 2D cache locality
            float ratio = static_cast<float>((std::max)(x, y)) / static_cast<float>((std::max)(1u, (std::min)(x, y)));
            if (ratio < bestRatio) {
                bestRatio = ratio;
                bestX = x;
                bestY = y;
            }
        }

        return {bestX, bestY, 1};
    }

    float EstimateOccupancy(const WorkgroupSize& local, uint32_t sharedPerThread) const
    {
        uint32_t threadsPerGroup = local.TotalThreads();
        uint32_t wavesPerGroup = (threadsPerGroup + m_limits.warpSize - 1) / m_limits.warpSize;

        // Approximate: max waves per CU is ~32 for NVIDIA, ~40 for AMD
        uint32_t maxWavesPerCU = (m_limits.vendor == WGVendor::AMD) ? 40u : 32u;

        // Shared memory constraint
        if (sharedPerThread > 0) {
            uint32_t sharedUsed = sharedPerThread * threadsPerGroup;
            uint32_t groupsPerCU = m_limits.maxSharedMemoryBytes / (std::max)(1u, sharedUsed);
            uint32_t wavesFromShared = groupsPerCU * wavesPerGroup;
            maxWavesPerCU = (std::min)(maxWavesPerCU, wavesFromShared);
        }

        float occupancy = static_cast<float>(wavesPerGroup) / static_cast<float>(maxWavesPerCU);
        return (std::min)(1.0f, occupancy);
    }

    GPUComputeLimits m_limits;
    mutable std::mutex m_mutex;
    WorkgroupStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
