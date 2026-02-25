#pragma once
// RoleBasedFormatPolicy.h — Role-Based Format Policy Engine
// Enterprise policy enforcement for format access control — restricts which
// file formats can generate thumbnails based on AD group membership or GPO.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Policy action for a format
enum class FormatPolicyAction : uint8_t {
  Allow = 0,   // Full thumbnail generation
  AllowCached, // Only serve from cache (no decode)
  Placeholder, // Show generic icon only
  Block,       // Deny thumbnail entirely
  AuditOnly,   // Generate but log access
  COUNT
};

/// Policy source
enum class FormatPolicySource : uint8_t {
  Default = 0,     // Built-in default
  UserPreference,  // Local user setting
  GroupPolicy,     // AD Group Policy Object
  IntunePolicy,    // Microsoft Intune MDM
  ConfigMgrPolicy, // SCCM/ConfigMgr
  COUNT
};

struct FormatPolicyRule {
  const wchar_t *extensionPattern = nullptr; // e.g., "*.exe", "*.psd"
  FormatPolicyAction action = FormatPolicyAction::Allow;
  FormatPolicySource source = FormatPolicySource::Default;
  uint32_t priority = 0;
  bool inherited = false;
};

class RoleBasedFormatPolicy {
public:
  static constexpr size_t ActionCount() {
    return static_cast<size_t>(FormatPolicyAction::COUNT);
  }
  static constexpr size_t SourceCount() {
    return static_cast<size_t>(FormatPolicySource::COUNT);
  }

  static const wchar_t *ActionName(FormatPolicyAction a) {
    switch (a) {
    case FormatPolicyAction::Allow:
      return L"Allow";
    case FormatPolicyAction::AllowCached:
      return L"Allow Cached";
    case FormatPolicyAction::Placeholder:
      return L"Placeholder";
    case FormatPolicyAction::Block:
      return L"Block";
    case FormatPolicyAction::AuditOnly:
      return L"Audit Only";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *SourceName(FormatPolicySource s) {
    switch (s) {
    case FormatPolicySource::Default:
      return L"Default";
    case FormatPolicySource::UserPreference:
      return L"User Preference";
    case FormatPolicySource::GroupPolicy:
      return L"Group Policy";
    case FormatPolicySource::IntunePolicy:
      return L"Intune";
    case FormatPolicySource::ConfigMgrPolicy:
      return L"ConfigMgr";
    default:
      return L"Unknown";
    }
  }

  /// Higher priority source wins
  static FormatPolicySource HigherPriority(FormatPolicySource a,
                                           FormatPolicySource b) {
    // GPO > Intune > ConfigMgr > User > Default
    return (static_cast<uint8_t>(a) > static_cast<uint8_t>(b)) ? a : b;
  }
};

} // namespace Engine
} // namespace ExplorerLens
