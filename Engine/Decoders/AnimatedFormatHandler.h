//==============================================================================
// ExplorerLens Engine — APNG & Animated Format Enhancement
// Validates APNG via WIC, improves animated WebP/JXL first-frame extraction.
// Core types defined inline after consolidation (originally from AnimatedThumbnailEngine.h).
//==============================================================================
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class AnimatedFormat : uint8_t {
    GIF = 0,
    APNG,
    WebPAnim,
    JXLAnim,
    AVIFSeq,
    FLIF,
    FormatCount
};

enum class FrameStrategy : uint8_t {
    First = 0,
    Keyframe,
    Middle,
    MostDetail,
    Composite,
    StrategyCount
};

struct AnimationInfo
{
    AnimatedFormat format = AnimatedFormat::GIF;
    uint32_t frameCount = 0;
    uint32_t durationMs = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool isLooping = false;
};

/// Animated format handler — extends the core AnimatedThumbnailEngine types
class AnimatedFormatHandler
{
  public:
    /// Detect animated format from extension
    static AnimatedFormat DetectFormat(const std::wstring& ext)
    {
        if (ext == L".apng")
            return AnimatedFormat::APNG;
        if (ext == L".webp")
            return AnimatedFormat::WebPAnim;
        if (ext == L".jxl")
            return AnimatedFormat::JXLAnim;
        if (ext == L".gif")
            return AnimatedFormat::GIF;
        if (ext == L".avif")
            return AnimatedFormat::AVIFSeq;
        return AnimatedFormat::GIF;  // default fallback
    }

    /// Format name
    static const wchar_t* FormatName(AnimatedFormat f)
    {
        switch (f) {
            case AnimatedFormat::APNG:
                return L"Animated PNG";
            case AnimatedFormat::WebPAnim:
                return L"Animated WebP";
            case AnimatedFormat::JXLAnim:
                return L"Animated JPEG XL";
            case AnimatedFormat::GIF:
                return L"Animated GIF";
            case AnimatedFormat::AVIFSeq:
                return L"Animated AVIF";
            case AnimatedFormat::FLIF:
                return L"Animated FLIF";
            default:
                return L"Unknown";
        }
    }

    /// Strategy name
    static const wchar_t* StrategyName(FrameStrategy s)
    {
        switch (s) {
            case FrameStrategy::First:
                return L"FirstFrame";
            case FrameStrategy::Keyframe:
                return L"KeyFrame";
            case FrameStrategy::Middle:
                return L"MiddleFrame";
            case FrameStrategy::MostDetail:
                return L"MostDetail";
            case FrameStrategy::Composite:
                return L"Composite";
            default:
                return L"Unknown";
        }
    }

    /// Select best frame index for thumbnail
    static uint32_t SelectFrame(const AnimationInfo& info, FrameStrategy strategy)
    {
        if (info.frameCount == 0)
            return 0;
        switch (strategy) {
            case FrameStrategy::First:
                return 0;
            case FrameStrategy::Middle:
                return info.frameCount / 2;
            case FrameStrategy::Keyframe:
                return 0;  // First frame is typically the keyframe in all formats
            case FrameStrategy::MostDetail:
                return 0;  // Full decode analysis required; defaults to first frame
            default:
                return 0;
        }
    }

    /// Count animation frames by parsing format-specific binary markers.
    /// - GIF: counts Graphic Control Extension blocks (0x21 0xF9).
    /// - APNG: counts fcTL (frame control) PNG chunks.
    /// - WebP: counts ANMF sub-chunks in the RIFF container.
    static uint32_t CountFrames(const uint8_t* data, size_t size, AnimatedFormat format)
    {
        if (!data || size < 12)
            return 0;
        uint32_t count = 0;
        switch (format) {
            case AnimatedFormat::GIF:
                for (size_t i = 0; i + 1 < size; ++i) {
                    if (data[i] == 0x21 && data[i + 1] == 0xF9)
                        ++count;
                }
                break;
            case AnimatedFormat::APNG: {
                size_t pos = 8;  // skip PNG 8-byte signature
                while (pos + 8 <= size) {
                    uint32_t len = (static_cast<uint32_t>(data[pos]) << 24)
                                   | (static_cast<uint32_t>(data[pos + 1]) << 16)
                                   | (static_cast<uint32_t>(data[pos + 2]) << 8) | static_cast<uint32_t>(data[pos + 3]);
                    if (data[pos + 4] == 'f' && data[pos + 5] == 'c' && data[pos + 6] == 'T' && data[pos + 7] == 'L')
                        ++count;
                    size_t advance = 12 + static_cast<size_t>(len);
                    if (advance < 12)
                        break;  // overflow guard
                    pos += advance;
                }
                break;
            }
            case AnimatedFormat::WebPAnim: {
                size_t pos = 12;  // skip RIFF header
                while (pos + 8 <= size) {
                    if (data[pos] == 'A' && data[pos + 1] == 'N' && data[pos + 2] == 'M' && data[pos + 3] == 'F')
                        ++count;
                    uint32_t cs = static_cast<uint32_t>(data[pos + 4]) | (static_cast<uint32_t>(data[pos + 5]) << 8)
                                  | (static_cast<uint32_t>(data[pos + 6]) << 16)
                                  | (static_cast<uint32_t>(data[pos + 7]) << 24);
                    size_t advance = 8 + static_cast<size_t>(cs);
                    if (cs & 1)
                        ++advance;  // RIFF word alignment
                    if (advance < 8)
                        break;  // overflow guard
                    pos += advance;
                }
                break;
            }
            default:
                break;
        }
        return count;
    }

    /// Check if APNG from magic bytes (PNG + acTL chunk)
    static bool IsAPNG(const uint8_t* data, size_t size)
    {
        if (size < 8)
            return false;
        // PNG magic
        if (data[0] != 0x89 || data[1] != 'P' || data[2] != 'N' || data[3] != 'G')
            return false;
        // Search for acTL chunk (animated control)
        for (size_t i = 8; i + 8 < size && i < 4096; i++) {
            if (data[i] == 'a' && data[i + 1] == 'c' && data[i + 2] == 'T' && data[i + 3] == 'L')
                return true;
        }
        return false;
    }

    /// Count of supported animated formats
    static constexpr size_t FormatCount()
    {
        return static_cast<size_t>(AnimatedFormat::FormatCount);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
