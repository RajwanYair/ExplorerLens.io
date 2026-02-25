#pragma once
// Sprint 404: Nested Archive Preview Engine
// Recursive extraction and thumbnail generation for archives-within-archives
// (e.g., .tar.gz, .zip containing .7z), with depth limits and memory budgets.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Archive nesting policy
enum class NestingPolicy : uint8_t {
  NoRecursion = 0, // Only top-level archive
  SingleLevel,     // One level deep
  Limited,         // Up to maxDepth (default 3)
  Unlimited,       // Full recursion (dangerous — capped at 10)
  COUNT
};

/// Extraction status
enum class ExtractionStatus : uint8_t {
  Success = 0,
  DepthLimitReached,
  MemoryBudgetExceeded,
  UnsupportedFormat,
  Corrupted,
  PasswordProtected,
  Timeout,
  COUNT
};

struct NestedArchiveConfig {
  NestingPolicy policy = NestingPolicy::Limited;
  uint32_t maxDepth = 3;
  uint64_t memoryBudgetBytes = 64 * 1024 * 1024; // 64 MB
  uint32_t timeoutMs = 5000;
  bool extractFirstImageOnly = true;
};

struct NestedArchiveStats {
  uint32_t levelsTraversed = 0;
  uint32_t filesEncountered = 0;
  uint32_t imagesFound = 0;
  uint64_t bytesExtracted = 0;
  double extractionTimeMs = 0.0;
  ExtractionStatus status = ExtractionStatus::Success;
};

class NestedArchivePreview {
public:
  static constexpr size_t PolicyCount() {
    return static_cast<size_t>(NestingPolicy::COUNT);
  }
  static constexpr size_t StatusCount() {
    return static_cast<size_t>(ExtractionStatus::COUNT);
  }

  static const wchar_t *PolicyName(NestingPolicy p) {
    switch (p) {
    case NestingPolicy::NoRecursion:
      return L"No Recursion";
    case NestingPolicy::SingleLevel:
      return L"Single Level";
    case NestingPolicy::Limited:
      return L"Limited Depth";
    case NestingPolicy::Unlimited:
      return L"Unlimited";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *StatusName(ExtractionStatus s) {
    switch (s) {
    case ExtractionStatus::Success:
      return L"Success";
    case ExtractionStatus::DepthLimitReached:
      return L"Depth Limit";
    case ExtractionStatus::MemoryBudgetExceeded:
      return L"Memory Exceeded";
    case ExtractionStatus::UnsupportedFormat:
      return L"Unsupported";
    case ExtractionStatus::Corrupted:
      return L"Corrupted";
    case ExtractionStatus::PasswordProtected:
      return L"Password Protected";
    case ExtractionStatus::Timeout:
      return L"Timeout";
    default:
      return L"Unknown";
    }
  }

  static uint32_t EffectiveMaxDepth(NestingPolicy policy,
                                    uint32_t configuredDepth) {
    switch (policy) {
    case NestingPolicy::NoRecursion:
      return 0;
    case NestingPolicy::SingleLevel:
      return 1;
    case NestingPolicy::Limited:
      return configuredDepth;
    case NestingPolicy::Unlimited:
      return 10; // hard cap
    default:
      return 0;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
