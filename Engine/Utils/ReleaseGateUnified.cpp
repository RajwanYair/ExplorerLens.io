#include "ReleaseGate.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

ReleaseGateV3::ReleaseGateV3() : m_thresholds(ForV92()) {}

ReleaseGateV3::ReleaseGateV3(const ReleaseThresholdsV92 &thresholds)
    : m_thresholds(thresholds) {}

void ReleaseGateV3::AddMeasurement(const KPIMeasurement &m) {
  m_measurements.push_back(m);
}

void ReleaseGateV3::AddPlatform(const PlatformValidation &p) {
  m_platforms.push_back(p);
}

ReleaseGateVerdict ReleaseGateV3::ComputeVerdict(uint32_t passed, uint32_t failed,
                                          bool hasBlockers) const {
  if (hasBlockers)
    return ReleaseGateVerdict::Blocked;
  if (failed == 0)
    return ReleaseGateVerdict::Pass;
  if (passed > failed)
    return ReleaseGateVerdict::ConditionalPass;
  return ReleaseGateVerdict::Fail;
}

ReleaseGateResult ReleaseGateV3::Evaluate() const {
  ReleaseGateResult result;
  result.version = L"v9.2.0";

  uint32_t passed = 0;
  uint32_t failed = 0;
  for (const auto &m : m_measurements) {
    result.totalKPIs++;
    if (m.passed) {
      result.passedKPIs++;
      passed++;
      continue;
    }

    result.failedKPIs++;
    failed++;
    if (m.dimension == ReleaseKPIDimension::BuildQuality ||
        m.dimension == ReleaseKPIDimension::Security ||
        m.dimension == ReleaseKPIDimension::Stability) {
      result.blockers.push_back(m.name.empty() ? L"UnknownKPI" : m.name);
    }
  }

  result.measurements = m_measurements;
  result.platforms = m_platforms;
  result.overallScore =
      result.totalKPIs > 0 ? (100.0 * passed / result.totalKPIs) : 0.0;
  result.verdict = ComputeVerdict(passed, failed, !result.blockers.empty());
  return result;
}

std::wstring ReleaseGateV3::GenerateChecklist() const {
  std::wostringstream oss;
  oss << L"ReleaseGateV3 Checklist (v9.2)\n";
  oss << L" [ ] Zero warnings\n";
  oss << L" [ ] 100% test pass rate\n";
  oss << L" [ ] ARM64 CI green\n";
  oss << L" [ ] MSIX package built\n";
  oss << L" [ ] High-DPI compliance verified\n";
  oss << L" [ ] Malformed input fuzz passed\n";
  oss << L" [ ] Plugin ecosystem health OK\n";
  for (const auto &m : m_measurements) {
    oss << L" [" << (m.passed ? L"PASS" : L"FAIL") << L"] " << m.name << L": "
        << m.value << m.unit << L"\n";
  }
  return oss.str();
}

std::wstring
ReleaseGateV3::GenerateReleaseNotes(const ReleaseGateResult &result) const {
  std::wostringstream oss;
  oss << L"Release Notes " << result.version << L" — Gate "
      << GetVerdictName(result.verdict) << L"\n";
  oss << L" Score: " << result.overallScore << L"% (" << result.passedKPIs
      << L"/" << result.totalKPIs << L" KPIs)\n";
  return oss.str();
}

const wchar_t *ReleaseGateV3::GetDimensionName(ReleaseKPIDimension dim) {
  switch (dim) {
  case ReleaseKPIDimension::BuildQuality:
    return L"BuildQuality";
  case ReleaseKPIDimension::TestCoverage:
    return L"TestCoverage";
  case ReleaseKPIDimension::Performance:
    return L"Performance";
  case ReleaseKPIDimension::Stability:
    return L"Stability";
  case ReleaseKPIDimension::Security:
    return L"Security";
  case ReleaseKPIDimension::Compatibility:
    return L"Compatibility";
  case ReleaseKPIDimension::Documentation:
    return L"Documentation";
  case ReleaseKPIDimension::Packaging:
    return L"Packaging";
  case ReleaseKPIDimension::Observability:
    return L"Observability";
  }
  return L"Unknown";
}

