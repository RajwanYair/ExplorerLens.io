#pragma once
// QuickActionsOverlay.h — Quick Actions Overlay
// Contextual action buttons overlaid on thumbnails — rotate, crop, convert,
// share, and open-with shortcuts accessible on hover.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Quick action type
enum class QuickActionType : uint8_t {
    RotateLeft = 0,
    RotateRight,
    Crop,
    ConvertFormat,
    ShareFile,
    OpenWith,
    CopyToClipboard,
    SetAsWallpaper,
    ShowProperties,
    COUNT
};

/// Overlay trigger
enum class OverlayTrigger : uint8_t {
    Hover = 0,
    Click,
    RightClick,
    LongPress,
    COUNT
};

struct QuickActionConfig
{
    bool enabled = true;
    OverlayTrigger trigger = OverlayTrigger::Hover;
    uint32_t fadeInMs = 100;
    uint32_t fadeOutMs = 200;
    uint32_t buttonSizePx = 24;
    float opacity = 0.9f;
    uint32_t maxVisibleActions = 4;
};

class QuickActionsOverlay
{
  public:
    static constexpr size_t ActionCount()
    {
        return static_cast<size_t>(QuickActionType::COUNT);
    }
    static constexpr size_t TriggerCount()
    {
        return static_cast<size_t>(OverlayTrigger::COUNT);
    }

    static const wchar_t* ActionName(QuickActionType a)
    {
        switch (a) {
            case QuickActionType::RotateLeft:
                return L"Rotate Left";
            case QuickActionType::RotateRight:
                return L"Rotate Right";
            case QuickActionType::Crop:
                return L"Crop";
            case QuickActionType::ConvertFormat:
                return L"Convert";
            case QuickActionType::ShareFile:
                return L"Share";
            case QuickActionType::OpenWith:
                return L"Open With";
            case QuickActionType::CopyToClipboard:
                return L"Copy";
            case QuickActionType::SetAsWallpaper:
                return L"Wallpaper";
            case QuickActionType::ShowProperties:
                return L"Properties";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* TriggerName(OverlayTrigger t)
    {
        switch (t) {
            case OverlayTrigger::Hover:
                return L"Hover";
            case OverlayTrigger::Click:
                return L"Click";
            case OverlayTrigger::RightClick:
                return L"Right-Click";
            case OverlayTrigger::LongPress:
                return L"Long Press";
            default:
                return L"Unknown";
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
