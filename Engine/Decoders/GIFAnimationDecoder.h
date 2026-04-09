// GIFAnimationDecoder.h — GIF Animation Frame Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes GIF87a/GIF89a animation frames from raw byte data, applying
// LZW decompression, frame disposal, and palette-to-BGRA32 conversion.
// Supports transparent index handling and interlaced frames. Returns up
// to `maxFrames` key frames for hover-scrub thumbnail display.
//
#pragma once
#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct GIFFrame {
    std::vector<uint8_t> pixelsBGRA;  // width × height × 4 bytes, BGRA32
    uint32_t width     = 0;
    uint32_t height    = 0;
    uint32_t delayMs   = 100;         // Frame delay in milliseconds
    bool     hasAlpha  = false;
};

struct GIFDecodeResult {
    std::vector<GIFFrame> frames;
    uint32_t canvasWidth  = 0;
    uint32_t canvasHeight = 0;
    uint32_t totalFrames  = 0;        // Total frames in GIF (may be > frames decoded)
    bool     success      = false;
    bool     isAnimated   = false;
};

struct GIFDecodeOptions {
    uint32_t maxFrames   = 5;    // Maximum frames to decode (0 = all)
    bool     bgra        = true; // Output BGRA32; false = RGBA32
    bool     applyDispose = true; // Apply GIF disposal method between frames
};

class GIFAnimationDecoder {
public:
    GIFAnimationDecoder()  = default;
    ~GIFAnimationDecoder() = default;

    // Decode GIF animation from raw bytes.
    GIFDecodeResult Decode(
        const uint8_t*          gifData,
        size_t                  gifSize,
        const GIFDecodeOptions& opts = {}) const noexcept;

    // Quick probe: returns total frame count without full decode.
    static uint32_t ProbeFrameCount(
        const uint8_t* gifData, size_t gifSize) noexcept;

    // Check GIF magic bytes (GIF87a / GIF89a).
    static bool IsGIF(const uint8_t* data, size_t size) noexcept;
};

}} // namespace ExplorerLens::Engine
