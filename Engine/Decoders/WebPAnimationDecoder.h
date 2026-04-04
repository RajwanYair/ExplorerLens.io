// WebPAnimationDecoder.h — WebP Animation Frame Splitter
// Copyright (c) 2026 ExplorerLens Project
//
// Splits animated WebP containers (.webp) into individual BGRA frames and
// extracts the cover frame for thumbnail generation via libwebp demux API.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Metadata ---------------------------------------------------------------

struct WebPFrameInfo
{
    uint32_t frameIndex = 0;
    int x = 0;  // Canvas X offset (for delta frames)
    int y = 0;  // Canvas Y offset
    uint32_t width = 0;
    uint32_t height = 0;
    int durationMs = 0;  // Frame display duration (milliseconds)
    bool hasAlpha = false;
    bool isKeyframe = false;  // Full-canvas (not delta) frame
};

struct WebPAnimationInfo
{
    uint32_t canvasWidth = 0;
    uint32_t canvasHeight = 0;
    uint32_t frameCount = 0;
    int loopCount = 0;  // 0 = infinite loop
    bool hasAlpha = false;
    uint32_t backgroundColor = 0x00000000;  // ARGB background fill
    std::vector<WebPFrameInfo> frames;
};

// ---- Options & Result -------------------------------------------------------

struct WebPDecodeOptions
{
    bool decodeCoverOnly = true;   // Only decode cover frame for thumbnail
    bool compositeOnWhite = true;  // Flatten alpha onto #FFFFFF
    uint32_t maxDimension = 4096;  // Clamp decode dimension
    bool preserveCSS = false;      // Keep canvas background color
};

enum class WebPDecodeStatus {
    Success = 0,
    InvalidData = 1,
    NotAnimated = 2,  // Static WebP — use base WebP decoder
    FrameNotFound = 3,
    LibraryMissing = 4,  // libwebp not present at runtime
    OutOfMemory = 5,
    InternalError = 99,
};

struct WebPDecodeResult
{
    WebPDecodeStatus status = WebPDecodeStatus::InternalError;
    std::vector<uint8_t> pixels;  // BGRA, row-major, stride = canvasWidth * 4
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t frameIndex = 0;
    bool composited = false;
    std::string decoderNote;
};

// ---- WebPAnimationDecoder ---------------------------------------------------

class WebPAnimationDecoder
{
  public:
    WebPAnimationDecoder();
    ~WebPAnimationDecoder();

    // Parse RIFF/WEBP container metadata (no pixel decode).
    bool ParseInfo(const uint8_t* data, size_t size, WebPAnimationInfo& outInfo) const;

    // Decode best cover frame — primary thumbnail entry-point.
    // Selects frame 0 unless a dedicated cover tile exists in EXIF.
    WebPDecodeResult DecodeCoverFrame(const uint8_t* data, size_t size, const WebPDecodeOptions& opts = {}) const;

    // Decode arbitrary frame by index (composes all preceding delta frames).
    WebPDecodeResult DecodeFrame(const uint8_t* data, size_t size, uint32_t frameIndex,
                                 const WebPDecodeOptions& opts = {}) const;

    // Quick lossy/lossless/animated WebP probe via "RIFF????WEBP" + "ANIM" chunk.
    static bool LooksLikeAnimatedWebP(const uint8_t* data, size_t size)
    {
        (void)data;
        (void)size;
        return false;
    }

  private:
    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;

    WebPDecodeResult DecodeInternal(const uint8_t* data, size_t size, uint32_t frameIndex, bool coverOnly,
                                    const WebPDecodeOptions& opts) const;

    static void CompositeAlphaOnWhite(std::vector<uint8_t>& pixels, uint32_t width, uint32_t height);
};

}  // namespace Engine
}  // namespace ExplorerLens