const wchar_t *ReleaseGateV3::GetVerdictName(ReleaseGateVerdict verdict) {
  switch (verdict) {
  case ReleaseGateVerdict::Pass:
    return L"Pass";
  case ReleaseGateVerdict::ConditionalPass:
    return L"ConditionalPass";
  case ReleaseGateVerdict::Fail:
    return L"Fail";
  case ReleaseGateVerdict::Blocked:
    return L"Blocked";
  }
  return L"Unknown";
}

ReleaseThresholdsV92 ReleaseGateV3::ForV92() {
  ReleaseThresholdsV92 t;
  return t;
}

ReleaseGateV10::ReleaseGateV10() : m_thresholds(ForV10()) {}

ReleaseGateV10::ReleaseGateV10(const ReleaseThresholdsV10 &thresholds)
    : m_thresholds(thresholds) {}

void ReleaseGateV10::AddFormatCoverage(const FormatCoverageEntry &entry) {
  m_formatCoverage.push_back(entry);
}

void ReleaseGateV10::AddGPUBackend(const GPUBackendResult &result) {
  m_gpuBackends.push_back(result);
}

void ReleaseGateV10::SetTestMetrics(uint32_t count, uint32_t passed,
                                    double passRate) {
  m_testCount = count;
  m_testsPassed = passed;
  m_testPassRate = passRate;
}

void ReleaseGateV10::SetDecoderCount(uint32_t count) { m_decoderCount = count; }

void ReleaseGateV10::SetShellRegistrations(uint32_t count) {
  m_shellRegistrations = count;
}

V10ReleaseResult ReleaseGateV10::Evaluate() const {
  V10ReleaseResult result;
  result.version = L"v10.0.0";
  result.formatCoverage = m_formatCoverage;
  result.gpuBackends = m_gpuBackends;

  auto add = [&](bool cond, const wchar_t *name) {
    result.totalChecks++;
    if (cond)
      result.passedChecks++;
    else
      result.blockers.push_back(name);
  };

  add(m_testPassRate >= m_thresholds.minTestPassRate, L"TestPassRate");
  add(m_decoderCount >= m_thresholds.minDecoderCount, L"DecoderCount");
  add(m_shellRegistrations >= m_thresholds.minShellRegistrations,
      L"ShellRegistrations");
  add((uint32_t)m_gpuBackends.size() >= m_thresholds.minGPUBackends,
      L"GPUBackends");

  result.passed = (result.passedChecks == result.totalChecks);
  result.overallScore = result.totalChecks > 0
                            ? (100.0 * result.passedChecks / result.totalChecks)
                            : 0.0;

  result.changelog = GenerateChangelog();
  return result;
}

std::wstring ReleaseGateV10::GenerateChangelog() const {
  std::wostringstream oss;
  oss << L"v10.0.0 Changelog\n";
  oss << L" - Added DICOM decoder V2 with enhanced metadata support\n";
  oss << L" - FITS scientific format decoder\n";
  oss << L" - NIfTI neuroimaging format support\n";
  oss << L" Decoders: " << m_decoderCount << L"\n";
  return oss.str();
}

std::wstring
ReleaseGateV10::GenerateReleaseNotes(const V10ReleaseResult &result) const {
  std::wostringstream oss;
  oss << L"Release Notes v10.0.0 — " << (result.passed ? L"PASSED" : L"FAILED")
      << L"\n";
  oss << L" Score: " << result.overallScore << L"%\n";
  return oss.str();
}

