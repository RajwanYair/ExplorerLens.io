#pragma once
// PreviewTooltipRenderer.h — Preview Tooltip Renderer
// Rich tooltip preview on hover — shows larger thumbnail, metadata badges,
// format info, and file properties in a smooth animated popup.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Tooltip style
enum class TooltipStyle : uint8_t {
    Minimal = 0,  // Name + dimensions only
    Standard,     // + format info + file size
    Detailed,     // + metadata (EXIF, camera)
    Rich,         // + large preview thumbnail
    Custom,       // User-configured layout
    COUNT
};

/// Tooltip animation
enum class TooltipAnimation : uint8_t {
    None = 0,
    FadeIn,
    SlideUp,
    ScaleIn,
    COUNT
};

struct TooltipConfig
{
    TooltipStyle style = TooltipStyle::Standard;
    TooltipAnimation anim = TooltipAnimation::FadeIn;
    uint32_t maxWidthPx = 400;
    uint32_t maxHeightPx = 300;
    uint32_t delayMs = 500;
    uint32_t fadeInMs = 150;
    float cornerRadius = 8.0f;
    bool showHistogram = false;
    bool showColorPalette = false;
};

class PreviewTooltipRenderer
{
  public:
    static constexpr size_t StyleCount()
    {
        return static_cast<size_t>(TooltipStyle::COUNT);
    }
    static constexpr size_t AnimationCount()
    {
        return static_cast<size_t>(TooltipAnimation::COUNT);
    }

    static const wchar_t* StyleName(TooltipStyle s)
    {
        switch (s) {
            case TooltipStyle::Minimal:
                return L"Minimal";
            case TooltipStyle::Standard:
                return L"Standard";
            case TooltipStyle::Detailed:
                return L"Detailed";
            case TooltipStyle::Rich:
                return L"Rich";
            case TooltipStyle::Custom:
                return L"Custom";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* AnimationName(TooltipAnimation a)
    {
        switch (a) {
            case TooltipAnimation::None:
                return L"None";
            case TooltipAnimation::FadeIn:
                return L"Fade In";
            case TooltipAnimation::SlideUp:
                return L"Slide Up";
            case TooltipAnimation::ScaleIn:
                return L"Scale In";
            default:
                return L"Unknown";
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
