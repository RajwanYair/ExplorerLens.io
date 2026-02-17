#pragma once
//==============================================================================
// DarkThumbs — Sprint 38: Animated & Multi-Frame Thumbnail Support
// First-frame extraction for animated WebP/JXL, multi-page PDF composite,
// multi-page TIFF, Apple Live Photo key frame, frame count badges.
//==============================================================================

#ifndef DARKTHUMBS_ANIMATED_THUMBNAIL_DECODER_H
#define DARKTHUMBS_ANIMATED_THUMBNAIL_DECODER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <array>

namespace DarkThumbs { namespace Engine { namespace Decoders {

//==============================================================================
// Animated Format Detection
//==============================================================================

enum class AnimationFormat
{
    None,           // Static image
    AnimatedWebP,   // Animated WebP
    AnimatedJXL,    // Animated JPEG XL
    AnimatedGIF,    // Animated GIF
    MultiPagePDF,   // Multi-page PDF document
    MultiPageTIFF,  // Multi-page TIFF
    LivePhoto,      // Apple Live Photo (HEIC + MOV pair)
    AnimatedPNG     // APNG (Animated PNG)
};

inline const char* AnimationFormatName(AnimationFormat f)
{
    switch (f) {
        case AnimationFormat::None:          return "Static";
        case AnimationFormat::AnimatedWebP:  return "Animated WebP";
        case AnimationFormat::AnimatedJXL:   return "Animated JXL";
        case AnimationFormat::AnimatedGIF:   return "Animated GIF";
        case AnimationFormat::MultiPagePDF:  return "Multi-Page PDF";
        case AnimationFormat::MultiPageTIFF: return "Multi-Page TIFF";
        case AnimationFormat::LivePhoto:     return "Live Photo";
        case AnimationFormat::AnimatedPNG:   return "Animated PNG";
    }
    return "Unknown";
}

inline bool IsAnimated(AnimationFormat f)
{
    return f != AnimationFormat::None;
}

inline bool IsMultiPage(AnimationFormat f)
{
    return f == AnimationFormat::MultiPagePDF ||
           f == AnimationFormat::MultiPageTIFF;
}

//==============================================================================
// Frame Info — metadata for a single frame
//==============================================================================

struct FrameInfo
{
    uint32_t index    = 0;
    uint32_t width    = 0;
    uint32_t height   = 0;
    uint32_t durationMs = 0;   // Frame display duration (animated formats)
    bool     isKeyFrame = false;

    double AspectRatio() const
    {
        if (height == 0) return 1.0;
        return static_cast<double>(width) / height;
    }
};

struct AnimationInfo
{
    AnimationFormat format = AnimationFormat::None;
    uint32_t frameCount    = 0;
    uint32_t loopCount     = 0;  // 0 = infinite loop
    uint32_t totalDurationMs = 0;
    uint32_t canvasWidth   = 0;
    uint32_t canvasHeight  = 0;
    std::vector<FrameInfo> frames;

    bool IsAnimated() const { return format != AnimationFormat::None && frameCount > 1; }

    double TotalDurationSec() const
    {
        return totalDurationMs / 1000.0;
    }

    double AverageFrameDurationMs() const
    {
        if (frameCount == 0) return 0.0;
        return static_cast<double>(totalDurationMs) / frameCount;
    }

    double FPS() const
    {
        double avg = AverageFrameDurationMs();
        if (avg <= 0.0) return 0.0;
        return 1000.0 / avg;
    }
};

//==============================================================================
// Thumbnail Composition Strategies
//==============================================================================

enum class CompositionStrategy
{
    FirstFrame,         // Extract first frame only (default for animated)
    KeyFrame,           // Extract first key frame
    StackedPreview,     // Fanned/stacked N-page composite
    GridComposite,      // 2x2 grid of first 4 frames
    CoverWithBadge      // First frame + frame count badge overlay
};

inline const char* CompositionStrategyName(CompositionStrategy s)
{
    switch (s) {
        case CompositionStrategy::FirstFrame:      return "First Frame";
        case CompositionStrategy::KeyFrame:         return "Key Frame";
        case CompositionStrategy::StackedPreview:   return "Stacked Preview";
        case CompositionStrategy::GridComposite:    return "Grid Composite";
        case CompositionStrategy::CoverWithBadge:   return "Cover + Badge";
    }
    return "Unknown";
}

// Select optimal strategy for a given format
inline CompositionStrategy RecommendedStrategy(AnimationFormat format)
{
    switch (format) {
        case AnimationFormat::AnimatedWebP:
        case AnimationFormat::AnimatedJXL:
        case AnimationFormat::AnimatedGIF:
        case AnimationFormat::AnimatedPNG:
            return CompositionStrategy::CoverWithBadge;
        case AnimationFormat::MultiPagePDF:
            return CompositionStrategy::StackedPreview;
        case AnimationFormat::MultiPageTIFF:
            return CompositionStrategy::CoverWithBadge;
        case AnimationFormat::LivePhoto:
            return CompositionStrategy::KeyFrame;
        default:
            return CompositionStrategy::FirstFrame;
    }
}

//==============================================================================
// Frame Count Badge — overlay for animated/multi-page thumbnails
//==============================================================================

struct BadgeConfig
{
    uint32_t cornerRadius = 4;
    uint32_t paddingX     = 6;
    uint32_t paddingY     = 3;
    uint32_t fontSize     = 11;
    uint32_t bgColor      = 0xCC000000;  // Semi-transparent black (ARGB)
    uint32_t textColor    = 0xFFFFFFFF;  // White (ARGB)
    bool     showAtBottomRight = true;

    static BadgeConfig Default() { return BadgeConfig{}; }
};

struct FrameCountBadge
{
    uint32_t  frameCount = 0;
    BadgeConfig config;

