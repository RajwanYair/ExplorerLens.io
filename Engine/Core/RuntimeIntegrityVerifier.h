//==============================================================================
// ExplorerLens Engine — Runtime Integrity Verifier
// Authenticode verification of loaded modules, CFG (Control Flow Guard)
// validation, shadow stack compliance, and anti-tamper runtime checks.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class IntegrityCheckType : uint8_t {
  Authenticode = 0,
  CFGCompliance,
  ShadowStack,
  IAT_Integrity,
  PEChecksum,
  CodeSigning = Authenticode, // compat alias
  COUNT = PEChecksum + 1
};
enum class IntegrityVerifyResult : uint8_t {
  Pass = 0,
  Fail,
  Skip,
  NotSupported,
  COUNT
};
enum class TamperIndicator : uint8_t {
  None = 0,
  PatchedCode,
  HookedIAT,
  InjectedDLL,
  CorruptPE,
  COUNT
};

struct IntegrityCheckResult {
  IntegrityCheckType check = IntegrityCheckType::Authenticode;
  IntegrityVerifyResult result = IntegrityVerifyResult::Pass;
  std::wstring moduleName;
  std::wstring detail;
};

struct RuntimeIntegrityReport {
  uint32_t passed = 0;
  uint32_t failed = 0;
  uint32_t skipped = 0;
  TamperIndicator worstTamper = TamperIndicator::None;
  bool trustworthy = true;
};

class RuntimeIntegrityVerifier {
public:
  static const wchar_t *CheckTypeName(IntegrityCheckType t) {
    switch (t) {
    case IntegrityCheckType::Authenticode:
      return L"Authenticode";
    case IntegrityCheckType::CFGCompliance:
      return L"CFG Compliance";
    case IntegrityCheckType::ShadowStack:
      return L"Shadow Stack";
    case IntegrityCheckType::IAT_Integrity:
      return L"IAT Integrity";
    case IntegrityCheckType::PEChecksum:
      return L"PE Checksum";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *VerifyResultName(IntegrityVerifyResult r) {
    switch (r) {
    case IntegrityVerifyResult::Pass:
      return L"Pass";
    case IntegrityVerifyResult::Fail:
      return L"Fail";
    case IntegrityVerifyResult::Skip:
      return L"Skip";
    case IntegrityVerifyResult::NotSupported:
      return L"Not Supported";
    default:
      return L"Unknown";
    }
  }
  static const wchar_t *TamperIndicatorName(TamperIndicator t) {
    switch (t) {
    case TamperIndicator::None:
      return L"None";
    case TamperIndicator::PatchedCode:
      return L"Patched Code";
    case TamperIndicator::HookedIAT:
      return L"Hooked IAT";
    case TamperIndicator::InjectedDLL:
      return L"Injected DLL";
    case TamperIndicator::CorruptPE:
      return L"Corrupt PE";
    default:
      return L"Unknown";
    }
  }
  static constexpr size_t CheckTypeCount() {
    return static_cast<size_t>(IntegrityCheckType::COUNT);
  }
  static constexpr size_t VerifyResultCount() {
    return static_cast<size_t>(IntegrityVerifyResult::COUNT);
  }
  static constexpr size_t TamperCount() {
    return static_cast<size_t>(TamperIndicator::COUNT);
  }

  // Compatibility aliases (tests)
  static const wchar_t *TamperName(TamperIndicator t) {
    return TamperIndicatorName(t);
  }
  static bool IsTrustworthy(const RuntimeIntegrityReport &r) {
    return r.failed == 0 && r.worstTamper == TamperIndicator::None;
  }
};

} // namespace Engine
} // namespace ExplorerLens
