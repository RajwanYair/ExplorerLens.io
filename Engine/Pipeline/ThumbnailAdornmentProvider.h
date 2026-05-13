// Engine/Pipeline/ThumbnailAdornmentProvider.h
// ExplorerLens — Shell API thumbnail adornments: TypeOverlay + Treatment (H35 + H38 / ROADMAP v8.0 Phase 2)
// Sprint S336.
//
// Purpose:
//   Two Windows shell mechanisms let extensions augment thumbnails without
//   custom drawing code:
//
//   H35 — TypeOverlay: The registry key
//       HKCR\.ext\TypeOverlay = "path_to_resource,idx"
//   tells Explorer to paint a small file-type badge over the lower-right
//   corner of every thumbnail for that extension.  This is the canonical way
//   to brand thumbnails (not painting directly on the HBITMAP).
//
//   H38 — Treatment: The registry DWORD values
//       HKCR\.ext\Treatment = 0  (plain — default)
//                            = 1  (drop shadow behind thumbnail)
//                            = 2  (white photo border + drop shadow)
//   apply a visual treatment to the thumbnail container that matches
//   Windows Explorer's built-in photo presentation style.
//
//   ThumbnailAdornmentProvider manages:
//     1. The configuration table of per-extension overlay and treatment settings
//     2. ApplyRegistryConfig() — writes HKCR entries at install/upgrade
//     3. QueryForExtension() — looks up config for a file extension
//
//   Integration:
//     Called from the installer (Install-ExplorerLens.ps1 → RegistrationHelper)
//     and from the COM server registration path.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_THUMBNAIL_ADORNMENT_PROVIDER_H
#define EXPLORERLENS_ENGINE_THUMBNAIL_ADORNMENT_PROVIDER_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <array>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// AdornmentKind — category of file group (drives icon/treatment selection)
// ---------------------------------------------------------------------------
enum class AdornmentKind : std::uint8_t {
    PHOTO        = 0,  ///< JPEG, PNG, TIFF, WebP, HEIC — photo border treatment
    DOCUMENT     = 1,  ///< PDF, SVG — plain treatment
    RAW_CAMERA   = 2,  ///< CR3, ARW, NEF, DNG — plain with camera overlay
    ARCHIVE      = 3,  ///< ZIP, CBZ, RAR — plain
    VIDEO        = 4,  ///< MP4, MOV, MKV — plain
    HDR_IMAGE    = 5,  ///< EXR, HDR — plain
    MODERN_IMAGE = 6,  ///< JXL, AVIF — photo border
    UNKNOWN      = 7,
};

// ---------------------------------------------------------------------------
// AdornmentTreatment — Windows Shell Treatment DWORD value
// ---------------------------------------------------------------------------
enum class AdornmentTreatment : std::uint32_t {
    PLAIN       = 0,  ///< Default: no special treatment
    DROP_SHADOW = 1,  ///< Drop shadow around thumbnail
    PHOTO_FRAME = 2,  ///< White photo border + drop shadow (best for photos)
};

// ---------------------------------------------------------------------------
// AdornmentConfig — per-extension adornment specification
// ---------------------------------------------------------------------------
struct AdornmentConfig final {
    std::string_view   extension;         ///< File extension (e.g., ".jpg")
    AdornmentKind      kind;              ///< File group classification
    AdornmentTreatment treatment;         ///< Shell Treatment value
    bool               hasTypeOverlay;    ///< Whether to write a TypeOverlay value
    std::string_view   overlayResource;   ///< Resource string for TypeOverlay (optional)
};

// ---------------------------------------------------------------------------
// ThumbnailAdornmentProvider
// ---------------------------------------------------------------------------
class ThumbnailAdornmentProvider final {
public:
    ThumbnailAdornmentProvider() = delete;

    // Number of entries in the adornment table
    static constexpr std::size_t kTableSize = 28u;

