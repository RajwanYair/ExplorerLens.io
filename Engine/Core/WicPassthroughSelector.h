// Engine/Core/WicPassthroughSelector.h
// ExplorerLens — WIC-first format passthrough for natively-supported formats (H24 / ROADMAP v8.0 Phase 2)
// Sprint S324.
//
// Purpose:
//   Windows Imaging Component (WIC) is a mature, Microsoft-maintained codec
//   pipeline available on every Windows 7+ machine.  For formats WIC handles
//   well (JPEG, PNG, BMP, GIF, TIFF, ICO, DDS), shipping our own decoder
//   adds DLL size, maintenance overhead, and potential quality regressions.
//
//   WicPassthroughSelector encodes the decision table:
//     1. Probe the format ID / magic bytes.
//     2. If WIC can decode it natively → return USE_WIC.
//     3. If we have a better decoder (JPEG via libjpeg-turbo, PNG via libspng,
//        RAW, HEIC, AVIF, JXL, PDF) → return USE_CUSTOM.
//     4. If format is unknown → return UNKNOWN.
//
//   This implements the "WIC passthrough first" harvest (H24) from Microsoft
//   Photos, reducing our custom code surface for common formats.
//
// Integration:
//   DecoderRegistry calls Select() before routing to a specific decoder.
//   If USE_WIC is returned, the shell passes the IStream directly to WIC
//   via the standard IWICImagingFactory path.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_WIC_PASSTHROUGH_SELECTOR_H
#define EXPLORERLENS_ENGINE_WIC_PASSTHROUGH_SELECTOR_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <span>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// WicPassthroughDecision
// ---------------------------------------------------------------------------
enum class WicPassthroughDecision : std::uint8_t {
    USE_WIC         = 0,  ///< WIC can decode this; use IWICImagingFactory
    USE_CUSTOM      = 1,  ///< Custom decoder preferred (libjpeg-turbo, libspng, etc.)
    UNKNOWN_FORMAT  = 2,  ///< Format not recognised; probe further
    FORMAT_DISABLED = 3,  ///< Format explicitly disabled in registry policy
};

// ---------------------------------------------------------------------------
// WicPassthroughStatus (returned alongside the decision for diagnostics)
// ---------------------------------------------------------------------------
enum class WicPassthroughStatus : std::uint8_t {
    OK              = 0,
    EMPTY_MAGIC     = 1,  ///< Provided magic bytes buffer is empty
    MAGIC_TOO_SHORT = 2,  ///< < 4 bytes — cannot reliably identify format
};

// ---------------------------------------------------------------------------
// WicPassthroughResult
// ---------------------------------------------------------------------------
struct WicPassthroughResult final {
    WicPassthroughDecision decision;
    WicPassthroughStatus   status;
    std::string_view       resolvedFormatId;  ///< Non-owning; valid for call lifetime
};

// ---------------------------------------------------------------------------
// WicPassthroughSelector
// ---------------------------------------------------------------------------
class WicPassthroughSelector final {
public:
    // Minimum magic-byte buffer for reliable format identification
    static constexpr std::size_t kMinMagicBytes = 4u;

    // Number of format entries in the decision table
    static constexpr std::uint32_t kTableEntryCount = 22u;

