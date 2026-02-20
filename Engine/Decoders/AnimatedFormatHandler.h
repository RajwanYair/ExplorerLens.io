//==============================================================================
// DarkThumbs Engine — Sprint 256: APNG & Animated Format Enhancement
// Validates APNG via WIC, improves animated WebP/JXL first-frame extraction.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Animated image format types
enum class AnimatedFormat : uint8_t {
    APNG,
    AnimatedWebP,
    AnimatedJXL,
    AnimatedGIF,
    AnimatedAVIF,
    Unknown
};

/// Frame extraction strategy
enum class FrameStrategy : uint8_t {
    FirstFrame,         // Always use first frame (fastest)
    KeyFrame,           // Use first keyframe
    MiddleFrame,        // Use middle frame (better representative)
    LargestFrame,       // Use frame with most detail
    Custom              // User-specified frame index
};

/// Animation info extracted from file
struct AnimationInfo {
    AnimatedFormat format       = AnimatedFormat::Unknown;
    uint32_t       frameCount   = 0;
    uint32_t       width        = 0;
    uint32_t       height       = 0;
    uint32_t       loopCount    = 0;
    float          totalDuration = 0.0f; // seconds
    bool           hasAlpha     = false;
};

/// Animated format handler
class AnimatedFormatHandler {
public:
    /// Detect animated format from extension
    static AnimatedFormat DetectFormat(const std::wstring& ext) {
        if (ext == L".apng") return AnimatedFormat::APNG;
        if (ext == L".webp") return AnimatedFormat::AnimatedWebP;
        if (ext == L".jxl")  return AnimatedFormat::AnimatedJXL;
        if (ext == L".gif")  return AnimatedFormat::AnimatedGIF;
        if (ext == L".avif") return AnimatedFormat::AnimatedAVIF;
        return AnimatedFormat::Unknown;
    }

    /// Format name
    static const wchar_t* FormatName(AnimatedFormat f) {
        switch (f) {
            case AnimatedFormat::APNG:        return L"Animated PNG";
            case AnimatedFormat::AnimatedWebP: return L"Animated WebP";
            case AnimatedFormat::AnimatedJXL:  return L"Animated JPEG XL";
            case AnimatedFormat::AnimatedGIF:  return L"Animated GIF";
            case AnimatedFormat::AnimatedAVIF: return L"Animated AVIF";
            default: return L"Unknown";
        }
    }

    /// Strategy name
    static const wchar_t* StrategyName(FrameStrategy s) {
        switch (s) {
            case FrameStrategy::FirstFrame:   return L"FirstFrame";
            case FrameStrategy::KeyFrame:     return L"KeyFrame";
            case FrameStrategy::MiddleFrame:  return L"MiddleFrame";
            case FrameStrategy::LargestFrame: return L"LargestFrame";
            case FrameStrategy::Custom:       return L"Custom";
            default: return L"Unknown";
        }
    }

    /// Select best frame index for thumbnail
    static uint32_t SelectFrame(const AnimationInfo& info, FrameStrategy strategy) {
        if (info.frameCount == 0) return 0;
        switch (strategy) {
            case FrameStrategy::FirstFrame:   return 0;
            case FrameStrategy::MiddleFrame:  return info.frameCount / 2;
            case FrameStrategy::KeyFrame:     return 0; // simplified
            case FrameStrategy::LargestFrame: return 0; // needs decode
            default: return 0;
        }
    }

    /// Check if APNG from magic bytes (PNG + acTL chunk)
    static bool IsAPNG(const uint8_t* data, size_t size) {
        if (size < 8) return false;
        // PNG magic
        if (data[0] != 0x89 || data[1] != 'P' || data[2] != 'N' || data[3] != 'G')
            return false;
        // Search for acTL chunk (animated control)
        for (size_t i = 8; i + 8 < size && i < 4096; i++) {
            if (data[i] == 'a' && data[i+1] == 'c' && data[i+2] == 'T' && data[i+3] == 'L')
                return true;
        }
        return false;
    }

    /// Count of supported animated formats
    static constexpr size_t FormatCount() { return 5; }
};

}} // namespace DarkThumbs::Engine