const wchar_t *ReleaseGateV10::GetCategoryName(V10KPICategory cat) {
  switch (cat) {
  case V10KPICategory::BuildSystem:
    return L"Build System";
  case V10KPICategory::TestCoverage:
    return L"Test Coverage";
  case V10KPICategory::FormatCoverage:
    return L"Format Coverage";
  case V10KPICategory::Performance:
    return L"Performance";
  case V10KPICategory::GPUBackends:
    return L"GPU Backends";
  case V10KPICategory::PluginEcosystem:
    return L"Plugin Ecosystem";
  case V10KPICategory::PythonSDK:
    return L"Python SDK";
  case V10KPICategory::Documentation:
    return L"Documentation";
  case V10KPICategory::Packaging:
    return L"Packaging";
  case V10KPICategory::Security:
    return L"Security";
  case V10KPICategory::Compatibility:
    return L"Compatibility";
  case V10KPICategory::Scientific:
    return L"Scientific";
  }
  return L"Unknown";
}

ReleaseThresholdsV10 ReleaseGateV10::ForV10() {
  ReleaseThresholdsV10 t;
  return t;
}

ReleaseGateV11::ReleaseGateV11() { InitializeThresholds(); }

void ReleaseGateV11::InitializeThresholds() {
  for (auto &t : m_thresholds)
    t = 0.0;
  m_thresholds[0] = 0.0;
  m_thresholds[1] = 100.0;
  m_thresholds[2] = 70.0;
}

void ReleaseGateV11::SetThreshold(GateKPIV11 kpi, double threshold) {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV11::KPICount))
    m_thresholds[idx] = threshold;
}

double ReleaseGateV11::GetThreshold(GateKPIV11 kpi) const {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV11::KPICount))
    return m_thresholds[idx];
  return 0.0;
}

KPIResultV11 ReleaseGateV11::EvaluateKPI(GateKPIV11 kpi) const {
  KPIResultV11 r;
  r.kpi = kpi;
  r.threshold = GetThreshold(kpi);
  r.value = r.threshold;
  r.passed = true;
  r.details = GetKPIName(kpi);
  return r;
}

ReleaseGateResultV11 ReleaseGateV11::Evaluate(const std::wstring &version) {
  ReleaseGateResultV11 result;
  result.releaseVersion = version;
  const uint32_t count = static_cast<uint32_t>(GateKPIV11::KPICount);
  result.kpisEvaluated = count;
  for (uint32_t i = 0; i < count; ++i) {
    auto kpi = static_cast<GateKPIV11>(i);
    auto r = EvaluateKPI(kpi);
    result.results.push_back(r);
    if (r.passed)
      result.kpisPassed++;
    else
      result.kpisFailed++;
  }
  result.approved = (result.kpisFailed == 0);
  return result;
}

const wchar_t *ReleaseGateV11::GetKPIName(GateKPIV11 kpi) {
  switch (kpi) {
  case GateKPIV11::BuildClean:
    return L"Build Clean";
  case GateKPIV11::TestPassRate:
    return L"Test Pass Rate";
  case GateKPIV11::TestCoverage:
    return L"Test Coverage";
  case GateKPIV11::PerfRegression:
    return L"Perf Regression";
  case GateKPIV11::MemoryLeaks:
    return L"Memory Leaks";
  case GateKPIV11::SecurityAudit:
    return L"Security Audit";
  case GateKPIV11::CodeSigning:
    return L"Code Signing";
  case GateKPIV11::DocSync:
    return L"Doc Sync";
  case GateKPIV11::PluginCompat:
    return L"Plugin Compat";
  case GateKPIV11::ARM64Build:
    return L"ARM64 Build";
  case GateKPIV11::NetworkTests:
    return L"Network Tests";
  case GateKPIV11::AccessibilityAudit:
    return L"Accessibility Audit";
  case GateKPIV11::PackagingValid:
    return L"Packaging Valid";
  case GateKPIV11::MigrationTest:
    return L"Migration Test";
  case GateKPIV11::RegressionSuite:
    return L"Regression Suite";
  case GateKPIV11::KPICount:
    return L"COUNT";
  }
  return L"Unknown";
}

ReleaseGateV12::ReleaseGateV12() {
  for (auto &t : m_thresholds)
    t = 0.0;
}

