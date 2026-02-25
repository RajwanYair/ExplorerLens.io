#pragma once
// CrashAnalyticsCollector.h — Crash Analytics Collector
// Structured crash dump collection with minidump generation, stack walking,
// and anonymous telemetry for decoder stability tracking.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Crash dump type
enum class CrashDumpType : uint8_t {
  MiniDump = 0,     // Small (~200KB), stack + register state
  MiniDumpWithHeap, // Medium (~5-50MB), includes heap
  FullDump,         // Large (~hundreds MB), full process
  CustomFiltered,   // Filtered — specific modules only
  COUNT
};

/// Crash category
enum class CrashCategory : uint8_t {
  AccessViolation = 0,
  StackOverflow,
  HeapCorruption,
  DivideByZero,
  UnhandledException,
  DecoderTimeout,
  GPUDeviceLost,
  OutOfMemory,
  COUNT
};

struct CrashReport {
  CrashDumpType type = CrashDumpType::MiniDump;
  CrashCategory category = CrashCategory::UnhandledException;
  uint32_t processId = 0;
  uint32_t threadId = 0;
  uint64_t exceptionCode = 0;
  uint64_t faultAddress = 0;
  const wchar_t *moduleName = nullptr;
  const wchar_t *stackTrace = nullptr;
  uint64_t timestampTicks = 0;
  bool anonymized = true;
};

struct CrashAnalyticsConfig {
  bool enabled = true;
  bool autoSubmit = false;
  bool collectMiniDumps = true;
  bool anonymize = true;
  uint32_t maxDumpsRetained = 10;
  uint64_t maxStorageSizeBytes = 100 * 1024 * 1024;
  CrashDumpType preferredType = CrashDumpType::MiniDump;
};

class CrashAnalyticsCollector {
public:
  static constexpr size_t DumpTypeCount() {
    return static_cast<size_t>(CrashDumpType::COUNT);
  }
  static constexpr size_t CategoryCount() {
    return static_cast<size_t>(CrashCategory::COUNT);
  }

  static const wchar_t *DumpTypeName(CrashDumpType t) {
    switch (t) {
    case CrashDumpType::MiniDump:
      return L"Mini Dump";
    case CrashDumpType::MiniDumpWithHeap:
      return L"Mini Dump + Heap";
    case CrashDumpType::FullDump:
      return L"Full Dump";
    case CrashDumpType::CustomFiltered:
      return L"Custom Filtered";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *CategoryName(CrashCategory c) {
    switch (c) {
    case CrashCategory::AccessViolation:
      return L"Access Violation";
    case CrashCategory::StackOverflow:
      return L"Stack Overflow";
    case CrashCategory::HeapCorruption:
      return L"Heap Corruption";
    case CrashCategory::DivideByZero:
      return L"Divide by Zero";
    case CrashCategory::UnhandledException:
      return L"Unhandled Exception";
    case CrashCategory::DecoderTimeout:
      return L"Decoder Timeout";
    case CrashCategory::GPUDeviceLost:
      return L"GPU Device Lost";
    case CrashCategory::OutOfMemory:
      return L"Out of Memory";
    default:
      return L"Unknown";
    }
  }

  /// Estimate dump file size
  static uint64_t EstimateDumpSize(CrashDumpType type) {
    switch (type) {
    case CrashDumpType::MiniDump:
      return 256 * 1024ULL;
    case CrashDumpType::MiniDumpWithHeap:
      return 10 * 1024 * 1024ULL;
    case CrashDumpType::FullDump:
      return 500 * 1024 * 1024ULL;
    case CrashDumpType::CustomFiltered:
      return 1024 * 1024ULL;
    default:
      return 256 * 1024ULL;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