    std::string BadgeText() const
    {
        if (frameCount == 0) return "";
        if (frameCount == 1) return "1 page";
        return std::to_string(frameCount) + " frames";
    }

    std::string PageBadgeText() const
    {
        if (frameCount == 0) return "";
        if (frameCount == 1) return "1 page";
        return std::to_string(frameCount) + " pages";
    }

    bool ShouldShow() const { return frameCount > 1; }
};

//==============================================================================
// Stacked Preview Layout — PDF fanned-page composite
//==============================================================================

struct StackedPageRect
{
    float x, y, width, height;
    float rotation;  // Degrees, for fanned effect
    uint32_t pageIndex;
};

class StackedPreviewLayout
{
public:
    explicit StackedPreviewLayout(uint32_t canvasSize = 256)
        : canvasSize_(canvasSize) {}

    // Generate layout for N stacked pages
    std::vector<StackedPageRect> GenerateLayout(uint32_t pageCount, uint32_t maxPages = 3)
    {
        std::vector<StackedPageRect> rects;
        uint32_t pagesToShow = std::min(pageCount, maxPages);

        float baseSize = canvasSize_ * 0.7f;
        float offset = canvasSize_ * 0.04f;
        float rotStep = 3.0f;

        for (uint32_t i = 0; i < pagesToShow; ++i) {
            StackedPageRect r;
            r.pageIndex = pagesToShow - 1 - i;  // Back pages first
            r.width = baseSize;
            r.height = baseSize * 1.414f;  // A4 aspect ratio
            if (r.height > canvasSize_ * 0.9f) r.height = canvasSize_ * 0.9f;
            r.x = (canvasSize_ - r.width) / 2.0f + offset * i;
            r.y = (canvasSize_ - r.height) / 2.0f - offset * i;
            r.rotation = rotStep * (static_cast<float>(pagesToShow - 1 - i) - 1.0f);
            rects.push_back(r);
        }

        return rects;
    }

    uint32_t CanvasSize() const { return canvasSize_; }

private:
    uint32_t canvasSize_;
};

//==============================================================================
// Live Photo Detector — identify HEIC+MOV pairs
//==============================================================================

struct LivePhotoComponents
{
    std::string photoPath;    // .HEIC file
    std::string videoPath;    // .MOV file
    bool        isComplete = false;

    bool IsValid() const
    {
        return !photoPath.empty() && !videoPath.empty() && isComplete;
    }
};

class LivePhotoDetector
{
public:
    // Check if a file might be part of a Live Photo pair
    static bool IsLivePhotoCandidate(const std::string& ext)
    {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == ".heic" || lower == ".heif" || lower == ".mov";
    }

    // Try to find the pair for a given file
    LivePhotoComponents DetectPair(const std::string& filePath)
    {
        LivePhotoComponents lp;
        std::string lower = filePath;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.size() < 5) return lp;

        std::string base = filePath.substr(0, filePath.size() - 4);
        std::string ext = filePath.substr(filePath.size() - 4);
        std::string extLower = ext;
        std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);

        if (extLower == ".hei" || filePath.size() > 5) {
            // Check for .heic → .mov pair
            if (lower.find(".heic") != std::string::npos || lower.find(".heif") != std::string::npos) {
                std::string baseName = filePath.substr(0, filePath.find_last_of('.'));
                lp.photoPath = filePath;
                lp.videoPath = baseName + ".MOV";
                lp.isComplete = true;
            }
        }

        return lp;
    }
};

//==============================================================================
// Animated Decoder Config — per-format settings
//==============================================================================

struct AnimatedDecoderConfig
{
    uint32_t maxFramesToDecode = 4;
    uint32_t maxDecodeTimeMs   = 2000;
    bool     extractFirstFrameOnly = false;
    bool     showFrameCountBadge   = true;
    bool     enableStackedPDFPreview = true;
    bool     enableLivePhotoDetection = true;
    CompositionStrategy defaultStrategy = CompositionStrategy::CoverWithBadge;

    static AnimatedDecoderConfig Default() { return AnimatedDecoderConfig{}; }

    static AnimatedDecoderConfig Performance()
    {
        AnimatedDecoderConfig c;
        c.extractFirstFrameOnly = true;
        c.showFrameCountBadge = true;
        c.enableStackedPDFPreview = false;
        c.maxFramesToDecode = 1;
        return c;
    }
};

//==============================================================================
// Format Detection — determine animation type from file header/extension
//==============================================================================

class AnimationFormatDetector
{
public:
    // Simple extension-based detection
    static AnimationFormat DetectFromExtension(const std::string& ext)
    {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == ".webp") return AnimationFormat::AnimatedWebP;
        if (lower == ".jxl")  return AnimationFormat::AnimatedJXL;
        if (lower == ".gif")  return AnimationFormat::AnimatedGIF;
        if (lower == ".apng") return AnimationFormat::AnimatedPNG;
        if (lower == ".pdf")  return AnimationFormat::MultiPagePDF;
        if (lower == ".tif" || lower == ".tiff") return AnimationFormat::MultiPageTIFF;
        if (lower == ".heic") return AnimationFormat::LivePhoto;

        return AnimationFormat::None;
    }

    // All formats that can potentially have animation/multi-page
    static std::vector<std::string> AnimatableExtensions()
    {
        return {".webp", ".jxl", ".gif", ".apng", ".pdf", ".tif", ".tiff", ".heic"};
    }

    static size_t AnimatableFormatCount()
    {
        return AnimatableExtensions().size();
    }
};

}}} // namespace DarkThumbs::Engine::Decoders

#endif // DARKTHUMBS_ANIMATED_THUMBNAIL_DECODER_H