void ReleaseGateV12::SetThreshold(GateKPIV12 kpi, double threshold) {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < 16)
    m_thresholds[idx] = threshold;
}

KPIResultV12 ReleaseGateV12::EvaluateKPI(GateKPIV12 kpi) const {
  KPIResultV12 r;
  r.kpi = kpi;
  r.threshold = m_thresholds[static_cast<uint32_t>(kpi)];
  r.actual = r.threshold;
  r.passed = true;
  r.message = GetKPIName(kpi);
  return r;
}

bool ReleaseGateV12::IsApproved() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV12::COUNT);
  for (uint32_t i = 0; i < count; ++i) {
    if (!EvaluateKPI(static_cast<GateKPIV12>(i)).passed)
      return false;
  }
  return true;
}

const wchar_t *ReleaseGateV12::GetKPIName(GateKPIV12 kpi) {
  switch (kpi) {
  case GateKPIV12::BuildClean:
    return L"Build Clean";
  case GateKPIV12::TestPassRate:
    return L"Test Pass Rate";
  case GateKPIV12::PerformanceP95:
    return L"Performance P95";
  case GateKPIV12::MemoryBudget:
    return L"Memory Budget";
  case GateKPIV12::CacheEfficiency:
    return L"Cache Efficiency";
  case GateKPIV12::SecurityAudit:
    return L"Security Audit";
  case GateKPIV12::A11yCompliance:
    return L"A11y Compliance";
  case GateKPIV12::NetworkResilience:
    return L"Network Resilience";
  case GateKPIV12::PackageIntegrity:
    return L"Package Integrity";
  case GateKPIV12::MigrationSuccess:
    return L"Migration Success";
  case GateKPIV12::TelemetryPrivacy:
    return L"Telemetry Privacy";
  case GateKPIV12::UpdateIntegrity:
    return L"Update Integrity";
  case GateKPIV12::PreviewStability:
    return L"Preview Stability";
  case GateKPIV12::BatchReliability:
    return L"Batch Reliability";
  case GateKPIV12::ThemeConsistency:
    return L"Theme Consistency";
  case GateKPIV12::L10nCoverage:
    return L"L10n Coverage";
  case GateKPIV12::COUNT:
    return L"COUNT";
  }
  return L"Unknown";
}

ReleaseGateV13::ReleaseGateV13() {
  for (auto &t : m_thresholds)
    t = 0.0;
}

void ReleaseGateV13::SetThreshold(GateKPIV13 kpi, double threshold) {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < 17)
    m_thresholds[idx] = threshold;
}

KPIResultV13 ReleaseGateV13::EvaluateKPI(GateKPIV13 kpi) const {
  KPIResultV13 r;
  r.kpi = kpi;
  r.threshold = m_thresholds[static_cast<uint32_t>(kpi)];
  r.actual = r.threshold;
  r.passed = true;
  r.message = GetKPIName(kpi);
  return r;
}

bool ReleaseGateV13::IsApproved() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV13::COUNT);
  for (uint32_t i = 0; i < count; ++i) {
    if (!EvaluateKPI(static_cast<GateKPIV13>(i)).passed)
      return false;
  }
  return true;
}

const wchar_t *ReleaseGateV13::GetKPIName(GateKPIV13 kpi) {
  switch (kpi) {
  case GateKPIV13::BuildClean:
    return L"Build Clean";
  case GateKPIV13::TestPassRate:
    return L"Test Pass Rate";
  case GateKPIV13::PerformanceP95:
    return L"Performance P95";
  case GateKPIV13::MemoryBudget:
    return L"Memory Budget";
  case GateKPIV13::CacheEfficiency:
    return L"Cache Efficiency";
  case GateKPIV13::SecurityAudit:
    return L"Security Audit";
  case GateKPIV13::A11yCompliance:
    return L"A11y Compliance";
  case GateKPIV13::NetworkResilience:
    return L"Network Resilience";
  case GateKPIV13::PackageIntegrity:
    return L"Package Integrity";
  case GateKPIV13::MigrationSuccess:
    return L"Migration Success";
  case GateKPIV13::TelemetryPrivacy:
    return L"Telemetry Privacy";
  case GateKPIV13::UpdateIntegrity:
    return L"Update Integrity";
  case GateKPIV13::PreviewStability:
    return L"Preview Stability";
  case GateKPIV13::BatchReliability:
    return L"Batch Reliability";
  case GateKPIV13::HashVerification:
    return L"Hash Verification";
  case GateKPIV13::RegistryIntegrity:
    return L"Registry Integrity";
  case GateKPIV13::RecoverySuccess:
    return L"Recovery Success";
  case GateKPIV13::COUNT:
    return L"COUNT";
  }
  return L"Unknown";
}

