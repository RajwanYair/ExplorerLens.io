//==============================================================================
// ExplorerLens Engine — Windows 12 Compatibility Layer
// Runtime detection of Windows 12 shell APIs, adaptive rendering paths for
// new window compositor features, Fluent v4 style adaptation, and
// graceful fallback to Windows 11 behaviour.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class Win12Feature : uint8_t {
    DynamicIsland   = 0,  // Windows 12 Dynamic Island shell integration
    FluentV4        = 1,  // Fluent Design System V4
    AIProcessingUnit= 2,  // NPU tile APIs
    SmartDock       = 3,  // Snap / Smart Dock layouts V2
    UnifiedSearch   = 4,  // Unified search provider protocol
    COUNT
};

enum class Win12CompatMode : uint8_t { Native=0, Adaptive, Fallback11, Fallback10, COUNT };
enum class Win12APIFamily  : uint8_t { Shell=0, Compositor, InputMethod, SystemTray, COUNT };

struct Win12FeatureAvailability {
    Win12Feature feature        = Win12Feature::FluentV4;
    bool         available      = false;
    Win12CompatMode mode        = Win12CompatMode::Fallback11;
    std::wstring minBuildNumber;
};

class Windows12Compatibility {
public:
    static const wchar_t* FeatureName(Win12Feature f) {
        switch(f) {
            case Win12Feature::DynamicIsland:    return L"Dynamic Island Shell";
            case Win12Feature::FluentV4:         return L"Fluent Design V4";
            case Win12Feature::AIProcessingUnit: return L"NPU AI Processing Unit";
            case Win12Feature::SmartDock:        return L"Smart Dock Layouts V2";
            case Win12Feature::UnifiedSearch:    return L"Unified Search Provider";
            default: return L"Unknown";
        }
    }
    static const wchar_t* CompatModeName(Win12CompatMode m) {
        switch(m) {
            case Win12CompatMode::Native:      return L"Native";
            case Win12CompatMode::Adaptive:    return L"Adaptive";
            case Win12CompatMode::Fallback11:  return L"Fallback (Win11)";
            case Win12CompatMode::Fallback10:  return L"Fallback (Win10)";
            default: return L"Unknown";
        }
    }
    static const wchar_t* APIFamilyName(Win12APIFamily a) {
        switch(a) {
            case Win12APIFamily::Shell:       return L"Shell";
            case Win12APIFamily::Compositor:  return L"Compositor";
            case Win12APIFamily::InputMethod: return L"Input Method";
            case Win12APIFamily::SystemTray:  return L"System Tray";
            default: return L"Unknown";
        }
    }
    static constexpr size_t FeatureCount()    { return static_cast<size_t>(Win12Feature::COUNT); }
    static constexpr size_t CompatModeCount() { return static_cast<size_t>(Win12CompatMode::COUNT); }
    static constexpr size_t APIFamilyCount()  { return static_cast<size_t>(Win12APIFamily::COUNT); }
};

}} // namespace ExplorerLens::Engine

