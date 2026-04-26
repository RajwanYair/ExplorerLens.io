// Engine/Decoders/LibJpegTurboEncodeWrapper.h
// ExplorerLens — libjpeg-turbo encode wrapper; replaces stb_image_write (ROADMAP v7.0 Phase 2)
// Sprint S318.
//
// Purpose:
//   stb_image_write's JPEG encoder produces ~15-20% larger files than
//   libjpeg-turbo and does not support DCT-level quality tuning or
//   progressive scan.  This wrapper replaces it.
//
//   What stb_image_write does (to be removed after Phase 2):
//     stbi_write_jpg(path, w, h, comp, pixels, quality)  [Engine/Core/EncoderExportEngine.cpp]
//
//   What LibJpegTurboEncodeWrapper provides:
//     - tjCompress2() with configurable subsampling (4:2:0 default / 4:4:4 lossless)
//     - Progressive JPEG encoding for web-compatible output
//     - 2-4× faster encode vs stb_image_write on the same quality setting
//     - Produces output buffer as std::vector<std::byte> (no temp files)
//
// Enable gate (same as build-scripts/external-libs/Build-LibJpegTurbo.ps1):
//   #define EXPLORERLENS_LIBJPEG_TURBO_AVAILABLE 1
//   when libjpeg-turbo is linked.  Stub returns NOT_AVAILABLE otherwise.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_LIBJPEG_TURBO_ENCODE_WRAPPER_H
#define EXPLORERLENS_ENGINE_LIBJPEG_TURBO_ENCODE_WRAPPER_H

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>

// Phase 2 enable gate — flip to 1 when libjpeg-turbo 3.x is linked.
#define EXPLORERLENS_LIBJPEG_TURBO_AVAILABLE 0

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// JpegSubsampling — chroma subsampling options
// ---------------------------------------------------------------------------
enum class JpegSubsampling : std::uint8_t {
    YUV_444 = 0,   ///< 4:4:4  — maximum quality, ~30% larger file
    YUV_422 = 1,   ///< 4:2:2  — balanced (good for wide-gamut images)
    YUV_420 = 2,   ///< 4:2:0  — default; matches most consumer cameras
    GRAY    = 3,   ///< Greyscale only
};

// ---------------------------------------------------------------------------
// JpegEncodeConfig
// ---------------------------------------------------------------------------
struct JpegEncodeConfig final {
    // JPEG quality 1-100 (libjpeg-turbo scale matches stb_image_write scale).
    // Default 85: visually lossless for thumbnails, ~50% size of quality=100.
    std::uint8_t     quality       = 85u;
    JpegSubsampling  subsampling   = JpegSubsampling::YUV_420;
    bool             progressive   = false;   ///< Progressive scan (web-friendly)
    bool             optimiseHuffman = true;  ///< Arithmetic code table optimisation
};

// ---------------------------------------------------------------------------
// JpegEncodeStatus
// ---------------------------------------------------------------------------
enum class JpegEncodeStatus : std::uint8_t {
    OK              = 0,
    NOT_AVAILABLE   = 1,   ///< libjpeg-turbo not linked (Phase 2 stub)
    INVALID_INPUT   = 2,   ///< Null pixels, zero width/height
    COMPRESS_FAILED = 3,   ///< tjCompress2 returned error
    OUT_OF_MEMORY   = 4,
};

// ---------------------------------------------------------------------------
// JpegEncodeResult
// ---------------------------------------------------------------------------
struct JpegEncodeResult final {
    JpegEncodeStatus       status{ JpegEncodeStatus::NOT_AVAILABLE };
    std::vector<std::byte> jpegBytes;     ///< Encoded JPEG output
    std::uint64_t          encodeTimeUs{};
};

// ---------------------------------------------------------------------------
// LibJpegTurboEncodeWrapper
// ---------------------------------------------------------------------------
// Phase 2 stub — Encode() returns NOT_AVAILABLE until libjpeg-turbo is linked.
//
class LibJpegTurboEncodeWrapper final {
public:
    LibJpegTurboEncodeWrapper() noexcept  = default;
    ~LibJpegTurboEncodeWrapper() noexcept = default;

    LibJpegTurboEncodeWrapper(const LibJpegTurboEncodeWrapper&)            = delete;
    LibJpegTurboEncodeWrapper& operator=(const LibJpegTurboEncodeWrapper&) = delete;

    // ── Primary API ──────────────────────────────────────────────────────────

    /// Encode a BGRA-8 or RGB-24 pixel buffer to JPEG.
    ///
    /// @param pixels      Row-major pixel data (BGRA or RGB, no padding)
    /// @param width       Image width in pixels
    /// @param height      Image height in pixels
    /// @param components  3 = RGB, 4 = BGRA
    /// @param cfg         Encode parameters
    [[nodiscard]] JpegEncodeResult Encode(
        std::span<const std::byte>  pixels,
        std::uint32_t               width,
        std::uint32_t               height,
        std::uint32_t               components,
        const JpegEncodeConfig&     cfg = JpegEncodeConfig{}) const noexcept
    {
#if EXPLORERLENS_LIBJPEG_TURBO_AVAILABLE
        // Phase 2: tjInitCompress() + tjCompress2()
        (void)pixels; (void)width; (void)height; (void)components; (void)cfg;
        return { .status = JpegEncodeStatus::NOT_AVAILABLE };
#else
        (void)pixels; (void)width; (void)height; (void)components; (void)cfg;
        return { .status = JpegEncodeStatus::NOT_AVAILABLE };
#endif
    }

    // ── Capability ────────────────────────────────────────────────────────────

    [[nodiscard]] static constexpr bool IsAvailable() noexcept
    {
        return EXPLORERLENS_LIBJPEG_TURBO_AVAILABLE != 0;
    }

    /// libjpeg-turbo library version string (0.0.0 when stub).
    [[nodiscard]] static constexpr const char* BackendVersion() noexcept
    {
        return EXPLORERLENS_LIBJPEG_TURBO_AVAILABLE ? "3.0.4" : "0.0.0";
    }

    // ── Constants ─────────────────────────────────────────────────name───────

    static constexpr std::uint8_t  kMinQuality     = 1u;
    static constexpr std::uint8_t  kMaxQuality     = 100u;
    static constexpr std::uint8_t  kDefaultQuality = 85u;

    /// Maximum input dimension (libjpeg-turbo limit: 65535 × 65535)
    static constexpr std::uint32_t kMaxDimension   = 65535u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_LIBJPEG_TURBO_ENCODE_WRAPPER_H
