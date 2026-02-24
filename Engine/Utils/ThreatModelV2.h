//==============================================================================
// ExplorerLens Engine — Threat Model V2
// STRIDE-based threat modeling with attack surface enumeration, mitigations
// tracker, and automated security policy generation for the thumbnail pipeline.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ThreatCategory : uint8_t {
  Spoofing = 0,
  Tampering,
  Repudiation,
  InfoDisclosure,
  DoS,
  EoP,
  COUNT
};
enum class ThreatSeverity : uint8_t { Low = 0, Medium, High, Critical, COUNT };
enum class MitigationStatus : uint8_t {
  Open = 0,
  InProgress,
  Mitigated,
  Accepted,
  WontFix,
  Implemented = Mitigated, // compat alias
  COUNT = WontFix + 1
};

struct ThreatEntry {
  std::wstring id;
  ThreatCategory category = ThreatCategory::Tampering;
  ThreatSeverity severity = ThreatSeverity::Medium;
  MitigationStatus status = MitigationStatus::Open;
  std::wstring description;
  std::wstring mitigation;
};

struct ThreatModelSummary {
  uint32_t criticalCount = 0;
  uint32_t highCount = 0;
  uint32_t openCount = 0;
  uint32_t mitigatedCount = 0;
  float riskScore = 0.0f; // 0-10
};

class ThreatModelV2 {
public:
  static const wchar_t *CategoryName(ThreatCategory c) {
    switch (c) {
    case ThreatCategory::Spoofing:
      return L"Spoofing";
    case ThreatCategory::Tampering:
      return L"Tampering";
    case ThreatCategory::Repudiation:
      return L"Repudiation";
    case ThreatCategory::InfoDisclosure:
      return L"Info Disclosure";
    case ThreatCategory::DoS:
      return L"Denial of Service";
    case ThreatCategory::EoP:
      return L"Elevation of Privilege";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *SeverityName(ThreatSeverity s) {
    switch (s) {
    case ThreatSeverity::Low:
      return L"Low";
    case ThreatSeverity::Medium:
      return L"Medium";
    case ThreatSeverity::High:
      return L"High";
    case ThreatSeverity::Critical:
      return L"Critical";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *MitigationStatusName(MitigationStatus m) {
    switch (m) {
    case MitigationStatus::Open:
      return L"Open";
    case MitigationStatus::InProgress:
      return L"In Progress";
    case MitigationStatus::Mitigated:
      return L"Mitigated";
    case MitigationStatus::Accepted:
      return L"Accepted";
    case MitigationStatus::WontFix:
      return L"Won't Fix";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t CategoryCount() {
    return static_cast<size_t>(ThreatCategory::COUNT);
  }
  static constexpr size_t SeverityCount() {
    return static_cast<size_t>(ThreatSeverity::COUNT);
  }
  static constexpr size_t MitigStatusCount() {
    return static_cast<size_t>(MitigationStatus::COUNT);
  }

  // Compatibility aliases (tests)
  static const wchar_t *MitigationName(MitigationStatus m) {
    return MitigationStatusName(m);
  }
  static bool IsRiskAcceptable(const ThreatModelSummary &s) {
    return s.criticalCount == 0 && s.highCount == 0 && s.riskScore < 5.0f;
  }
};

} // namespace Engine
} // namespace ExplorerLens
