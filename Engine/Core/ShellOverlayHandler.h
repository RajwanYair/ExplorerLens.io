// ShellOverlayHandler.h — Shell extension icon overlay management
// Copyright (c) 2026 ExplorerLens Project
//
// Provides overlay icon types, positions, and validation for Windows Shell
// Icon Overlay Handlers (IShellIconOverlayIdentifier).
//
#pragma once
#include <cstdint>
#include <cstddef>
#include <cwchar>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

enum class OverlayIconType : uint8_t {
    Cached          = 0,
    NotCached       = 1,
    Processing      = 2,
    Error           = 3,
    Warning         = 4,
    Protected       = 5,
    CloudSynced     = 6,
    COUNT
};

enum class OverlayPosition : uint8_t {
    BottomRight = 0,
    BottomLeft  = 1,
    TopRight    = 2,
    TopLeft     = 3,
    COUNT
};

struct OverlayIconConfig {
    OverlayPosition position  = OverlayPosition::BottomRight;
    uint32_t        iconSize  = 16;
    bool            enabled   = true;
    bool            useHiDPI  = true;
};

class ShellOverlayHandler {
public:
    static const wchar_t* OverlayName(OverlayIconType t) noexcept {
        switch (t) {
        case OverlayIconType::Cached:      return L"Cached";
        case OverlayIconType::NotCached:   return L"Not Cached";
        case OverlayIconType::Processing:  return L"Processing";
        case OverlayIconType::Error:       return L"Error";
        case OverlayIconType::Warning:     return L"Warning";
        case OverlayIconType::Protected:   return L"Protected";
        case OverlayIconType::CloudSynced: return L"Cloud Synced";
        default:                           return L"Unknown";
        }
    }

    static const wchar_t* PositionName(OverlayPosition p) noexcept {
        switch (p) {
        case OverlayPosition::BottomRight: return L"Bottom Right";
        case OverlayPosition::BottomLeft:  return L"Bottom Left";
        case OverlayPosition::TopRight:    return L"Top Right";
        case OverlayPosition::TopLeft:     return L"Top Left";
        default:                           return L"Unknown";
        }
    }

    static bool ValidateOpacity(float opacity) noexcept {
        return opacity >= 0.0f && opacity <= 1.0f;
    }

    static constexpr size_t OverlayCount() noexcept {
        return static_cast<size_t>(OverlayIconType::COUNT);
    }

    static constexpr size_t PositionCount() noexcept {
        return static_cast<size_t>(OverlayPosition::COUNT);
    }
};

} // namespace Engine
} // namespace ExplorerLens
