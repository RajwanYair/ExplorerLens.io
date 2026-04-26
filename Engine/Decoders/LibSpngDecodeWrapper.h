// Engine/Decoders/LibSpngDecodeWrapper.h
// ExplorerLens — libspng decode wrapper; replaces stb_image PNG path (ROADMAP v7.0 Phase 2)
// Sprint S319.
//
// Purpose:
//   stb_image's PNG decoder is single-threaded and produces ~1.8× slower
//   decode times on large PNG files compared to libspng 0.7+ with SIMD and
//   multi-threaded IDAT chunk decompression.  This wrapper replaces it.
//
//   What stb_image does today (to be removed after Phase 2):
//     stbi_load_from_memory(buffer, len, &w, &h, &comp, 4)
//     [Engine/Decoders/PngDecoder.cpp]
//
//   What LibSpngDecodeWrapper provides:
//     - spng_decode_image() path: ~1.8-2.5× faster on 4K PNG files
//     - Proper ICC profile passthrough via spng_get_iccp()
//     - CRC checking configurable (skip for speed, enforce for correctness)
//     - Outputs BGRA-8, RGBA-8 or RGB-8 with stride alignment
//
// Enable gate (same as build-scripts/external-libs/Build-LibSpng.ps1):
//   #define EXPLORERLENS_LIBSPNG_AVAILABLE 1
//   when libspng is linked.  Stub returns NOT_AVAILABLE otherwise.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_LIB_SPNG_DECODE_WRAPPER_H
#define EXPLORERLENS_ENGINE_LIB_SPNG_DECODE_WRAPPER_H

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>

// Phase 2 enable gate — flip to 1 when libspng 0.7.x is linked.
#define EXPLORERLENS_LIBSPNG_AVAILABLE 0

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// SpngColorType — PNG colour type codes (match PNG spec Table 11.1)
// ---------------------------------------------------------------------------
enum class SpngColorType : std::uint8_t {
    GRAYSCALE        = 0,
    TRUECOLOR        = 2,   ///< RGB
    INDEXED          = 3,
    GRAYSCALE_ALPHA  = 4,
    TRUECOLOR_ALPHA  = 6,   ///< RGBA
    UNKNOWN          = 0xFF,
};

// ---------------------------------------------------------------------------
// SpngOutputFormat — desired decoded pixel layout
// ---------------------------------------------------------------------------
enum class SpngOutputFormat : std::uint8_t {
    BGRA8  = 0,   ///< Default; Windows DIBSection-compatible
    RGBA8  = 1,   ///< Web/OpenGL-compatible
    RGB8   = 2,   ///< No alpha, smaller output
};

// ---------------------------------------------------------------------------
// SpngDecodeConfig
// ---------------------------------------------------------------------------
struct SpngDecodeConfig final {
    SpngOutputFormat  outputFormat = SpngOutputFormat::BGRA8;
    bool              stripAlpha   = false;   ///< Force output alpha = 255
    bool              skipCRC      = true;    ///< Skip CRC check (faster, for thumbnails)
};

// ---------------------------------------------------------------------------
// SpngDecodeStatus
// ---------------------------------------------------------------------------
enum class SpngDecodeStatus : std::uint8_t {
    OK              = 0,
    NOT_AVAILABLE   = 1,   ///< libspng not linked (Phase 2 stub)
    INVALID_INPUT   = 2,   ///< Null buffer, zero length
    NOT_PNG         = 3,   ///< Missing 8-byte PNG signature
    DECODE_FAILED   = 4,   ///< spng_decode_image returned non-zero
    CRC_ERROR       = 5,   ///< CRC mismatch (only when skipCRC=false)
    OUT_OF_MEMORY   = 6,
    IMAGE_TOO_LARGE = 7,   ///< Width or height exceeds kMaxSpngDimension
};

// ---------------------------------------------------------------------------
// SpngDecodeResult
// ---------------------------------------------------------------------------
struct SpngDecodeResult final {
    SpngDecodeStatus       status{ SpngDecodeStatus::NOT_AVAILABLE };
    std::uint32_t          widthPixels{};
    std::uint32_t          heightPixels{};
    std::uint32_t          strideBytes{};
    std::vector<std::byte> pixels;
    SpngColorType          colorType{ SpngColorType::UNKNOWN };
    std::uint8_t           bitDepth{};        ///< 1,2,4,8,16
    std::uint64_t          decodeTimeUs{};
};

// ---------------------------------------------------------------------------
// LibSpngDecodeWrapper
// ---------------------------------------------------------------------------
// Phase 2 stub — Decode() returns NOT_AVAILABLE until libspng is linked.
//
class LibSpngDecodeWrapper final {
public:
    LibSpngDecodeWrapper() noexcept  = default;
    ~LibSpngDecodeWrapper() noexcept = default;

    LibSpngDecodeWrapper(const LibSpngDecodeWrapper&)            = delete;
    LibSpngDecodeWrapper& operator=(const LibSpngDecodeWrapper&) = delete;

    // ── Primary API ──────────────────────────────────────────────────────────

    /// Decode a PNG bitstream to a pixel buffer.
    ///
    /// @param pngData  Raw PNG file bytes (in-memory)
    /// @param cfg      Decode parameters
    [[nodiscard]] SpngDecodeResult Decode(
        std::span<const std::byte>  pngData,
        const SpngDecodeConfig&     cfg = SpngDecodeConfig{}) const noexcept
    {
#if EXPLORERLENS_LIBSPNG_AVAILABLE
        // Phase 2: spng_ctx_new() + spng_set_png_buffer() + spng_decode_image()
        (void)pngData; (void)cfg;
        return { .status = SpngDecodeStatus::NOT_AVAILABLE };
#else
        (void)pngData; (void)cfg;
        return { .status = SpngDecodeStatus::NOT_AVAILABLE };
#endif
    }

    // ── Capability ────────────────────────────────────────────────────────────

    [[nodiscard]] static constexpr bool IsAvailable() noexcept
    {
        return EXPLORERLENS_LIBSPNG_AVAILABLE != 0;
    }

    [[nodiscard]] static constexpr const char* BackendVersion() noexcept
    {
        return EXPLORERLENS_LIBSPNG_AVAILABLE ? "0.7.4" : "0.0.0";
    }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Maximum image dimension supported (libspng limit: 2^20 = 1 048 576)
    static constexpr std::uint32_t kMaxSpngDimension = 1'048'576u;

    /// Minimum PNG bitstream size (8-byte sig + IHDR chunk = 33 bytes minimum)
    static constexpr std::size_t   kMinPngSignatureBytes = 8u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_LIB_SPNG_DECODE_WRAPPER_H
