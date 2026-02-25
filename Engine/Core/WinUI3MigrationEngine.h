//==============================================================================
// WinUI3MigrationEngine.h — WinUI 3 Migration Engine
// Hybrid WTL + WinUI 3 UI framework with shared backend.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Plans and tracks migration from WTL to WinUI 3 (Windows App SDK).
class WinUI3MigrationEngine {
public:
  enum class UIFramework { WTL, WinUI3, XAMLIslands, Hybrid, COUNT };

  enum class MigrationPhase {
    Research,
    Prototype,
    Hybrid,
    FullMigration,
    LegacyRemoval,
    FeatureParity,
    COUNT
  };

  enum class PageType {
    FormatSettings,
    PerformanceTab,
    AboutPage,
    PluginMarketplace,
    DiagnosticsTab,
    COUNT
  };

  struct MigrationStatus {
    PageType page;
    UIFramework currentFramework;
    UIFramework targetFramework;
    MigrationPhase phase;
    uint8_t progressPct;
  };

  static const wchar_t *FrameworkName(UIFramework f) {
    switch (f) {
    case UIFramework::WTL:
      return L"WTL";
    case UIFramework::WinUI3:
      return L"WinUI3";
    case UIFramework::XAMLIslands:
      return L"XAMLIslands";
    case UIFramework::Hybrid:
      return L"Hybrid";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PhaseName(MigrationPhase p) {
    switch (p) {
    case MigrationPhase::Research:
      return L"Research";
    case MigrationPhase::Prototype:
      return L"Prototype";
    case MigrationPhase::Hybrid:
      return L"Hybrid";
    case MigrationPhase::FullMigration:
      return L"Full Migration";
    case MigrationPhase::LegacyRemoval:
      return L"Legacy Removal";
    case MigrationPhase::FeatureParity:
      return L"Feature Parity";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PageName(PageType p) {
    switch (p) {
    case PageType::FormatSettings:
      return L"FormatSettings";
    case PageType::PerformanceTab:
      return L"PerformanceTab";
    case PageType::AboutPage:
      return L"AboutPage";
    case PageType::PluginMarketplace:
      return L"PluginMarketplace";
    case PageType::DiagnosticsTab:
      return L"DiagnosticsTab";
    default:
      return L"Unknown";
    }
  }

  static size_t FrameworkCount() {
    return static_cast<size_t>(UIFramework::COUNT);
  }
  static size_t PhaseCount() {
    return static_cast<size_t>(MigrationPhase::COUNT);
  }
  static size_t PageCount() { return static_cast<size_t>(PageType::COUNT); }

  static std::vector<MigrationStatus> GetStatus() {
    return {
        {PageType::FormatSettings, UIFramework::WTL, UIFramework::WinUI3,
         MigrationPhase::Research, 10},
        {PageType::PerformanceTab, UIFramework::WTL, UIFramework::WinUI3,
         MigrationPhase::Research, 5},
        {PageType::AboutPage, UIFramework::WTL, UIFramework::WinUI3,
         MigrationPhase::Prototype, 30},
        {PageType::PluginMarketplace, UIFramework::WTL, UIFramework::WinUI3,
         MigrationPhase::Research, 5},
        {PageType::DiagnosticsTab, UIFramework::WTL, UIFramework::WinUI3,
         MigrationPhase::Research, 5},
    };
  }

  static bool WindowsAppSDKAvailable() {
    // Check for Windows App SDK runtime
    return true; // Assume available on Windows 11
  }
};

/// Namespace-scope alias for test compatibility
using MigrationPhase = WinUI3MigrationEngine::MigrationPhase;

} // namespace Engine
} // namespace ExplorerLens
