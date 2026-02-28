//==============================================================================
// ExplorerLens Engine — Thumbnail Cache Warming Service
// Background service for proactive thumbnail generation and cache warming.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Warming mode
enum class WarmingMode : uint8_t { Idle = 0, Proactive, OnDemand, COUNT };

/// Cache warming strategy
enum class WarmingStrategy : uint8_t {
 MostRecent, // Warm most recently accessed files
 MostFrequent, // Warm most frequently accessed files
 DirectoryWatch, // Watch directories for new files
 Schedule, // Scheduled warming (idle time)
 Predictive, // ML-based access prediction
 COUNT
};

/// Warming priority
enum class WarmingPriority : uint8_t {
 Idle, // Only during system idle
 Low, // Background, minimal resources
 Normal, // Standard background
 High, // Aggressive warming
 COUNT
};

/// Warming job status
enum class WarmingJobStatus : uint8_t {
 Queued,
 Running,
 Paused,
 Complete,
 Failed,
 Cancelled,
 COUNT
};

/// Cache warming statistics
struct CacheWarmingStats {
 uint64_t filesWarmed = 0;
 uint64_t bytesProcessed = 0;
 uint64_t cacheHitsAdded = 0;
 uint32_t errorsSkipped = 0;
 double totalTimeSeconds = 0;
 double avgTimePerFile = 0;
};

/// Cache warming config
struct CacheWarmingConfig {
 WarmingStrategy strategy = WarmingStrategy::MostRecent;
 WarmingPriority priority = WarmingPriority::Idle;
 uint32_t maxConcurrent = 2;
 uint32_t maxFilesPerSession = 1000;
 uint64_t maxFileSizeBytes = 100 * 1024 * 1024; // 100MB
 bool respectPowerMode = true; // Pause on battery
 bool pauseOnUserActivity = true;
};

/// Thumbnail cache warming service
class CacheWarmingService {
public:
 static const wchar_t *StrategyName(WarmingStrategy s) {
 switch (s) {
 case WarmingStrategy::MostRecent:
 return L"Most Recent";
 case WarmingStrategy::MostFrequent:
 return L"Most Frequent";
 case WarmingStrategy::DirectoryWatch:
 return L"Directory Watch";
 case WarmingStrategy::Schedule:
 return L"Scheduled";
 case WarmingStrategy::Predictive:
 return L"Predictive";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *PriorityName(WarmingPriority p) {
 switch (p) {
 case WarmingPriority::Idle:
 return L"Idle";
 case WarmingPriority::Low:
 return L"Low";
 case WarmingPriority::Normal:
 return L"Normal";
 case WarmingPriority::High:
 return L"High";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *JobStatusName(WarmingJobStatus s) {
 switch (s) {
 case WarmingJobStatus::Queued:
 return L"Queued";
 case WarmingJobStatus::Running:
 return L"Running";
 case WarmingJobStatus::Paused:
 return L"Paused";
 case WarmingJobStatus::Complete:
 return L"Complete";
 case WarmingJobStatus::Failed:
 return L"Failed";
 case WarmingJobStatus::Cancelled:
 return L"Cancelled";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t StrategyCount() {
 return static_cast<size_t>(WarmingStrategy::COUNT);
 }
 static constexpr size_t PriorityCount() {
 return static_cast<size_t>(WarmingPriority::COUNT);
 }
 static constexpr size_t JobStatusCount() {
 return static_cast<size_t>(WarmingJobStatus::COUNT);
 }
 static constexpr size_t ModeCount() {
 return static_cast<size_t>(WarmingMode::COUNT);
 }

 static const wchar_t *ModeName(WarmingMode m) {
 switch (m) {
 case WarmingMode::Idle:
 return L"Idle";
 case WarmingMode::Proactive:
 return L"Proactive";
 case WarmingMode::OnDemand:
 return L"On Demand";
 default:
 return L"Unknown";
 }
 }
};

} // namespace Engine
} // namespace ExplorerLens
