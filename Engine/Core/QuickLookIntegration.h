//==============================================================================
// DarkThumbs Engine — Sprint 323: Quick Look Integration
// macOS Quick Look-inspired space-bar preview for Windows with full-screen
// slideshow, compare mode, and metadata overlay on Windows 11.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class QuickLookMode : uint8_t { Single=0,Compare,Slideshow,Fullscreen,COUNT };
enum class QuickLookTransition : uint8_t { None=0,Fade,Slide,Zoom,COUNT };
enum class QuickLookMetadataOverlay : uint8_t { Hidden=0,Minimal,Full,COUNT };

struct QuickLookConfig {
    QuickLookMode           mode        = QuickLookMode::Single;
    QuickLookTransition     transition  = QuickLookTransition::Fade;
    QuickLookMetadataOverlay overlay    = QuickLookMetadataOverlay::Minimal;
    uint32_t                slideshowIntervalSec = 3;
    bool                    loopSlideshow = true;
};

struct QuickLookSession {
    QuickLookMode   mode        = QuickLookMode::Single;
    uint32_t        fileCount   = 0;
    uint32_t        currentIndex= 0;
    bool            active      = false;
};

class QuickLookIntegration {
public:
    static const wchar_t* ModeName(QuickLookMode m) {
        switch(m) {
            case QuickLookMode::Single:     return L"Single";
            case QuickLookMode::Compare:    return L"Compare";
            case QuickLookMode::Slideshow:  return L"Slideshow";
            case QuickLookMode::Fullscreen: return L"Fullscreen";
            default: return L"Unknown";
        }
    }
    static const wchar_t* TransitionName(QuickLookTransition t) {
        switch(t) {
            case QuickLookTransition::None:  return L"None";
            case QuickLookTransition::Fade:  return L"Fade";
            case QuickLookTransition::Slide: return L"Slide";
            case QuickLookTransition::Zoom:  return L"Zoom";
            default: return L"Unknown";
        }
    }
    static const wchar_t* OverlayName(QuickLookMetadataOverlay o) {
        switch(o) {
            case QuickLookMetadataOverlay::Hidden:  return L"Hidden";
            case QuickLookMetadataOverlay::Minimal: return L"Minimal";
            case QuickLookMetadataOverlay::Full:    return L"Full";
            default: return L"Unknown";
        }
    }
    static constexpr size_t ModeCount()       { return static_cast<size_t>(QuickLookMode::COUNT); }
    static constexpr size_t TransitionCount() { return static_cast<size_t>(QuickLookTransition::COUNT); }
    static constexpr size_t OverlayCount()    { return static_cast<size_t>(QuickLookMetadataOverlay::COUNT); }
    static QuickLookConfig DefaultConfig()    { return QuickLookConfig{}; }
};

}} // namespace DarkThumbs::Engine
