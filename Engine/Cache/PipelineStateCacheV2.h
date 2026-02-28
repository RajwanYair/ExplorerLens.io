//==============================================================================
// ExplorerLens Engine — Pipeline State Cache V2
// Persistent PSO library with binary cache, async precompilation, and
// versioned signature validation for near-zero first-draw latency.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// PSO cache entry state
enum class PSOCacheState : uint8_t {
 NotCached = 0,
 Compiling,
 Cached,
 Invalidated,
 COUNT
};

/// Pipeline type
enum class PipelineType : uint8_t {
 Graphics = 0,
 Compute,
 MeshShader,
 WorkGraph,
 COUNT
};

/// PSO warmup strategy
enum class PSOWarmupStrategy : uint8_t {
 Lazy = 0, // compile on first use
 Eager, // compile at startup
 Background, // async background compile
 COUNT
};

/// PSO cache strategy
enum class PSOCacheStrategy : uint8_t {
 InMemory = 0,
 PersistentDisk,
 Hybrid,
 COUNT
};

/// PSO cache entry
struct PSOCacheEntry {
 std::wstring name;
 PipelineType type = PipelineType::Compute;
 PSOCacheState state = PSOCacheState::NotCached;
 uint32_t version = 1;
 uint64_t sizeBytes = 0;
 bool valid = false;
};

/// Cache V2 stats
struct PSOCacheStats {
 uint32_t totalEntries = 0;
 uint32_t cachedEntries = 0;
 uint32_t hitCount = 0;
 uint32_t missCount = 0;
 uint64_t cacheSizeBytes = 0;
};

/// Pipeline State Cache V2
class PipelineStateCacheV2 {
public:
 static const wchar_t *CacheStateName(PSOCacheState s) {
 switch (s) {
 case PSOCacheState::NotCached:
 return L"Not Cached";
 case PSOCacheState::Compiling:
 return L"Compiling";
 case PSOCacheState::Cached:
 return L"Cached";
 case PSOCacheState::Invalidated:
 return L"Invalidated";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *PipelineTypeName(PipelineType t) {
 switch (t) {
 case PipelineType::Graphics:
 return L"Graphics";
 case PipelineType::Compute:
 return L"Compute";
 case PipelineType::MeshShader:
 return L"Mesh Shader";
 case PipelineType::WorkGraph:
 return L"Work Graph";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *WarmupStrategyName(PSOWarmupStrategy w) {
 switch (w) {
 case PSOWarmupStrategy::Lazy:
 return L"Lazy";
 case PSOWarmupStrategy::Eager:
 return L"Eager";
 case PSOWarmupStrategy::Background:
 return L"Background";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t CacheStateCount() {
 return static_cast<size_t>(PSOCacheState::COUNT);
 }
 static constexpr size_t PipelineTypeCount() {
 return static_cast<size_t>(PipelineType::COUNT);
 }
 static constexpr size_t WarmupStrategyCount() {
 return static_cast<size_t>(PSOWarmupStrategy::COUNT);
 }
 static constexpr size_t StrategyCount() {
 return static_cast<size_t>(PSOCacheStrategy::COUNT);
 }

 static const wchar_t *StrategyName(PSOCacheStrategy s) {
 switch (s) {
 case PSOCacheStrategy::InMemory:
 return L"In-Memory";
 case PSOCacheStrategy::PersistentDisk:
 return L"Persistent Disk";
 case PSOCacheStrategy::Hybrid:
 return L"Hybrid";
 default:
 return L"Unknown";
 }
 }

 static PSOCacheStats DefaultStats() { return PSOCacheStats{}; }

 static bool IsHitRateGood(const PSOCacheStats &s) {
 if (s.hitCount + s.missCount == 0)
 return true;
 double rate = static_cast<double>(s.hitCount) / (s.hitCount + s.missCount);
 return rate >= 0.80;
 }
};

} // namespace Engine
} // namespace ExplorerLens
