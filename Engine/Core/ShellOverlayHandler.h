//==============================================================================
// ExplorerLens Engine — Shell Overlay Icon Handler
// IShellIconOverlayIdentifier for status overlay icons on thumbnails.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Overlay icon type
enum class OverlayIconType : uint8_t {
    Cached,         // Thumbnail is cached
    Processing,     // Currently generating
    Error,          // Decode error
    Unsupported,    // Format not supported
    Encrypted,      // Password-protected archive
    Corrupted,      // File appears corrupted
    Large,          // File exceeds size limit
    COUNT
};

/// Overlay position
enum class OverlayPosition : uint8_t {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    COUNT
};

/// Overlay icon config
struct OverlayIconConfig {
    OverlayPosition position    = OverlayPosition::BottomRight;
    uint32_t        iconSize    = 16;
    float           opacity     = 0.85f;
    bool            enabled     = true;
    bool            showOnHover = false;
};

/// Shell overlay icon handler
class ShellOverlayHandler {
public:
    static const wchar_t* OverlayName(OverlayIconType t) {
        switch (t) {
            case OverlayIconType::Cached:      return L"Cached";
            case OverlayIconType::Processing:  return L"Processing";
            case OverlayIconType::Error:       return L"Error";
            case OverlayIconType::Unsupported: return L"Unsupported";
            case OverlayIconType::Encrypted:   return L"Encrypted";
            case OverlayIconType::Corrupted:   return L"Corrupted";
            case OverlayIconType::Large:       return L"Large File";
            default: return L"Unknown";
        }
    }

    static const wchar_t* PositionName(OverlayPosition p) {
        switch (p) {
            case OverlayPosition::TopLeft:     return L"Top Left";
            case OverlayPosition::TopRight:    return L"Top Right";
            case OverlayPosition::BottomLeft:  return L"Bottom Left";
            case OverlayPosition::BottomRight: return L"Bottom Right";
            default: return L"Unknown";
        }
    }

    static constexpr size_t OverlayCount() { return static_cast<size_t>(OverlayIconType::COUNT); }
    static constexpr size_t PositionCount() { return static_cast<size_t>(OverlayPosition::COUNT); }

    static bool ValidateOpacity(float o) { return o >= 0.0f && o <= 1.0f; }
};

}} // namespace ExplorerLens::Engine