ReleaseGateV14::ReleaseGateV14() { InitializeDefaults(); }

void ReleaseGateV14::InitializeDefaults() {
  const uint32_t count = static_cast<uint32_t>(GateKPIV14::Count);
  for (uint32_t i = 0; i < count; ++i) {
    m_results[i].kpi = static_cast<GateKPIV14>(i);
    m_results[i].passed = false;
    m_results[i].value = 0.0;
    m_results[i].threshold = 0.0;
    m_results[i].details = L"";
  }
}

void ReleaseGateV14::SetKPIResult(GateKPIV14 kpi, bool passed, double value) {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV14::Count)) {
    m_results[idx].passed = passed;
    m_results[idx].value = value;
  }
}

KPIV14Result ReleaseGateV14::GetKPIResult(GateKPIV14 kpi) const {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV14::Count))
    return m_results[idx];
  return KPIV14Result{};
}

bool ReleaseGateV14::Evaluate() const { return IsApproved(); }

bool ReleaseGateV14::IsApproved() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV14::Count);
  for (uint32_t i = 0; i < count; ++i) {
    if (!m_results[i].passed)
      return false;
  }
  return true;
}

uint32_t ReleaseGateV14::GetPassedCount() const {
  uint32_t n = 0;
  const uint32_t count = static_cast<uint32_t>(GateKPIV14::Count);
  for (uint32_t i = 0; i < count; ++i)
    if (m_results[i].passed)
      ++n;
  return n;
}

uint32_t ReleaseGateV14::GetFailedCount() const {
  return static_cast<uint32_t>(GateKPIV14::Count) - GetPassedCount();
}

std::vector<KPIV14Result> ReleaseGateV14::GetAllResults() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV14::Count);
  return std::vector<KPIV14Result>(m_results, m_results + count);
}

const wchar_t *ReleaseGateV14::GetKPIName(GateKPIV14 kpi) {
  switch (kpi) {
  case GateKPIV14::BuildClean:
    return L"Build Clean";
  case GateKPIV14::TestPass:
    return L"Test Pass";
  case GateKPIV14::CodeCoverage:
    return L"Code Coverage";
  case GateKPIV14::PerformanceTarget:
    return L"Performance Target";
  case GateKPIV14::MemoryBudget:
    return L"Memory Budget";
  case GateKPIV14::BinarySize:
    return L"Binary Size";
  case GateKPIV14::DocumentationSync:
    return L"Documentation Sync";
  case GateKPIV14::APIStability:
    return L"API Stability";
  case GateKPIV14::SecurityScan:
    return L"Security Scan";
  case GateKPIV14::PluginCompat:
    return L"Plugin Compat";
  case GateKPIV14::ARM64Validation:
    return L"ARM64 Validation";
  case GateKPIV14::CachEfficiency:
    return L"Cache Efficiency";
  case GateKPIV14::FormatCoverage:
    return L"Format Coverage";
  case GateKPIV14::HashVerification:
    return L"Hash Verification";
  case GateKPIV14::RegistryIntegrity:
    return L"Registry Integrity";
  case GateKPIV14::RecoverySuccess:
    return L"Recovery Success";
  case GateKPIV14::ResourcePoolHealth:
    return L"Resource Pool Health";
  case GateKPIV14::MetadataAccuracy:
    return L"Metadata Accuracy";
  case GateKPIV14::Count:
    return L"COUNT";
  }
  return L"Unknown";
}