    // ------------------------------------------------------------------
    // QueryForExtension() — look up the adornment config for an extension.
    //   Returns nullptr if the extension is not in the table.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static const AdornmentConfig* QueryForExtension(std::string_view ext) noexcept
    {
        for (const auto& cfg : kTable)
            if (cfg.extension == ext) return &cfg;
        return nullptr;
    }

    // ------------------------------------------------------------------
    // TreatmentForKind() — get the default Treatment for an AdornmentKind
    // ------------------------------------------------------------------
    [[nodiscard]]
    static constexpr AdornmentTreatment TreatmentForKind(AdornmentKind k) noexcept
    {
        switch (k) {
        case AdornmentKind::PHOTO:        return AdornmentTreatment::PHOTO_FRAME;
        case AdornmentKind::MODERN_IMAGE: return AdornmentTreatment::PHOTO_FRAME;
        default:                          return AdornmentTreatment::PLAIN;
        }
    }

    // ------------------------------------------------------------------
    // IsPhotoFormat() — true for formats that should use PHOTO_FRAME
    // ------------------------------------------------------------------
    [[nodiscard]]
    static bool IsPhotoFormat(std::string_view ext) noexcept
    {
        const auto* cfg = QueryForExtension(ext);
        return cfg && cfg->treatment == AdornmentTreatment::PHOTO_FRAME;
    }

    // ------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kTreatmentPlain      = 0u;
    static constexpr std::uint32_t kTreatmentShadow     = 1u;
    static constexpr std::uint32_t kTreatmentPhotoFrame = 2u;

private:
    static constexpr std::array<AdornmentConfig, kTableSize> kTable = {{
        // Photos — white frame + shadow
        { ".jpg",   AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".jpeg",  AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".png",   AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".tif",   AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".tiff",  AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".bmp",   AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".gif",   AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".webp",  AdornmentKind::PHOTO,        AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".heic",  AdornmentKind::MODERN_IMAGE, AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".heif",  AdornmentKind::MODERN_IMAGE, AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".avif",  AdornmentKind::MODERN_IMAGE, AdornmentTreatment::PHOTO_FRAME, false, "" },
        { ".jxl",   AdornmentKind::MODERN_IMAGE, AdornmentTreatment::PHOTO_FRAME, false, "" },
        // Camera RAW — plain treatment, camera-type overlay
        { ".cr3",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        { ".cr2",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        { ".arw",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        { ".nef",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        { ".dng",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        { ".raf",   AdornmentKind::RAW_CAMERA,   AdornmentTreatment::PLAIN,       false, "" },
        // Document
        { ".pdf",   AdornmentKind::DOCUMENT,     AdornmentTreatment::PLAIN,       false, "" },
        { ".svg",   AdornmentKind::DOCUMENT,     AdornmentTreatment::PLAIN,       false, "" },
        // Archives
        { ".zip",   AdornmentKind::ARCHIVE,      AdornmentTreatment::PLAIN,       false, "" },
        { ".cbz",   AdornmentKind::ARCHIVE,      AdornmentTreatment::PLAIN,       false, "" },
        { ".cbr",   AdornmentKind::ARCHIVE,      AdornmentTreatment::PLAIN,       false, "" },
        // HDR
        { ".exr",   AdornmentKind::HDR_IMAGE,    AdornmentTreatment::PLAIN,       false, "" },
        { ".hdr",   AdornmentKind::HDR_IMAGE,    AdornmentTreatment::PLAIN,       false, "" },
        // Video
        { ".mp4",   AdornmentKind::VIDEO,        AdornmentTreatment::PLAIN,       false, "" },
        { ".mov",   AdornmentKind::VIDEO,        AdornmentTreatment::PLAIN,       false, "" },
        { ".mkv",   AdornmentKind::VIDEO,        AdornmentTreatment::PLAIN,       false, "" },
    }};
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_THUMBNAIL_ADORNMENT_PROVIDER_H
