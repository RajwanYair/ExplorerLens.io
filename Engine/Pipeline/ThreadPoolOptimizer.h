#pragma once
// Sprint 417: Thread Pool Optimizer
// Adaptive thread pool sizing based on CPU topology, thermal state,
// power profile, and current system load — avoids oversubscription.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Thread pool sizing policy
enum class PoolSizingPolicy : uint8_t {
  Fixed = 0,    // Static thread count
  CoreCount,    // One thread per physical core
  HyperThread,  // One thread per logical processor
  LoadAdaptive, // Scale up/down based on queue depth
  PowerAware,   // Reduce on battery / thermal throttle
  COUNT
};

/// Power profile for thread count decisions
enum class PowerProfile : uint8_t {
  HighPerformance = 0,
  Balanced,
  PowerSaver,
  BatteryLow,
  ThermalThrottled,
  COUNT
};

struct ThreadPoolConfig {
  PoolSizingPolicy policy = PoolSizingPolicy::CoreCount;
  uint32_t minThreads = 1;
  uint32_t maxThreads = 16;
  uint32_t queueDepthLimit = 1024;
  bool useIOCP = true; // Windows I/O completion ports
  bool setAffinity = false;
};

struct TPOptStats {
  uint32_t activeThreads = 0;
  uint32_t idleThreads = 0;
  uint32_t totalThreads = 0;
  uint64_t tasksCompleted = 0;
  uint64_t tasksQueued = 0;
  double avgTaskTimeMs = 0.0;
  float cpuUtilization = 0.0f;
};

class ThreadPoolOptimizer {
public:
  static constexpr size_t PolicyCount() {
    return static_cast<size_t>(PoolSizingPolicy::COUNT);
  }
  static constexpr size_t PowerProfileCount() {
    return static_cast<size_t>(PowerProfile::COUNT);
  }

  static const wchar_t *PolicyName(PoolSizingPolicy p) {
    switch (p) {
    case PoolSizingPolicy::Fixed:
      return L"Fixed";
    case PoolSizingPolicy::CoreCount:
      return L"Per-Core";
    case PoolSizingPolicy::HyperThread:
      return L"Hyper-Thread";
    case PoolSizingPolicy::LoadAdaptive:
      return L"Load Adaptive";
    case PoolSizingPolicy::PowerAware:
      return L"Power Aware";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PowerProfileName(PowerProfile p) {
    switch (p) {
    case PowerProfile::HighPerformance:
      return L"High Performance";
    case PowerProfile::Balanced:
      return L"Balanced";
    case PowerProfile::PowerSaver:
      return L"Power Saver";
    case PowerProfile::BatteryLow:
      return L"Battery Low";
    case PowerProfile::ThermalThrottled:
      return L"Thermal Throttled";
    default:
      return L"Unknown";
    }
  }

  /// Recommend thread count based on policy and logical processor count
  static uint32_t
  RecommendThreads(PoolSizingPolicy policy, uint32_t logicalCores,
                   PowerProfile power = PowerProfile::HighPerformance) {
    uint32_t base = 0;
    switch (policy) {
    case PoolSizingPolicy::Fixed:
      base = 4;
      break;
    case PoolSizingPolicy::CoreCount:
      base = logicalCores / 2;
      break; // physical ~= logical/2
    case PoolSizingPolicy::HyperThread:
      base = logicalCores;
      break;
    case PoolSizingPolicy::LoadAdaptive:
      base = logicalCores / 2;
      break;
    case PoolSizingPolicy::PowerAware:
      base = logicalCores / 2;
      break;
    default:
      base = 4;
    }
    // Apply power profile modifier
    switch (power) {
    case PowerProfile::PowerSaver:
      base = (std::max)(1u, base / 2);
      break;
    case PowerProfile::BatteryLow:
      base = (std::max)(1u, base / 4);
      break;
    case PowerProfile::ThermalThrottled:
      base = (std::max)(1u, base / 3);
      break;
    default:
      break;
    }
    return (std::max)(1u, base);
  }
};

} // namespace Engine
} // namespace ExplorerLens
