// Engine/Pipeline/FormatFamilyDispatcher.h
#pragma once

// FormatFamilyDispatcher — 5-family decoder dispatch table (S368)
//
// Implements ROADMAP v8.0 §8.2: "Decoder Architecture — Simplify to 5 Families."
// Routes a decoded LENSTYPE to one of five decoder family groups:
//   Image | Modern | Camera | Document | Media
//
// This is distinct from SyntheticDecoderGenerator's DecoderFamily enum
// (which tracks generator metadata). FormatFamilyKind is the runtime dispatch
// classification used by the 8-stage pipeline (Stage 3: Decoder Selection).
//
// ROADMAP ref: ADR A32 — decoder families simplified to 5 (Phase 3, S368)
//
// Usage:
//   FormatFamilyKind fam = FormatFamilyDispatcher::Classify(LENSTYPE_HEIC);
//   switch (fam) {
//     case FormatFamilyKind::MODERN: return LazyLibraryLoader::Load(LIBHEIF);
//   }

#ifndef EXPLORERLENS_ENGINE_FORMATFAMILYDISPATCHER_H
#define EXPLORERLENS_ENGINE_FORMATFAMILYDISPATCHER_H

#include <cstdint>
#include <string>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// FormatFamilyKind — the 5 decoder family groups (ADR §8.2)
// ---------------------------------------------------------------------------
enum class FormatFamilyKind : std::uint8_t {
    UNKNOWN   = 0,  ///< Unknown/unsupported format
    IMAGE     = 1,  ///< WIC + libjpeg-turbo + libspng + libwebp + tinyexr
    MODERN    = 2,  ///< libheif + libjxl + libavif
    CAMERA    = 3,  ///< LibRaw — all camera RAW formats
    DOCUMENT  = 4,  ///< MuPDF + GDI+ + FreeType (PDF, SVG, Font, Office)
    MEDIA     = 5,  ///< Media Foundation + libarchive (Video, Audio, Archives)
};

// ---------------------------------------------------------------------------
// FormatFamilyRoute — a classified format with its preferred library hint
// ---------------------------------------------------------------------------
struct FormatFamilyRoute {
    FormatFamilyKind family          = FormatFamilyKind::UNKNOWN;
    std::wstring     formatExtension;   ///< Canonical lower-case extension (e.g. L"heic")
    std::wstring     primaryLibrary;    ///< Primary decode library name (informational)
    bool             supportsLazyLoad  = false; ///< true = handled by LazyLibraryLoader
    bool             requiresGpu       = false; ///< true = DXVA2 acceleration available

    [[nodiscard]] bool IsKnown() const noexcept { return family != FormatFamilyKind::UNKNOWN; }
};

// ---------------------------------------------------------------------------
// FormatFamilyDispatchConfig
// ---------------------------------------------------------------------------
struct FormatFamilyDispatchConfig {
    bool enableLazyLoading     = true;  ///< Route MODERN family through LazyLibraryLoader
    bool enableGpuAcceleration = false; ///< Route qualifying formats through DXVA2 (Phase 4)
    bool enableWicPassthrough  = true;  ///< Route IMAGE family through WIC if natively supported

    [[nodiscard]] static FormatFamilyDispatchConfig Default() noexcept {
        return FormatFamilyDispatchConfig{};
    }

    [[nodiscard]] static FormatFamilyDispatchConfig ForShellExtension() noexcept {
        FormatFamilyDispatchConfig cfg{};
        cfg.enableLazyLoading     = true;  // H43: lazy-load codec DLLs
        cfg.enableGpuAcceleration = false; // Phase 4 feature
        cfg.enableWicPassthrough  = true;  // H24: WIC first for standard formats
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// FormatFamilyDispatcher — static classification table
// ---------------------------------------------------------------------------
class FormatFamilyDispatcher final {
public:
    FormatFamilyDispatcher()                                         = delete;
    FormatFamilyDispatcher(const FormatFamilyDispatcher&)            = delete;
    FormatFamilyDispatcher& operator=(const FormatFamilyDispatcher&) = delete;

    /// Classifies a file extension into a FormatFamilyKind.
    /// ext must be lower-case without leading dot (e.g. "heic", "pdf", "cr3").
    [[nodiscard]] static FormatFamilyKind Classify(const wchar_t* ext) noexcept;

    /// Returns a fully-populated route record for the given extension.
    [[nodiscard]] static FormatFamilyRoute Route(const wchar_t* ext) noexcept;

    /// Returns a human-readable name for the family (for diagnostics/logging).
    [[nodiscard]] static const wchar_t* FamilyName(FormatFamilyKind kind) noexcept;

    /// Returns the total number of formats registered in the dispatch table.
    [[nodiscard]] static std::uint32_t RegisteredFormatCount() noexcept;

    // -----------------------------------------------------------------------
    // Well-known format counts per family (Phase 3 targets)
    // -----------------------------------------------------------------------
    static constexpr std::uint32_t kImageFormatCount    = 18u; ///< JPEG PNG BMP GIF TIFF WebP ICO TGA PPM QOI EXR HDR DDS PCXBMP DICOM TIF ...
    static constexpr std::uint32_t kModernFormatCount   =  5u; ///< HEIC AVIF JXL JP2 HEIF
    static constexpr std::uint32_t kCameraFormatCount   = 42u; ///< All LibRaw RAW extensions
    static constexpr std::uint32_t kDocumentFormatCount = 12u; ///< PDF SVG TTF OTF WOFF PS EPS DOC DOCX XPS CBZ ...
    static constexpr std::uint32_t kMediaFormatCount    = 25u; ///< MP4 MKV AVI MOV MP3 FLAC ZIP TAR CBR CBZ 7Z RAR ...
    static constexpr std::uint32_t kTotalFormatCount    =
        kImageFormatCount + kModernFormatCount + kCameraFormatCount +
        kDocumentFormatCount + kMediaFormatCount; // 102
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_FORMATFAMILYDISPATCHER_H
