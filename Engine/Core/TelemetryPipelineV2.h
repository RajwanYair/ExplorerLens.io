#pragma once
// Sprint 447: Telemetry Pipeline V2
// Privacy-first structured telemetry with differential privacy,
// local aggregation, and configurable data retention policies.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Telemetry data classification
enum class TelemetryLevel : uint8_t {
  Off = 0,         // No telemetry
  DiagnosticsOnly, // Crash/error data only
  BasicUsage,      // Feature usage counts (anonymized)
  Enhanced,        // Performance metrics + usage
  Full,            // All data (opt-in only)
  COUNT
};

/// Privacy mechanism applied
enum class PrivacyMechanism : uint8_t {
  None = 0,            // Raw data (Full mode only)
  LocalAggregation,    // Aggregate before send
  KAnonymity,          // k-anonymity grouping
  DifferentialPrivacy, // Laplace noise injection
  SecureAggregation,   // Encrypted aggregation
  COUNT
};

struct TelemetryV2Event {
  uint64_t eventId = 0;
  const wchar_t *category = nullptr;
  const wchar_t *action = nullptr;
  double value = 0.0;
  TelemetryLevel level = TelemetryLevel::BasicUsage;
  PrivacyMechanism privacy = PrivacyMechanism::LocalAggregation;
  uint64_t timestampMs = 0;
  bool isSampled = false;
};

struct TelemetryPipelineConfig {
  TelemetryLevel level = TelemetryLevel::BasicUsage;
  PrivacyMechanism defaultPrivacy = PrivacyMechanism::LocalAggregation;
  double samplingRate = 0.1; // 10% sampling
  uint32_t batchSize = 100;
  uint32_t flushIntervalSec = 300; // 5 minutes
  uint32_t retentionDays = 30;
  double epsilonDP = 1.0; // Differential privacy epsilon
  bool consentRequired = true;
  bool offlineBuffer = true;
};

struct TelemetryStats {
  uint64_t eventsCollected = 0;
  uint64_t eventsSent = 0;
  uint64_t eventsDropped = 0;
  uint64_t batchesFlushed = 0;
  double avgBatchSizeKB = 0.0;
  size_t bufferUsedBytes = 0;
};

class TelemetryPipelineV2 {
public:
  static constexpr size_t LevelCount() {
    return static_cast<size_t>(TelemetryLevel::COUNT);
  }
  static constexpr size_t PrivacyCount() {
    return static_cast<size_t>(PrivacyMechanism::COUNT);
  }

  static const wchar_t *LevelName(TelemetryLevel l) {
    switch (l) {
    case TelemetryLevel::Off:
      return L"Off";
    case TelemetryLevel::DiagnosticsOnly:
      return L"Diagnostics Only";
    case TelemetryLevel::BasicUsage:
      return L"Basic Usage";
    case TelemetryLevel::Enhanced:
      return L"Enhanced";
    case TelemetryLevel::Full:
      return L"Full";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PrivacyName(PrivacyMechanism p) {
    switch (p) {
    case PrivacyMechanism::None:
      return L"None";
    case PrivacyMechanism::LocalAggregation:
      return L"Local Aggregation";
    case PrivacyMechanism::KAnonymity:
      return L"k-Anonymity";
    case PrivacyMechanism::DifferentialPrivacy:
      return L"Differential Privacy";
    case PrivacyMechanism::SecureAggregation:
      return L"Secure Aggregation";
    default:
      return L"Unknown";
    }
  }

  /// Check if level requires explicit consent
  static bool RequiresConsent(TelemetryLevel level) {
    return level >= TelemetryLevel::Enhanced;
  }

  /// Minimum privacy for given level
  static PrivacyMechanism MinPrivacy(TelemetryLevel level) {
    switch (level) {
    case TelemetryLevel::Off:
      return PrivacyMechanism::None;
    case TelemetryLevel::DiagnosticsOnly:
      return PrivacyMechanism::LocalAggregation;
    case TelemetryLevel::BasicUsage:
      return PrivacyMechanism::KAnonymity;
    case TelemetryLevel::Enhanced:
      return PrivacyMechanism::DifferentialPrivacy;
    case TelemetryLevel::Full:
      return PrivacyMechanism::LocalAggregation;
    default:
      return PrivacyMechanism::DifferentialPrivacy;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
