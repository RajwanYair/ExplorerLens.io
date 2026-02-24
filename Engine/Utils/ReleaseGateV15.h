#pragma once
// =============================================================================
// ReleaseGateV15.h — v10.5 Release Quality Gate (Final)
// ExplorerLens.io Engine — Utils Module
// =============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {

/// v10.5 KPI dimensions (20 total — milestone gate)
enum class GateKPIV15 : uint32_t {
  BuildClean = 0,
  TestPass = 1,
  CodeCoverage = 2,
  PerformanceTarget = 3,
  MemoryBudget = 4,
  BinarySize = 5,
  DocumentationSync = 6,
  APIStability = 7,
  SecurityScan = 8,
  PluginCompat = 9,
  ARM64Validation = 10,
  CacheEfficiency = 11,
  FormatCoverage = 12,
  HashVerification = 13,
  RegistryIntegrity = 14,
  RecoverySuccess = 15,
  ResourcePoolHealth = 16,
  MetadataAccuracy = 17,
  ContentIndexHealth = 18, ///< NEW — indexer coverage & accuracy
  ConfigMigration = 19,    ///< NEW — migration success rate
  Count = 20
};

/// KPI evaluation result
struct KPIV15Result {
  GateKPIV15 kpi = GateKPIV15::BuildClean;
  bool passed = false;
  double value = 0.0;
  double threshold = 0.0;
  std::wstring details;
};

/// ReleaseGateV15 — v10.5 milestone release gate with 20 KPIs
class ReleaseGateV15 {
public:
  ReleaseGateV15();

  // Evaluation
  void SetKPIResult(GateKPIV15 kpi, bool passed, double value = 0.0);
  KPIV15Result GetKPIResult(GateKPIV15 kpi) const;
  bool Evaluate() const;
  bool IsApproved() const;

  // Reporting
  uint32_t GetPassedCount() const;
  uint32_t GetFailedCount() const;
  std::vector<KPIV15Result> GetAllResults() const;
  std::wstring GetVersion() const { return L"10.5.0"; }

  // Static
  static const wchar_t *GetKPIName(GateKPIV15 kpi);
  static constexpr uint32_t GetKPICount() {
    return static_cast<uint32_t>(GateKPIV15::Count);
  }

private:
  KPIV15Result m_results[static_cast<uint32_t>(GateKPIV15::Count)];
  void InitializeDefaults();
};

} // namespace ExplorerLens