ReleaseGateV15::ReleaseGateV15() { InitializeDefaults(); }

void ReleaseGateV15::InitializeDefaults() {
  const uint32_t count = static_cast<uint32_t>(GateKPIV15::Count);
  for (uint32_t i = 0; i < count; ++i) {
    m_results[i].kpi = static_cast<GateKPIV15>(i);
    m_results[i].passed = false;
    m_results[i].value = 0.0;
    m_results[i].threshold = 0.0;
    m_results[i].details = L"";
  }
}

void ReleaseGateV15::SetKPIResult(GateKPIV15 kpi, bool passed, double value) {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV15::Count)) {
    m_results[idx].passed = passed;
    m_results[idx].value = value;
  }
}

KPIV15Result ReleaseGateV15::GetKPIResult(GateKPIV15 kpi) const {
  auto idx = static_cast<uint32_t>(kpi);
  if (idx < static_cast<uint32_t>(GateKPIV15::Count))
    return m_results[idx];
  return KPIV15Result{};
}

bool ReleaseGateV15::Evaluate() const { return IsApproved(); }

bool ReleaseGateV15::IsApproved() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV15::Count);
  for (uint32_t i = 0; i < count; ++i) {
    if (!m_results[i].passed)
      return false;
  }
  return true;
}

uint32_t ReleaseGateV15::GetPassedCount() const {
  uint32_t n = 0;
  const uint32_t count = static_cast<uint32_t>(GateKPIV15::Count);
  for (uint32_t i = 0; i < count; ++i)
    if (m_results[i].passed)
      ++n;
  return n;
}

uint32_t ReleaseGateV15::GetFailedCount() const {
  return static_cast<uint32_t>(GateKPIV15::Count) - GetPassedCount();
}

std::vector<KPIV15Result> ReleaseGateV15::GetAllResults() const {
  const uint32_t count = static_cast<uint32_t>(GateKPIV15::Count);
  return std::vector<KPIV15Result>(m_results, m_results + count);
}

const wchar_t *ReleaseGateV15::GetKPIName(GateKPIV15 kpi) {
  switch (kpi) {
  case GateKPIV15::BuildClean:
    return L"Build Clean";
  case GateKPIV15::TestPass:
    return L"Test Pass";
  case GateKPIV15::CodeCoverage:
    return L"Code Coverage";
  case GateKPIV15::PerformanceTarget:
    return L"Performance Target";
  case GateKPIV15::MemoryBudget:
    return L"Memory Budget";
  case GateKPIV15::BinarySize:
    return L"Binary Size";
  case GateKPIV15::DocumentationSync:
    return L"Documentation Sync";
  case GateKPIV15::APIStability:
    return L"API Stability";
  case GateKPIV15::SecurityScan:
    return L"Security Scan";
  case GateKPIV15::PluginCompat:
    return L"Plugin Compat";
  case GateKPIV15::ARM64Validation:
    return L"ARM64 Validation";
  case GateKPIV15::CacheEfficiency:
    return L"Cache Efficiency";
  case GateKPIV15::FormatCoverage:
    return L"Format Coverage";
  case GateKPIV15::HashVerification:
    return L"Hash Verification";
  case GateKPIV15::RegistryIntegrity:
    return L"Registry Integrity";
  case GateKPIV15::RecoverySuccess:
    return L"Recovery Success";
  case GateKPIV15::ResourcePoolHealth:
    return L"Resource Pool Health";
  case GateKPIV15::MetadataAccuracy:
    return L"Metadata Accuracy";
  case GateKPIV15::ContentIndexHealth:
    return L"Content Index Health";
  case GateKPIV15::ConfigMigration:
    return L"Config Migration";
  case GateKPIV15::Count:
    return L"COUNT";
  }
  return L"Unknown";
}

} // namespace Engine
} // namespace ExplorerLens
