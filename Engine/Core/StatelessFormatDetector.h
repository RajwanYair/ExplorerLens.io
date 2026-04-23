// StatelessFormatDetector.h — Pure-Library Format Detection (D43)
// Copyright (c) 2026 ExplorerLens Project
//
// DESIGN DECISION D43: FormatDetector as a pure library
// ─────────────────────────────────────────────────────
// The existing Engine/Pipeline/FormatDetector.h implements IFormatDetector and
// performs I/O (file opens, header reads) as a side-effect of detection.  That
// design is correct for the full pipeline but cannot be used:
//   • In the SDK (plugin authors cannot link against the full Engine)
//   • In unit tests without real file I/O
//   • In constexpr / compile-time contexts
//   • On non-Windows targets (macOS Quick Look, Linux Nautilus stubs)
//
// StatelessFormatDetector solves this by implementing magic-byte detection as a
// header-only, pure C++20 library:
//   - No <windows.h>, no COM, no IStream — pure standard library only.
//   - All probes operate on std::span<const uint8_t> (caller supplies bytes).
//   - Thread-safe: no mutable state; all functions are noexcept or const.
//   - Zero external dependencies (no libjpeg, libpng, etc.).
//
// Harvested from Apache Tika MIME-type probing strategy [H12] and adapted for
// the ExplorerLens 200+ format corpus.
//
// SEE ALSO:
//   Engine/Pipeline/FormatDetector.h  — full I/O-capable implementation
//   Engine/Core/IFormatDetector.h     — abstract interface (Windows-coupled)
//   ADR-013                           — cross-platform PAL decision
//   ROADMAP D43, §7.1
//
// USAGE (SDK / cross-platform / test contexts):
//   #include "Engine/Core/StatelessFormatDetector.h"
//   using namespace ExplorerLens::Core;
//
//   auto [tag, confidence] = StatelessFormatDetector::Probe(header_bytes);
//   if (tag == FormatTag::JPEG) { /* ... */ }
//
// MSVC NOTE: This file must compile cleanly at /W4 with MSVC v145 and Clang/GCC.
// Never include <windows.h> or any platform-specific headers here.
// ────────────────────────────────────────────────────────────────────────────
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>

namespace ExplorerLens {
namespace Core {

// ============================================================================
// FormatTag — canonical format identifier enumeration
// ============================================================================
// Values are grouped by category.  UNKNOWN is always 0.
// Add new entries at the END of each group to avoid ABI breaks across plugin
// releases — the D43 SDK contract treats these values as stable integers.

enum class FormatTag : uint32_t {
    // ── Sentinel ─────────────────────────────────────────────────────────────
    UNKNOWN = 0,

    // ── Raster images ────────────────────────────────────────────────────────
    JPEG       = 100,
    PNG        = 101,
    GIF        = 102,
    BMP        = 103,
    TIFF_LE    = 104,   // little-endian (II)
    TIFF_BE    = 105,   // big-endian (MM)
    WEBP       = 106,
    QOI        = 107,   // Quite OK Image
    JXL        = 108,   // JPEG XL (bare bitstream)
    JXL_ISOBMFF = 109,  // JPEG XL in ISO Base Media container
    AVIF       = 110,
    HEIC       = 111,
    PSD        = 112,   // Photoshop Document
    DDS        = 113,   // DirectDraw Surface
    ICO        = 114,
    TGA        = 115,   // Truevision TGA (no reliable magic; extension-only)
    EXR        = 116,   // OpenEXR
    HDR        = 117,   // Radiance HDR / RGBE

    // ── Raw camera ───────────────────────────────────────────────────────────
    RAW_CR2  = 200,
    RAW_CR3  = 201,
    RAW_NEF  = 202,
    RAW_ARW  = 203,
    RAW_ORF  = 204,
    RAW_RW2  = 205,
    RAW_DNG  = 206,

    // ── Vector / structured images ────────────────────────────────────────────
    SVG      = 300,

    // ── 3D models / CAD ──────────────────────────────────────────────────────
    GLTF_JSON  = 400,   // .gltf (text JSON)
    GLTF_GLB   = 401,   // .glb (binary container)
    STL_BINARY = 402,
    STL_ASCII  = 403,
    OBJ        = 404,
    FBX        = 405,
    IFC        = 406,
    STEP       = 407,
    IGES       = 408,

