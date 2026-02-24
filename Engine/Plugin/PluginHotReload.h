//==============================================================================
// ExplorerLens Engine — Plugin Hot-Reload
// Zero-downtime plugin update with live DLL swap, state migration,
// decoder re-registration, and fallback rollback on failed reload.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HotReloadTrigger : uint8_t {
  Manual = 0,               // User-initiated
  FileChange,               // DLL file modification detected
  Scheduled,                // Periodic check
  ManifestUpdate,           // Plugin manifest version bump
  FileChanged = FileChange, // compat alias
  COUNT = ManifestUpdate + 1
};

enum class HotReloadState : uint8_t {
  Idle = 0,
  Detecting,
  Preparing,
  Swapping,
  Verifying,
  Committed,
  RolledBack,
  Failed,
  COUNT
};

enum class HotReloadPolicy : uint8_t {
  AlwaysReload = 0,
  ReloadIfIdle, // Only when no active decodes
  DelaySafe,    // Wait for drain then reload
  Disabled,
  Automatic = AlwaysReload, // compat alias
  COUNT = Disabled + 1
};

struct HotReloadEvent {
  std::wstring pluginId;
  HotReloadTrigger trigger = HotReloadTrigger::Manual;
  HotReloadState state = HotReloadState::Idle;
  bool success = false;
  uint32_t durationMs = 0;
};

class PluginHotReload {
public:
  static const wchar_t *TriggerName(HotReloadTrigger t) {
    switch (t) {
    case HotReloadTrigger::Manual:
      return L"Manual";
    case HotReloadTrigger::FileChange:
      return L"File Change";
    case HotReloadTrigger::Scheduled:
      return L"Scheduled";
    case HotReloadTrigger::ManifestUpdate:
      return L"Manifest Update";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *StateName(HotReloadState s) {
    switch (s) {
    case HotReloadState::Idle:
      return L"Idle";
    case HotReloadState::Detecting:
      return L"Detecting";
    case HotReloadState::Preparing:
      return L"Preparing";
    case HotReloadState::Swapping:
      return L"Swapping";
    case HotReloadState::Verifying:
      return L"Verifying";
    case HotReloadState::Committed:
      return L"Committed";
    case HotReloadState::RolledBack:
      return L"Rolled Back";
    case HotReloadState::Failed:
      return L"Failed";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PolicyName(HotReloadPolicy p) {
    switch (p) {
    case HotReloadPolicy::AlwaysReload:
      return L"Always Reload";
    case HotReloadPolicy::ReloadIfIdle:
      return L"Reload If Idle";
    case HotReloadPolicy::DelaySafe:
      return L"Delay Safe";
    case HotReloadPolicy::Disabled:
      return L"Disabled";
    default:
      return L"Unknown";
    }
  }

  static constexpr size_t TriggerCount() {
    return static_cast<size_t>(HotReloadTrigger::COUNT);
  }
  static constexpr size_t StateCount() {
    return static_cast<size_t>(HotReloadState::COUNT);
  }
  static constexpr size_t PolicyCount() {
    return static_cast<size_t>(HotReloadPolicy::COUNT);
  }
};

} // namespace Engine
} // namespace ExplorerLens
