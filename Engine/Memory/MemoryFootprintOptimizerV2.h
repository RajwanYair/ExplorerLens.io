//==============================================================================
// ExplorerLens Engine — Memory Footprint Optimizer V2
// Working-set trim automation, custom allocator selection per workload,
// large-page mapping for GPU staging buffers, paged-memory compaction,
// and heap defragmentation heuristics.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class AllocatorType : uint8_t { SystemHeap=0, MiMalloc, JEMalloc, SegAlloc, PoolSlab, COUNT };
enum class TrimStrategy : uint8_t { Immediate=0, Deferred, Idle, Pressure, COUNT };
enum class LargePagePolicy : uint8_t { Disabled=0, Preferred, Required, COUNT };

struct MemFootprintConfig {
 AllocatorType allocator = AllocatorType::MiMalloc;
 TrimStrategy trim = TrimStrategy::Idle;
 LargePagePolicy largePages = LargePagePolicy::Preferred;
 uint32_t targetWorkingSetMB = 128;
 bool heapDefrag = true;
 bool compactOnIdle= true;
};

struct MemFootprintReport {
 uint64_t workingSetBytes = 0;
 uint64_t peakWorkingSetBytes= 0;
 uint64_t privateBytes = 0;
 uint64_t largePageBytes = 0;
 uint32_t trimActionsCount = 0;
 float fragmentation = 0.0f; // 0.0 = no frag, 1.0 = fully frag
};

class MemoryFootprintOptimizerV2 {
public:
 static const wchar_t* AllocatorName(AllocatorType a) {
 switch(a) {
 case AllocatorType::SystemHeap: return L"System Heap (HeapAlloc)";
 case AllocatorType::MiMalloc: return L"mimalloc";
 case AllocatorType::JEMalloc: return L"jemalloc";
 case AllocatorType::SegAlloc: return L"Segment Allocator";
 case AllocatorType::PoolSlab: return L"Pool Slab";
 default: return L"Unknown";
 }
 }
 static const wchar_t* TrimStrategyName(TrimStrategy t) {
 switch(t) {
 case TrimStrategy::Immediate: return L"Immediate";
 case TrimStrategy::Deferred: return L"Deferred";
 case TrimStrategy::Idle: return L"On Idle";
 case TrimStrategy::Pressure: return L"On Pressure";
 default: return L"Unknown";
 }
 }
 static const wchar_t* LargePagePolicyName(LargePagePolicy p) {
 switch(p) {
 case LargePagePolicy::Disabled: return L"Disabled";
 case LargePagePolicy::Preferred: return L"Preferred";
 case LargePagePolicy::Required: return L"Required";
 default: return L"Unknown";
 }
 }
 static constexpr size_t AllocatorCount() { return static_cast<size_t>(AllocatorType::COUNT); }
 static constexpr size_t TrimStrategyCount() { return static_cast<size_t>(TrimStrategy::COUNT); }
 static constexpr size_t LargePagePolicyCount(){ return static_cast<size_t>(LargePagePolicy::COUNT); }
 static bool WithinTarget(const MemFootprintReport& r, uint32_t targetMB) {
 return r.workingSetBytes <= static_cast<uint64_t>(targetMB)*1024*1024;
 }
};

}} // namespace ExplorerLens::Engine