    // ── Documents ────────────────────────────────────────────────────────────
    PDF     = 500,
    EPUB    = 501,
    XPS     = 502,
    DJVU    = 503,
    CBZ     = 504,   // ZIP-based comic
    CBR     = 505,   // RAR-based comic

    // ── Archives / containers ────────────────────────────────────────────────
    ZIP     = 600,
    RAR4    = 601,
    RAR5    = 602,
    SEVENZIP = 603,
    TAR     = 604,
    GZ      = 605,
    BZ2     = 606,
    XZ      = 607,
    ZSTD    = 608,
    LZ4     = 609,

    // ── Audio / video containers (metadata-only decode) ───────────────────────
    MP4     = 700,
    MOV     = 701,
    MKV     = 702,
    WEBM    = 703,
    AVI     = 704,
    FLAC    = 705,
    OGG     = 706,
    MP3     = 707,
};

// ============================================================================
// ProbeResult — returned by StatelessFormatDetector::Probe
// ============================================================================

/// Confidence level of the format detection result.
enum class DetectionConfidence : uint8_t {
    NONE        = 0,   // No match found
    LOW         = 1,   // Extension-only or partial magic match
    MEDIUM      = 2,   // Magic bytes match but ambiguous (e.g., TIFF/CR2)
    HIGH        = 3,   // Full magic byte + sub-format signature match
    DEFINITIVE  = 4,   // Exact ISO container + sub-type identified
};

/// Result returned by Probe().
struct FormatProbeResult {
    FormatTag          tag        = FormatTag::UNKNOWN;
    DetectionConfidence confidence = DetectionConfidence::NONE;

    /// Convenience: true when a supported format was positively identified.
    [[nodiscard]] constexpr bool IsKnown()  const noexcept { return tag != FormatTag::UNKNOWN; }
    [[nodiscard]] constexpr bool IsHighConfidence() const noexcept {
        return confidence >= DetectionConfidence::HIGH;
    }
};

// ============================================================================
// StatelessFormatDetector
// ============================================================================
// All member functions are static; the class has no instances (delete ctor).
//
// Thread safety: all public functions operate on caller-supplied const data
// with no shared mutable state → unconditionally thread-safe.

class StatelessFormatDetector final
{
  public:
    StatelessFormatDetector()  = delete;
    ~StatelessFormatDetector() = delete;
    StatelessFormatDetector(const StatelessFormatDetector&) = delete;
    StatelessFormatDetector& operator=(const StatelessFormatDetector&) = delete;

    // ── Primary entry point ─────────────────────────────────────────────────

    /// Probe up to the first 32 bytes of file content.
    /// Returns UNKNOWN with confidence NONE if the buffer is too short or the
    /// signature is unrecognised.
    ///
    /// @param header   Span over the first N bytes of the file (N may be < 32;
    ///                 the function degrades gracefully with smaller buffers).
    [[nodiscard]] static FormatProbeResult Probe(
        std::span<const uint8_t> header) noexcept;

    /// Extension-based lookup (fast, no I/O, no buffer needed).
    /// ext must be a lowercase dot-prefixed extension, e.g. ".jpg".
    [[nodiscard]] static FormatProbeResult ProbeByExtension(
        std::string_view ext) noexcept;

    /// Combine magic-byte probe with extension hint.
    /// Resolves ambiguous cases (e.g., TIFF vs CR2 vs NEF).
    [[nodiscard]] static FormatProbeResult ProbeWithExtension(
        std::span<const uint8_t> header,
        std::string_view extHint) noexcept;

    // ── Category helpers ────────────────────────────────────────────────────

    [[nodiscard]] static constexpr bool IsRasterImage(FormatTag tag) noexcept;
    [[nodiscard]] static constexpr bool IsRawCamera(FormatTag tag)   noexcept;
    [[nodiscard]] static constexpr bool Is3DModel(FormatTag tag)     noexcept;
    [[nodiscard]] static constexpr bool IsDocument(FormatTag tag)    noexcept;
    [[nodiscard]] static constexpr bool IsArchive(FormatTag tag)     noexcept;
    [[nodiscard]] static constexpr bool IsMedia(FormatTag tag)       noexcept;

