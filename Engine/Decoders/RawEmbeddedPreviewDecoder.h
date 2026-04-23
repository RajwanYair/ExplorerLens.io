//==============================================================================
// ExplorerLens Engine — RAW Embedded Preview Decoder
// Sprint S223
// Copyright (c) 2026 — ExplorerLens Project
//
// PURPOSE:
// Pipeline-compatible decoder that uses LibRaw::unpack_thumb() to extract the
// camera-embedded JPEG thumbnail from RAW files (CR2, NEF, ARW, DNG, RAF, …)
// without decoding full sensor data.  This provides a fast-path that avoids
// the expensive full-RAW decode path for thumbnail generation.
//
// PERFORMANCE vs. FULL DECODE:
//   Full LibRaw decode (16-bit demosaicing): 800–3 000 ms
//   unpack_thumb() embedded JPEG path:          5–  30 ms
//   Speed-up: typically 50–200×
//
// ACTIVATION: Requires HAS_LIBRAW defined at compile-time.
//
// INTEGRATION POINT:
// The ThumbnailPipeline should probe RawEmbeddedPreviewDecoder::CanHandle()
// before falling through to the full LibRaw decoder, e.g.:
//
//   if (RawEmbeddedPreviewDecoder::CanHandle(ext)) {
//       RawEmbeddedPreviewDecoder dec;
//       auto result = dec.DecodeThumb(filePath, maxWidth, maxHeight);
//       if (result.ok) return BitmapFromJpegBytes(result.jpegBytes, ...);
//   }
//   // Fallback: full LibRaw decode
//==============================================================================

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "Core/EmbeddedPreviewExtractor.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// RawEmbeddedPreviewResult — output from RawEmbeddedPreviewDecoder
//==============================================================================
struct RawEmbeddedPreviewResult
{
    std::vector<uint8_t> jpegBytes;   ///< Raw JPEG bytes — decode with JpegTurboDecoder
    uint32_t             width    = 0;
    uint32_t             height   = 0;
    bool                 ok       = false;
    bool                 fromEmbedded = true;  ///< false → fell back to full decode path
    std::string          error;         ///< Non-empty on failure
};

//==============================================================================
// RawEmbeddedPreviewDecoder
//
// Wraps EmbeddedPreviewExtractor in a decoder interface compatible with the
// ThumbnailPipeline. Handles 28+ RAW format extensions.
//
// THREAD SAFETY: Not thread-safe. Instantiate per-thread or protect externally.
//==============================================================================
class RawEmbeddedPreviewDecoder
{
public:
    RawEmbeddedPreviewDecoder()  = default;
    ~RawEmbeddedPreviewDecoder() = default;

    RawEmbeddedPreviewDecoder(const RawEmbeddedPreviewDecoder&)            = delete;
    RawEmbeddedPreviewDecoder& operator=(const RawEmbeddedPreviewDecoder&) = delete;
    RawEmbeddedPreviewDecoder(RawEmbeddedPreviewDecoder&&)                  = default;
    RawEmbeddedPreviewDecoder& operator=(RawEmbeddedPreviewDecoder&&)       = default;

    //--------------------------------------------------------------------------
    // CanHandle — returns true if the extension maps to a RAW format that
    // typically carries an embedded JPEG thumbnail.
    //
    // Supported: .cr2 .cr3 .nef .nrw .arw .srf .sr2 .dng .orf .pef .ptx
    //            .raf .rw2 .rwl .mrw .mef .3fr .fff .iiq .cap .eip .dcs
    //            .dcr .drf .k25 .kdc .x3f .raw
    //--------------------------------------------------------------------------
    static bool CanHandle(std::wstring_view extension) noexcept;

    //--------------------------------------------------------------------------
    // DecodeThumb — fast-path: extract the embedded JPEG and return raw bytes.
    //
    // The caller is responsible for decoding the returned JPEG bytes (e.g. via
    // JpegTurboDecoder::DecodeThumb).  Returning bytes rather than pixels
    // allows the caller to apply DCT-scale at decode time for further speedup.
    //
    // Parameters:
    //   filePath  — full path to the RAW file
    //   maxWidth  — maximum thumbnail width (used only for EmbeddedPreviewExtractor
    //               minimum-resolution filtering; actual size comes from the
    //               embedded JPEG itself)
    //   maxHeight — maximum thumbnail height
    //
    // Returns: RawEmbeddedPreviewResult (.ok == false on any failure)
    //--------------------------------------------------------------------------
#ifdef HAS_LIBRAW
    RawEmbeddedPreviewResult DecodeThumb(
        std::wstring_view filePath,
        uint32_t          maxWidth  = 256,
        uint32_t          maxHeight = 256) noexcept;

    //--------------------------------------------------------------------------
    // DecodeThumbFromBuffer — same as DecodeThumb but from an in-memory buffer.
    //--------------------------------------------------------------------------
    RawEmbeddedPreviewResult DecodeThumbFromBuffer(
        const uint8_t* data,
        size_t         size,
        uint32_t       maxWidth  = 256,
        uint32_t       maxHeight = 256) noexcept;
#endif // HAS_LIBRAW

    //--------------------------------------------------------------------------
    // IsAvailable — runtime check: true iff linked with LibRaw
    //--------------------------------------------------------------------------
    static bool IsAvailable() noexcept
    {
#ifdef HAS_LIBRAW
        return true;
#else
        return false;
#endif
    }

private:
    EmbeddedPreviewExtractor m_extractor;

    // Convert EmbeddedPreviewResult → RawEmbeddedPreviewResult
    static RawEmbeddedPreviewResult FromExtractorResult(
        const EmbeddedPreviewResult& src) noexcept;
};

} // namespace Engine
} // namespace ExplorerLens
