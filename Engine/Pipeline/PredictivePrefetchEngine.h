#pragma once
// PredictivePrefetchEngine.h — Predictive Prefetch Engine
// Predicts which thumbnails the user will request next based on scroll
// direction, folder navigation patterns, and recently-accessed heuristics.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Prefetch strategy
enum class PrefetchStrategy : uint8_t {
 None = 0, // Disabled
 LinearAhead, // Prefetch N items ahead of scroll
 BidirectionalWindow, // Prefetch ahead and behind
 AccessFrequency, // Prioritize frequently-accessed folders
 MLPredicted, // ML model for navigation prediction
 COUNT
};

/// Scroll direction
enum class ScrollDirection : uint8_t {
 Unknown = 0,
 Down,
 Up,
 PageDown,
 PageUp,
 JumpToEnd,
 COUNT
};

struct PrefetchConfig {
 PrefetchStrategy strategy = PrefetchStrategy::LinearAhead;
 uint32_t windowSize = 32; // items to prefetch
 uint32_t lookAhead = 16; // ahead of viewport
 uint32_t lookBehind = 8; // behind viewport
 uint64_t memoryBudgetBytes = 32 * 1024 * 1024;
 bool cancelOnDirectionChange = true;
};

struct PrefetchStats {
 uint64_t prefetchHits = 0;
 uint64_t prefetchMisses = 0;
 uint64_t bytesTotalPrefetched = 0;
 float hitRate = 0.0f;
 double avgPrefetchMs = 0.0;
};

class PredictivePrefetchEngine {
public:
 static constexpr size_t StrategyCount() {
 return static_cast<size_t>(PrefetchStrategy::COUNT);
 }
 static constexpr size_t DirectionCount() {
 return static_cast<size_t>(ScrollDirection::COUNT);
 }

 static const wchar_t *StrategyName(PrefetchStrategy s) {
 switch (s) {
 case PrefetchStrategy::None:
 return L"Disabled";
 case PrefetchStrategy::LinearAhead:
 return L"Linear Ahead";
 case PrefetchStrategy::BidirectionalWindow:
 return L"Bidirectional Window";
 case PrefetchStrategy::AccessFrequency:
 return L"Access Frequency";
 case PrefetchStrategy::MLPredicted:
 return L"ML Predicted";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *DirectionName(ScrollDirection d) {
 switch (d) {
 case ScrollDirection::Unknown:
 return L"Unknown";
 case ScrollDirection::Down:
 return L"Down";
 case ScrollDirection::Up:
 return L"Up";
 case ScrollDirection::PageDown:
 return L"Page Down";
 case ScrollDirection::PageUp:
 return L"Page Up";
 case ScrollDirection::JumpToEnd:
 return L"Jump to End";
 default:
 return L"Unknown";
 }
 }

 /// Calculate prefetch range based on viewport position and direction
 static void CalcPrefetchRange(uint32_t viewportStart, uint32_t viewportSize,
 uint32_t totalItems, ScrollDirection dir,
 uint32_t lookAhead, uint32_t lookBehind,
 uint32_t &outStart, uint32_t &outEnd) {
 int32_t start = static_cast<int32_t>(viewportStart);
 int32_t end = static_cast<int32_t>(viewportStart + viewportSize);

 if (dir == ScrollDirection::Down || dir == ScrollDirection::PageDown) {
 end += lookAhead;
 start -= lookBehind;
 } else {
 start -= lookAhead;
 end += lookBehind;
 }

 outStart = static_cast<uint32_t>((start < 0) ? 0 : start);
 outEnd =
 static_cast<uint32_t>((end > (int32_t)totalItems) ? totalItems : end);
 }

 /// Calculate hit rate
 static float CalcHitRate(uint64_t hits, uint64_t misses) {
 uint64_t total = hits + misses;
 return total > 0 ? static_cast<float>(hits) / total : 0.0f;
 }
};

} // namespace Engine
} // namespace ExplorerLens
