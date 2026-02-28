#pragma once
// MemoryMappedIOOptimizer.h — Memory-Mapped I/O Optimizer
// Direct file mapping for zero-copy decode of large images,
// bypassing read() syscall overhead with demand-paged virtual memory.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Memory mapping strategy
enum class MappingStrategy : uint8_t {
 ReadOnly = 0, // MAP_READONLY: standard thumbnail path
 CopyOnWrite, // MAP_PRIVATE: decode buffer isolation
 LargePages, // 2MB large pages (requires SeLockMemoryPrivilege)
 Prefaulted, // Touch all pages upfront (avoid soft faults during decode)
 Sequential, // MADV_SEQUENTIAL hint for linear scans
 COUNT
};

/// File access pattern hint
enum class MMapAccessPattern : uint8_t {
 Sequential = 0, // Read front-to-back (most images)
 Random, // Random offset access (archives, multi-page)
 HeaderOnly, // First few KB only (format detection)
 Streaming, // Large file, partial reads
 COUNT
};

struct MMapFileInfo {
 uint64_t fileSize = 0;
 uint64_t mappedSize = 0;
 uint64_t viewSize = 0;
 uint64_t alignment = 65536; // 64KB default granularity
 bool isLargePages = false;
 bool isPrefaulted = false;
 MappingStrategy strategy = MappingStrategy::ReadOnly;
};

struct MMapStats {
 uint64_t filesOpened = 0;
 uint64_t totalBytesMapped = 0;
 uint64_t pageFaults = 0; // soft faults
 double avgMapTimeUs = 0.0;
 double savedCopyBytes = 0.0; // bytes saved vs read()
};

class MemoryMappedIOOptimizer {
public:
 static constexpr size_t StrategyCount() {
 return static_cast<size_t>(MappingStrategy::COUNT);
 }
 static constexpr size_t PatternCount() {
 return static_cast<size_t>(MMapAccessPattern::COUNT);
 }

 static const wchar_t *StrategyName(MappingStrategy s) {
 switch (s) {
 case MappingStrategy::ReadOnly:
 return L"Read-Only";
 case MappingStrategy::CopyOnWrite:
 return L"Copy-on-Write";
 case MappingStrategy::LargePages:
 return L"Large Pages (2MB)";
 case MappingStrategy::Prefaulted:
 return L"Prefaulted";
 case MappingStrategy::Sequential:
 return L"Sequential Hint";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *PatternName(MMapAccessPattern p) {
 switch (p) {
 case MMapAccessPattern::Sequential:
 return L"Sequential";
 case MMapAccessPattern::Random:
 return L"Random";
 case MMapAccessPattern::HeaderOnly:
 return L"Header Only";
 case MMapAccessPattern::Streaming:
 return L"Streaming";
 default:
 return L"Unknown";
 }
 }

 /// Calculate aligned mapping offset (Windows requires 64KB granularity)
 static uint64_t AlignOffset(uint64_t offset, uint64_t granularity = 65536) {
 return (offset / granularity) * granularity;
 }

 /// Recommend mapping strategy based on file size
 static MappingStrategy RecommendStrategy(uint64_t fileSize) {
 if (fileSize < 64 * 1024)
 return MappingStrategy::ReadOnly; // small file
 if (fileSize < 16 * 1024 * 1024)
 return MappingStrategy::Sequential;
 return MappingStrategy::LargePages; // large files benefit from 2MB pages
 }
};

} // namespace Engine
} // namespace ExplorerLens
