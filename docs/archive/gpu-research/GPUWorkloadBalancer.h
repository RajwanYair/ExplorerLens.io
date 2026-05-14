#pragma once
// ============================================================================
// GPUWorkloadBalancer.h — Multi-GPU workload distribution and affinity
//
// Purpose:   Multi-GPU workload distribution and affinity management
// Provides:  BalancingStrategy, GPUWorkloadType enums, GPUWorkItem,
//            GPUBalancerDeviceInfo structs, and GPUWorkloadBalancer class
// Used by:   GPU decode pipeline
// ============================================================================

#include <algorithm>
#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// GPUWorkloadBalancer — Multi-GPU workload distribution
// ============================================================================

enum class BalancingStrategy {
    RoundRobin,
    LoadBased,
    CapabilityBased,
    Affinity,
    Manual
};

inline const char* BalancingStrategyName(BalancingStrategy value)
{
    switch (value) {
        case BalancingStrategy::RoundRobin:
            return "RoundRobin";
        case BalancingStrategy::LoadBased:
            return "LoadBased";
        case BalancingStrategy::CapabilityBased:
            return "CapabilityBased";
        case BalancingStrategy::Affinity:
            return "Affinity";
        case BalancingStrategy::Manual:
            return "Manual";
        default:
            return "Unknown";
    }
}

enum class GPUWorkloadType {
    Decode,
    Resize,
    ToneMap,
    Compose,
    Compute
};

inline const char* GPUWorkloadTypeName(GPUWorkloadType value)
{
    switch (value) {
        case GPUWorkloadType::Decode:
            return "Decode";
        case GPUWorkloadType::Resize:
            return "Resize";
        case GPUWorkloadType::ToneMap:
            return "ToneMap";
        case GPUWorkloadType::Compose:
            return "Compose";
        case GPUWorkloadType::Compute:
            return "Compute";
        default:
            return "Unknown";
    }
}

struct GPUWorkItem
{
    GPUWorkloadType workloadType = GPUWorkloadType::Decode;
    float estimatedMs = 0.0f;
    uint32_t preferredGPU = 0;
    uint32_t priority = 0;  // 0 = highest
    uint64_t dataSizeBytes = 0;
    bool requiresDedicatedGPU = false;

    bool IsHighPriority() const
    {
        return priority == 0;
    }
};

struct GPUBalancerDeviceInfo
{
    uint32_t deviceIndex = 0;
    std::string deviceName;
    uint64_t vramBytes = 0;
    float currentLoadPercent = 0.0f;
    uint32_t pendingWorkItems = 0;
    bool isDiscrete = false;
    bool isAvailable = true;

    float GetAvailableCapacity() const
    {
        return 100.0f - currentLoadPercent;
    }
};

class GPUWorkloadBalancer
{
  public:
    static constexpr uint32_t MAX_GPUS = 8;
    static constexpr float OVERLOAD_THRESHOLD = 90.0f;
    static constexpr uint32_t MAX_QUEUE_DEPTH = 1024;

    GPUWorkloadBalancer()
        : m_strategy(BalancingStrategy::LoadBased)
        , m_activeGPUCount(0)
        , m_totalSubmitted(0)
        , m_roundRobinIndex(0)
    {
        m_devices.fill(GPUBalancerDeviceInfo{});
    }

    ~GPUWorkloadBalancer() = default;

    uint32_t SubmitWork(const GPUWorkItem& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        uint32_t targetGPU = GetOptimalGPUInternal(item);
        if (targetGPU < m_activeGPUCount) {
            m_devices[targetGPU].pendingWorkItems++;
            m_devices[targetGPU].currentLoadPercent += item.estimatedMs * 0.1f;
            if (m_devices[targetGPU].currentLoadPercent > 100.0f) {
                m_devices[targetGPU].currentLoadPercent = 100.0f;
            }
        }

        m_totalSubmitted++;
        return targetGPU;
    }

    uint32_t GetOptimalGPU(const GPUWorkItem& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return GetOptimalGPUInternal(item);
    }

    float GetUtilization(uint32_t gpuIndex) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (gpuIndex >= m_activeGPUCount)
            return 0.0f;
        return m_devices[gpuIndex].currentLoadPercent;
    }

    void RegisterGPU(uint32_t index, const std::string& name, uint64_t vramBytes, bool discrete)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (index >= MAX_GPUS)
            return;

        m_devices[index].deviceIndex = index;
        m_devices[index].deviceName = name;
        m_devices[index].vramBytes = vramBytes;
        m_devices[index].isDiscrete = discrete;
        m_devices[index].isAvailable = true;

        if (index >= m_activeGPUCount) {
            m_activeGPUCount = index + 1;
        }
    }

    void SetStrategy(BalancingStrategy strategy)
    {
        m_strategy = strategy;
    }
    BalancingStrategy GetStrategy() const
    {
        return m_strategy;
    }

    uint32_t GetActiveGPUCount() const
    {
        return m_activeGPUCount;
    }
    uint64_t GetTotalSubmitted() const
    {
        return m_totalSubmitted;
    }

    void CompleteWork(uint32_t gpuIndex, float actualMs)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (gpuIndex >= m_activeGPUCount)
            return;
        if (m_devices[gpuIndex].pendingWorkItems > 0) {
            m_devices[gpuIndex].pendingWorkItems--;
        }
        m_devices[gpuIndex].currentLoadPercent -= actualMs * 0.1f;
        if (m_devices[gpuIndex].currentLoadPercent < 0.0f) {
            m_devices[gpuIndex].currentLoadPercent = 0.0f;
        }
    }

  private:
    uint32_t GetOptimalGPUInternal(const GPUWorkItem& item) const
    {
        if (m_activeGPUCount == 0)
            return 0;

        if (m_strategy == BalancingStrategy::Manual && item.preferredGPU < m_activeGPUCount) {
            return item.preferredGPU;
        }

        if (m_strategy == BalancingStrategy::RoundRobin) {
            uint32_t idx = m_roundRobinIndex % m_activeGPUCount;
            const_cast<uint32_t&>(m_roundRobinIndex)++;
            return idx;
        }

        // LoadBased / CapabilityBased / Affinity: pick least loaded available GPU
        uint32_t bestGPU = 0;
        float bestScore = -1.0f;

        for (uint32_t i = 0; i < m_activeGPUCount; i++) {
            if (!m_devices[i].isAvailable)
                continue;
            if (item.requiresDedicatedGPU && !m_devices[i].isDiscrete)
                continue;

            float capacity = m_devices[i].GetAvailableCapacity();

            if (m_strategy == BalancingStrategy::CapabilityBased) {
                // Prefer discrete GPUs with more VRAM
                float vramBonus = static_cast<float>(m_devices[i].vramBytes >> 30);  // GB
                capacity += vramBonus * 10.0f;
                if (m_devices[i].isDiscrete)
                    capacity += 20.0f;
            }

            if (m_strategy == BalancingStrategy::Affinity && item.preferredGPU == i) {
                capacity += 50.0f;  // Strong affinity bonus
            }

            if (capacity > bestScore) {
                bestScore = capacity;
                bestGPU = i;
            }
        }

        return bestGPU;
    }

    mutable std::mutex m_mutex;
    BalancingStrategy m_strategy;
    std::array<GPUBalancerDeviceInfo, MAX_GPUS> m_devices;
    uint32_t m_activeGPUCount;
    uint64_t m_totalSubmitted;
    uint32_t m_roundRobinIndex;
};

}  // namespace Engine
}  // namespace ExplorerLens