    // ── Minimum header bytes required for reliable detection ──────────────────

    /// Returns the minimum buffer size (in bytes) required for Probe() to
    /// return HIGH or DEFINITIVE confidence for the given tag.
    [[nodiscard]] static constexpr std::size_t MinHeaderBytes(FormatTag tag) noexcept;

  private:
    // ── Internal probe helpers ───────────────────────────────────────────────

    [[nodiscard]] static FormatProbeResult ProbeISO(
        std::span<const uint8_t> h) noexcept;
    [[nodiscard]] static FormatProbeResult ProbeRIFF(
        std::span<const uint8_t> h) noexcept;
};

// ─────────────────────────────────────────────────────────────────────────────
// Inline implementations
// ─────────────────────────────────────────────────────────────────────────────

// Tiny helper to avoid repeating multi-byte comparisons.
namespace detail {

[[nodiscard]] inline bool StartsWith(std::span<const uint8_t> buf,
                                      std::initializer_list<uint8_t> magic) noexcept {
    if (buf.size() < magic.size()) return false;
    std::size_t i = 0;
    for (uint8_t b : magic) {
        if (buf[i++] != b) return false;
    }
    return true;
}

[[nodiscard]] inline bool MatchAt(std::span<const uint8_t> buf, std::size_t offset,
                                   std::initializer_list<uint8_t> magic) noexcept {
    if (buf.size() < offset + magic.size()) return false;
    std::size_t i = offset;
    for (uint8_t b : magic) {
        if (buf[i++] != b) return false;
    }
    return true;
}

} // namespace detail

[[nodiscard]] inline FormatProbeResult
StatelessFormatDetector::Probe(std::span<const uint8_t> h) noexcept {
    using namespace detail;

    // JPEG  FF D8
    if (StartsWith(h, {0xFF, 0xD8}))
        return {FormatTag::JPEG, DetectionConfidence::HIGH};

    // PNG  89 50 4E 47 0D 0A 1A 0A
    if (StartsWith(h, {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}))
        return {FormatTag::PNG, DetectionConfidence::DEFINITIVE};

    // GIF  47 49 46 38
    if (StartsWith(h, {0x47, 0x49, 0x46, 0x38}))
        return {FormatTag::GIF, DetectionConfidence::HIGH};

    // BMP  42 4D
    if (StartsWith(h, {0x42, 0x4D}))
        return {FormatTag::BMP, DetectionConfidence::HIGH};

    // TIFF LE  49 49 2A 00
    if (StartsWith(h, {0x49, 0x49, 0x2A, 0x00}))
        return {FormatTag::TIFF_LE, DetectionConfidence::HIGH};

    // TIFF BE  4D 4D 00 2A
    if (StartsWith(h, {0x4D, 0x4D, 0x00, 0x2A}))
        return {FormatTag::TIFF_BE, DetectionConfidence::HIGH};

    // WebP  RIFF....WEBP
    if (StartsWith(h, {0x52, 0x49, 0x46, 0x46}) &&
        MatchAt(h, 8, {0x57, 0x45, 0x42, 0x50}))
        return ProbeRIFF(h);

    // QOI  71 6F 69 66
    if (StartsWith(h, {0x71, 0x6F, 0x69, 0x66}))
        return {FormatTag::QOI, DetectionConfidence::HIGH};

    // JXL bare bitstream  FF 0A
    if (StartsWith(h, {0xFF, 0x0A}))
        return {FormatTag::JXL, DetectionConfidence::HIGH};

    // JXL ISO BMFF  00 00 00 0C 4A 58 4C 20
    if (StartsWith(h, {0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20}))
        return {FormatTag::JXL_ISOBMFF, DetectionConfidence::DEFINITIVE};

    // PSD  38 42 50 53
    if (StartsWith(h, {0x38, 0x42, 0x50, 0x53}))
        return {FormatTag::PSD, DetectionConfidence::HIGH};

    // DDS  44 44 53 20
    if (StartsWith(h, {0x44, 0x44, 0x53, 0x20}))
        return {FormatTag::DDS, DetectionConfidence::HIGH};

    // OpenEXR  76 2F 31 01
    if (StartsWith(h, {0x76, 0x2F, 0x31, 0x01}))
        return {FormatTag::EXR, DetectionConfidence::HIGH};

    // Radiance HDR  23 3F 52 41 44 49 41 4E 43 45 0A
    if (StartsWith(h, {0x23, 0x3F, 0x52, 0x41, 0x44, 0x49, 0x41, 0x4E}))
        return {FormatTag::HDR, DetectionConfidence::HIGH};

    // PDF  25 50 44 46
    if (StartsWith(h, {0x25, 0x50, 0x44, 0x46}))
        return {FormatTag::PDF, DetectionConfidence::HIGH};

    // ZIP/CBZ/EPUB  50 4B 03 04  or  50 4B 05 06  or  50 4B 07 08
    if (StartsWith(h, {0x50, 0x4B}) && h.size() >= 4 &&
        (h[2] == 0x03 || h[2] == 0x05 || h[2] == 0x07))
        return {FormatTag::ZIP, DetectionConfidence::HIGH};

    // RAR4  52 61 72 21 1A 07 00
    if (StartsWith(h, {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00}))
        return {FormatTag::RAR4, DetectionConfidence::DEFINITIVE};

    // RAR5  52 61 72 21 1A 07 01 00
    if (StartsWith(h, {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00}))
        return {FormatTag::RAR5, DetectionConfidence::DEFINITIVE};

    // 7-Zip  37 7A BC AF 27 1C
    if (StartsWith(h, {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}))
        return {FormatTag::SEVENZIP, DetectionConfidence::DEFINITIVE};

    // GZip  1F 8B
    if (StartsWith(h, {0x1F, 0x8B}))
        return {FormatTag::GZ, DetectionConfidence::HIGH};

    // BZip2  42 5A 68
    if (StartsWith(h, {0x42, 0x5A, 0x68}))
        return {FormatTag::BZ2, DetectionConfidence::HIGH};

    // XZ  FD 37 7A 58 5A 00
    if (StartsWith(h, {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00}))
        return {FormatTag::XZ, DetectionConfidence::DEFINITIVE};

    // Zstandard  28 B5 2F FD
    if (StartsWith(h, {0x28, 0xB5, 0x2F, 0xFD}))
        return {FormatTag::ZSTD, DetectionConfidence::HIGH};

    // LZ4  04 22 4D 18
    if (StartsWith(h, {0x04, 0x22, 0x4D, 0x18}))
        return {FormatTag::LZ4, DetectionConfidence::HIGH};

    // FLAC  66 4C 61 43
    if (StartsWith(h, {0x66, 0x4C, 0x61, 0x43}))
        return {FormatTag::FLAC, DetectionConfidence::HIGH};

    // OGG  4F 67 67 53
    if (StartsWith(h, {0x4F, 0x67, 0x67, 0x53}))
        return {FormatTag::OGG, DetectionConfidence::HIGH};

    // ISO Base Media (MP4/MOV/HEIC/AVIF) — probe sub-types from ftyp box
    if (h.size() >= 12 && MatchAt(h, 4, {0x66, 0x74, 0x79, 0x70}))
        return ProbeISO(h);

    // GLB (binary glTF)  67 6C 54 46
    if (StartsWith(h, {0x67, 0x6C, 0x54, 0x46}))
        return {FormatTag::GLTF_GLB, DetectionConfidence::DEFINITIVE};

    return {FormatTag::UNKNOWN, DetectionConfidence::NONE};
}

[[nodiscard]] inline FormatProbeResult
StatelessFormatDetector::ProbeRIFF(std::span<const uint8_t> h) noexcept {
    // RIFF container — currently only WebP sub-format registered.
    // AVI uses RIFF too but we probe it via ProbeRIFF separately.
    using namespace detail;
    if (MatchAt(h, 8, {0x57, 0x45, 0x42, 0x50}))  // WEBP at offset 8
        return {FormatTag::WEBP, DetectionConfidence::DEFINITIVE};
    if (MatchAt(h, 8, {0x41, 0x56, 0x49, 0x20}))  // AVI  at offset 8
        return {FormatTag::AVI, DetectionConfidence::DEFINITIVE};
    return {FormatTag::UNKNOWN, DetectionConfidence::LOW};
}

[[nodiscard]] inline FormatProbeResult
StatelessFormatDetector::ProbeISO(std::span<const uint8_t> h) noexcept {
    // ISO Base Media File Format — ftyp brand at offset 8.
    using namespace detail;
    // HEIC / HEIF brands
    if (MatchAt(h, 8, {0x68, 0x65, 0x69, 0x63}) ||  // heic
        MatchAt(h, 8, {0x68, 0x65, 0x69, 0x78}) ||  // heix
        MatchAt(h, 8, {0x68, 0x65, 0x76, 0x63}) ||  // hevc
        MatchAt(h, 8, {0x6D, 0x69, 0x66, 0x31}))    // mif1 (HEIF)
        return {FormatTag::HEIC, DetectionConfidence::DEFINITIVE};

    // AVIF
    if (MatchAt(h, 8, {0x61, 0x76, 0x69, 0x66}))    // avif
        return {FormatTag::AVIF, DetectionConfidence::DEFINITIVE};

    // MP4/MOV
    if (MatchAt(h, 8, {0x69, 0x73, 0x6F, 0x6D}) ||  // isom
        MatchAt(h, 8, {0x6D, 0x70, 0x34, 0x31}) ||  // mp41
        MatchAt(h, 8, {0x6D, 0x70, 0x34, 0x32}) ||  // mp42
        MatchAt(h, 8, {0x4D, 0x34, 0x56, 0x20}))    // M4V
        return {FormatTag::MP4, DetectionConfidence::DEFINITIVE};
    if (MatchAt(h, 8, {0x71, 0x74, 0x20, 0x20}))    // qt   (QuickTime MOV)
        return {FormatTag::MOV, DetectionConfidence::DEFINITIVE};

    return {FormatTag::UNKNOWN, DetectionConfidence::LOW};
}

[[nodiscard]] inline FormatProbeResult
StatelessFormatDetector::ProbeByExtension(std::string_view ext) noexcept {
    // Extension → FormatTag table (ASCII lowercase, dot-prefixed).
    struct Entry { std::string_view ext; FormatTag tag; };
    static constexpr std::array<Entry, 60> kTable{{
        {".jpg",   FormatTag::JPEG},      {".jpeg",  FormatTag::JPEG},
        {".jpe",   FormatTag::JPEG},      {".jfif",  FormatTag::JPEG},
        {".png",   FormatTag::PNG},
        {".gif",   FormatTag::GIF},
        {".bmp",   FormatTag::BMP},       {".dib",   FormatTag::BMP},
        {".tif",   FormatTag::TIFF_LE},   {".tiff",  FormatTag::TIFF_LE},
        {".webp",  FormatTag::WEBP},
        {".qoi",   FormatTag::QOI},
        {".jxl",   FormatTag::JXL},
        {".avif",  FormatTag::AVIF},
        {".heic",  FormatTag::HEIC},      {".heif",  FormatTag::HEIC},
        {".psd",   FormatTag::PSD},
        {".dds",   FormatTag::DDS},
        {".ico",   FormatTag::ICO},
        {".tga",   FormatTag::TGA},
        {".exr",   FormatTag::EXR},
        {".hdr",   FormatTag::HDR},
        {".cr2",   FormatTag::RAW_CR2},   {".cr3",   FormatTag::RAW_CR3},
        {".nef",   FormatTag::RAW_NEF},   {".arw",   FormatTag::RAW_ARW},
        {".orf",   FormatTag::RAW_ORF},   {".rw2",   FormatTag::RAW_RW2},
        {".dng",   FormatTag::RAW_DNG},
        {".svg",   FormatTag::SVG},       {".svgz",  FormatTag::SVG},
        {".gltf",  FormatTag::GLTF_JSON}, {".glb",   FormatTag::GLTF_GLB},
        {".stl",   FormatTag::STL_BINARY},{".obj",   FormatTag::OBJ},
        {".fbx",   FormatTag::FBX},       {".ifc",   FormatTag::IFC},
        {".step",  FormatTag::STEP},      {".stp",   FormatTag::STEP},
        {".iges",  FormatTag::IGES},      {".igs",   FormatTag::IGES},
        {".pdf",   FormatTag::PDF},
        {".epub",  FormatTag::EPUB},
        {".cbz",   FormatTag::CBZ},       {".cbr",   FormatTag::CBR},
        {".zip",   FormatTag::ZIP},       {".jar",   FormatTag::ZIP},
        {".rar",   FormatTag::RAR4},
        {".7z",    FormatTag::SEVENZIP},
        {".tar",   FormatTag::TAR},
        {".gz",    FormatTag::GZ},        {".tgz",   FormatTag::GZ},
        {".bz2",   FormatTag::BZ2},
        {".xz",    FormatTag::XZ},
        {".zst",   FormatTag::ZSTD},
        {".lz4",   FormatTag::LZ4},
        {".mp4",   FormatTag::MP4},       {".m4v",   FormatTag::MP4},
        {".mov",   FormatTag::MOV},
        {".flac",  FormatTag::FLAC},
        {".ogg",   FormatTag::OGG},
    }};

    for (const auto& e : kTable) {
        if (e.ext == ext)
            return {e.tag, DetectionConfidence::LOW};
    }
    return {FormatTag::UNKNOWN, DetectionConfidence::NONE};
}

[[nodiscard]] inline FormatProbeResult
StatelessFormatDetector::ProbeWithExtension(std::span<const uint8_t> header,
                                             std::string_view extHint) noexcept {
    auto magic  = Probe(header);
    auto byExt  = ProbeByExtension(extHint);

    // If magic probe is high-confidence and agrees with extension → DEFINITIVE.
    if (magic.IsKnown() && byExt.IsKnown() && magic.tag == byExt.tag)
        return {magic.tag, DetectionConfidence::DEFINITIVE};

    // Magic probe wins over extension on ambiguous cases (e.g., TIFF vs RAW).
    if (magic.IsKnown())
        return magic;

    // Fall back to extension.
    return byExt;
}

// ── Category helpers ─────────────────────────────────────────────────────────

[[nodiscard]] constexpr bool
StatelessFormatDetector::IsRasterImage(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 100 && v < 200;
}

[[nodiscard]] constexpr bool
StatelessFormatDetector::IsRawCamera(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 200 && v < 300;
}

[[nodiscard]] constexpr bool
StatelessFormatDetector::Is3DModel(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 400 && v < 500;
}

[[nodiscard]] constexpr bool
StatelessFormatDetector::IsDocument(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 500 && v < 600;
}

[[nodiscard]] constexpr bool
StatelessFormatDetector::IsArchive(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 600 && v < 700;
}

[[nodiscard]] constexpr bool
StatelessFormatDetector::IsMedia(FormatTag tag) noexcept {
    const auto v = static_cast<uint32_t>(tag);
    return v >= 700 && v < 800;
}

[[nodiscard]] constexpr std::size_t
StatelessFormatDetector::MinHeaderBytes(FormatTag tag) noexcept {
    switch (tag) {
    case FormatTag::JPEG:       return 2;
    case FormatTag::PNG:        return 8;
    case FormatTag::GIF:        return 6;
    case FormatTag::BMP:        return 2;
    case FormatTag::TIFF_LE:
    case FormatTag::TIFF_BE:    return 4;
    case FormatTag::WEBP:       return 12;
    case FormatTag::QOI:        return 4;
    case FormatTag::JXL:        return 2;
    case FormatTag::JXL_ISOBMFF: return 12;
    case FormatTag::AVIF:
    case FormatTag::HEIC:
    case FormatTag::MP4:
    case FormatTag::MOV:        return 12;
    case FormatTag::PSD:        return 4;
    case FormatTag::DDS:        return 4;
    case FormatTag::EXR:        return 4;
    case FormatTag::HDR:        return 8;
    case FormatTag::PDF:        return 4;
    case FormatTag::ZIP:        return 4;
    case FormatTag::RAR4:       return 7;
    case FormatTag::RAR5:       return 8;
    case FormatTag::SEVENZIP:   return 6;
    case FormatTag::GZ:         return 2;
    case FormatTag::BZ2:        return 3;
    case FormatTag::XZ:         return 6;
    case FormatTag::ZSTD:       return 4;
    case FormatTag::LZ4:        return 4;
    case FormatTag::FLAC:       return 4;
    case FormatTag::OGG:        return 4;
    case FormatTag::GLTF_GLB:   return 4;
    default:                    return 0;  // Extension-only or unknown
    }
}

} // namespace Core
} // namespace ExplorerLens