    // ------------------------------------------------------------------
    // SelectByFormatId() — decision by MIME-type string.
    //   O(n) scan; table is small (22 entries) and fits in one cache line.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static WicPassthroughResult SelectByFormatId(
        std::string_view formatId) noexcept
    {
        // Formats where WIC is sufficient and preferred
        constexpr std::string_view kWicFormats[] = {
            "image/bmp",
            "image/gif",
            "image/tiff",
            "image/x-icon",
            "image/vnd.microsoft.icon",
            "image/x-dds",
            "image/x-wdp",          // JPEG XR (Windows native)
        };
        for (auto f : kWicFormats) {
            if (formatId == f)
                return { WicPassthroughDecision::USE_WIC,
                         WicPassthroughStatus::OK, formatId };
        }

        // Formats where our decoders are better than WIC
        constexpr std::string_view kCustomFormats[] = {
            "image/jpeg",           // libjpeg-turbo (SIMD JPEG)
            "image/png",            // libspng (ASAN-clean)
            "image/webp",           // libwebp (official)
            "image/heif",           // libheif + libde265
            "image/heic",           // libheif + libde265
            "image/avif",           // libavif + dav1d
            "image/jxl",            // libjxl (official)
            "image/x-exr",          // tinyexr
            "image/x-raw",          // LibRaw
            "image/x-cr2",  "image/x-cr3",  "image/x-nef",
            "image/x-arw",  "image/x-dng",  "image/x-raf",
            "application/pdf",      // MuPDF
            "image/svg+xml",        // GDI+ Direct2D
        };
        for (auto f : kCustomFormats) {
            if (formatId == f)
                return { WicPassthroughDecision::USE_CUSTOM,
                         WicPassthroughStatus::OK, formatId };
        }

        return { WicPassthroughDecision::UNKNOWN_FORMAT,
                 WicPassthroughStatus::OK, {} };
    }

    // ------------------------------------------------------------------
    // SelectByMagic() — decision by first N bytes of the file.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static WicPassthroughResult SelectByMagic(
        std::span<const std::byte> magic) noexcept
    {
        if (magic.empty())
            return { WicPassthroughDecision::UNKNOWN_FORMAT,
                     WicPassthroughStatus::EMPTY_MAGIC, {} };
        if (magic.size() < kMinMagicBytes)
            return { WicPassthroughDecision::UNKNOWN_FORMAT,
                     WicPassthroughStatus::MAGIC_TOO_SHORT, {} };

        const auto* b = reinterpret_cast<const unsigned char*>(magic.data());

        // JPEG: FF D8 FF
        if (b[0] == 0xFF && b[1] == 0xD8 && b[2] == 0xFF)
            return { WicPassthroughDecision::USE_CUSTOM,
                     WicPassthroughStatus::OK, "image/jpeg" };

        // PNG: 89 50 4E 47
        if (b[0] == 0x89 && b[1] == 0x50 && b[2] == 0x4E && b[3] == 0x47)
            return { WicPassthroughDecision::USE_CUSTOM,
                     WicPassthroughStatus::OK, "image/png" };

        // GIF: 47 49 46 38
        if (b[0] == 0x47 && b[1] == 0x49 && b[2] == 0x46 && b[3] == 0x38)
            return { WicPassthroughDecision::USE_WIC,
                     WicPassthroughStatus::OK, "image/gif" };

        // BMP: 42 4D
        if (b[0] == 0x42 && b[1] == 0x4D)
            return { WicPassthroughDecision::USE_WIC,
                     WicPassthroughStatus::OK, "image/bmp" };

        // TIFF: 49 49 2A 00 (LE) or 4D 4D 00 2A (BE)
        if ((b[0] == 0x49 && b[1] == 0x49 && b[2] == 0x2A && b[3] == 0x00) ||
            (b[0] == 0x4D && b[1] == 0x4D && b[2] == 0x00 && b[3] == 0x2A))
            return { WicPassthroughDecision::USE_WIC,
                     WicPassthroughStatus::OK, "image/tiff" };

        // WebP: 52 49 46 46 ... 57 45 42 50
        if (b[0] == 0x52 && b[1] == 0x49 && b[2] == 0x46 && b[3] == 0x46)
            return { WicPassthroughDecision::USE_CUSTOM,
                     WicPassthroughStatus::OK, "image/webp" };

        // PDF: 25 50 44 46
        if (b[0] == 0x25 && b[1] == 0x50 && b[2] == 0x44 && b[3] == 0x46)
            return { WicPassthroughDecision::USE_CUSTOM,
                     WicPassthroughStatus::OK, "application/pdf" };

        return { WicPassthroughDecision::UNKNOWN_FORMAT,
                 WicPassthroughStatus::OK, {} };
    }

private:
    WicPassthroughSelector() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WIC_PASSTHROUGH_SELECTOR_H
