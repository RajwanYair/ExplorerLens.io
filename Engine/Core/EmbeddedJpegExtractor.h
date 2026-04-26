// Engine/Core/EmbeddedJpegExtractor.h
// ExplorerLens — EXIF embedded-JPEG fast-path extractor (H13 / ROADMAP v7.0 Phase 2)
// Sprint S308.
//
// Purpose:
//   Camera RAW formats (CR2, NEF, ARW, DNG, ORF …) embed a full-resolution
//   JPEG or a large preview JPEG directly inside the TIFF/RAW container.
//   Extracting this is orders of magnitude faster than full-blown RAW decode:
//
//     Full RAW decode: ~140-400 ms   (demosaic + white-balance + tonemapping)
//     EXIF JPEG extract: ~2-8 ms     (IStream seek + JFIF header strip)
//
//   EXIF tags used:
//     0x0201  JPEGInterchangeFormat      — byte offset of embedded JPEG SoI
//     0x0202  JPEGInterchangeFormatLength— byte length of the JPEG stream
//
//   When present, EmbeddedJpegExtractor returns the raw JPEG bytes which the
//   caller can decode with libjpeg-turbo at normal speed.
//
// Phase 2 wiring:
//   - RawDecoder::Decode() will call EmbeddedJpegExtractor::Extract() first.
//   - If extraction succeeds, the result bytes go directly to the JPEG decode
//     path, bypassing libraw entirely (fast path).
//   - Falls back to libraw full decode if no JPEG is found or the JPEG fails.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_EMBEDDED_JPEG_EXTRACTOR_H
#define EXPLORERLENS_ENGINE_EMBEDDED_JPEG_EXTRACTOR_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <optional>
#include <span>

// Forward-declare Windows COM interface to avoid pulling in objidl.h here.
struct IStream;

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// EmbeddedJpegInfo
// ---------------------------------------------------------------------------
// Result of a successful EXIF embedded-JPEG extraction.
//
struct EmbeddedJpegInfo final {
    std::vector<std::byte> jpegBytes;   ///< Raw JPEG bytes (starts with FF D8)
    std::uint32_t          offsetInFile{};  ///< Original byte offset in RAW file
    std::uint32_t          lengthInFile{};  ///< Original byte length in RAW file
    bool                   isFullResolution{ false }; ///< True if IFD0, false if thumbnail
};

// ---------------------------------------------------------------------------
// EXIF tag constants
// ---------------------------------------------------------------------------
namespace ExifTag {
    inline constexpr std::uint16_t kJpegInterchangeFormat       = 0x0201u;
    inline constexpr std::uint16_t kJpegInterchangeFormatLength = 0x0202u;

    inline constexpr std::uint16_t kSubIFDOffset         = 0x014A;  // SubIFD pointer
    inline constexpr std::uint16_t kExifIFDPointer       = 0x8769;  // Exif IFD pointer
} // namespace ExifTag

// ---------------------------------------------------------------------------
// EmbeddedJpegExtractor
// ---------------------------------------------------------------------------
// Stateless extractor: call Extract() with an IStream positioned at the
// beginning of the RAW file.  The extractor seeks internally and resets the
// stream position to the original offset on return.
//
// Thread-safety: Each call is independent; the class holds no mutable state.
//
class EmbeddedJpegExtractor final {
public:
    EmbeddedJpegExtractor() noexcept = default;
    ~EmbeddedJpegExtractor()         = default;

    // Non-copyable, non-movable (stateless; just construct on the stack).
    EmbeddedJpegExtractor(const EmbeddedJpegExtractor&)            = delete;
    EmbeddedJpegExtractor& operator=(const EmbeddedJpegExtractor&) = delete;

    // ── Primary API ───────────────────────────────────────────────────────────

    /// Attempt to extract the embedded JPEG from a RAW IStream.
    ///
    /// @param pStream  COM IStream at position 0 (or any; reset on return).
    /// @returns        EmbeddedJpegInfo if a JPEG was found and extracted,
    ///                 std::nullopt if the file has no EXIF JPEG or the
    ///                 TIFF/RAW header is malformed.
    ///
    /// @note  Phase 2 stub — returns std::nullopt until RAW/TIFF parser is
    ///        wired (see TODO below).
    [[nodiscard]] std::optional<EmbeddedJpegInfo>
    Extract(IStream* pStream) const noexcept;

    /// Validate that a byte sequence begins with a JFIF / JPEG SoI marker.
    [[nodiscard]] static bool IsJpegSoi(std::span<const std::byte> data) noexcept
    {
        return data.size() >= 2
            && data[0] == std::byte{ 0xFF }
            && data[1] == std::byte{ 0xD8 };
    }

    // ── Configuration ─────────────────────────────────────────────────────────

    /// Maximum JPEG size to accept as "embedded" (default 64 MiB).
    /// Prevents unbounded allocation on malformed inputs.
    static constexpr std::uint32_t kMaxEmbeddedJpegBytes = 64u * 1024u * 1024u;

    /// Minimum meaningful JPEG size (must have SoI + SoF + data).
    static constexpr std::uint32_t kMinEmbeddedJpegBytes = 128u;

private:
    // TODO(S308/Phase2): Implement TIFF IFD walker:
    //   1. Read TIFF byte-order mark (II=little-endian, MM=big-endian).
    //   2. Walk IFD0 for tags 0x0201 / 0x0202.
    //   3. If not found in IFD0, follow SubIFD (0x014A) chain.
    //   4. Seek to offset, read `length` bytes, verify JPEG SoI (FF D8).
    //   5. Return EmbeddedJpegInfo{ bytes, offset, length, isFullRes }.
    //
    // Expected implementation size: ~120 lines.
    // Dependency: none (pure IStream + manual TIFF parsing, no libtiff).
};

// ---------------------------------------------------------------------------
// Inline stub implementation (Phase 2 placeholder)
// ---------------------------------------------------------------------------
inline std::optional<EmbeddedJpegInfo>
EmbeddedJpegExtractor::Extract([[maybe_unused]] IStream* pStream) const noexcept
{
    // Phase 2 stub — returns no result until TIFF IFD walker is implemented.
    // RAW decode continues via libraw full-decode fallback.
    return std::nullopt;
}

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EMBEDDED_JPEG_EXTRACTOR_H
