// Engine/Core/PerFormatMemoryBudget.h
// ExplorerLens — Per-format memory allocation limits (H42 / ROADMAP v8.0 Phase 2)
// Sprint S323.
//
// Purpose:
//   A shell extension that decodes user-supplied files is a natural target for
//   allocation-bomb attacks: a specially crafted 1 KB RAW file that claims
//   a 10 GB decoded image.  Without a memory budget, one malicious file can
//   OOM the Explorer process.
//
//   PerFormatMemoryBudget provides:
//     1. A static table of per-format limits (in bytes) based on the format's
//        realistic maximum use-case for thumbnail generation.
//     2. A BudgetCheck() entry-point that decoders call before allocating the
//        decode buffer, returning BudgetExceeded immediately if the claimed
//        size exceeds the limit.
//     3. A fallback budget for unknown formats (conservative: 16 MiB).
//
//   Design principle (H42 harvest):
//     "RAW files get 128 MB; ICO files get 1 MB."
//     — limits scale with the realistic maximum output size at thumbnail cx.
//
// Integration:
//   Call PerFormatMemoryBudget::Check(format, claimedBytes) at the start of
//   every Decode() implementation.  Return E_OUTOFMEMORY on BudgetExceeded.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_PER_FORMAT_MEMORY_BUDGET_H
#define EXPLORERLENS_ENGINE_PER_FORMAT_MEMORY_BUDGET_H

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// BudgetCheckResult
// ---------------------------------------------------------------------------
enum class BudgetCheckResult : std::uint8_t {
    OK               = 0,  ///< Allocation is within the format's budget
    BUDGET_EXCEEDED  = 1,  ///< Claimed size exceeds the per-format limit
    ZERO_BUDGET      = 2,  ///< Format budget is zero (format disabled)
    UNKNOWN_FORMAT   = 3,  ///< Format ID not in table; fallback budget applied
};

// ---------------------------------------------------------------------------
// PerFormatMemoryBudget
// ---------------------------------------------------------------------------
class PerFormatMemoryBudget final {
public:
    // ------------------------------------------------------------------
    // Per-format limits (bytes).  Scale by thumbnail use-case, not
    // the maximum possible decoded image size.
    // ------------------------------------------------------------------

    // Camera RAW: full 24 MP demosaic at 16-bit = ~144 MiB; pad to 128 MiB
    // because we use embedded JPEG fast-path when available.
    static constexpr std::size_t kBudgetRaw       = 128u * 1024u * 1024u;

    // HEIF / AVIF / JXL: 4K @ 4bpp = 33 MiB; budget 64 MiB for HDR tiles.
    static constexpr std::size_t kBudgetHdr       =  64u * 1024u * 1024u;

    // Standard raster (JPEG, PNG, WebP, BMP, TIFF): 4K BGRA = 33 MiB; 48 MiB.
    static constexpr std::size_t kBudgetRaster    =  48u * 1024u * 1024u;

    // PDF page rasterisation at 96 DPI for thumbnail: typically < 4 MiB.
    static constexpr std::size_t kBudgetPdf       =  16u * 1024u * 1024u;

    // Archives: decode only the cover image inside; treat as raster budget.
    static constexpr std::size_t kBudgetArchive   =  48u * 1024u * 1024u;

    // Vector (SVG): rasterised at thumbnail size; 8 MiB is generous.
    static constexpr std::size_t kBudgetVector    =   8u * 1024u * 1024u;

    // Font / icon formats: small by design.
    static constexpr std::size_t kBudgetIcon      =   1u * 1024u * 1024u;

    // Video keyframe: single frame at full resolution.
    static constexpr std::size_t kBudgetVideo     =  48u * 1024u * 1024u;

    // Unknown / fallback: conservative.
    static constexpr std::size_t kBudgetUnknown   =  16u * 1024u * 1024u;

    // ------------------------------------------------------------------
    // Check() — primary entry point.
    //   formatId  : decoder identifier, e.g. "image/jpeg", "image/raw"
    //   claimed   : bytes the decoder intends to allocate
    // ------------------------------------------------------------------
    [[nodiscard]]
    static BudgetCheckResult Check(std::string_view formatId,
                                   std::size_t      claimed) noexcept
    {
        const std::size_t limit = LimitFor(formatId);
        if (limit == 0u)     return BudgetCheckResult::ZERO_BUDGET;
        if (claimed > limit) return BudgetCheckResult::BUDGET_EXCEEDED;
        return BudgetCheckResult::OK;
    }

    // ------------------------------------------------------------------
    // LimitFor() — returns the budget for a given format ID.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static std::size_t LimitFor(std::string_view formatId) noexcept
    {
        // Camera RAW
        if (formatId == "image/x-raw"  ||
            formatId == "image/x-cr2"  ||
            formatId == "image/x-cr3"  ||
            formatId == "image/x-nef"  ||
            formatId == "image/x-arw"  ||
            formatId == "image/x-dng"  ||
            formatId == "image/x-raf")
            return kBudgetRaw;

        // HDR modern
        if (formatId == "image/heif"   ||
            formatId == "image/heic"   ||
            formatId == "image/avif"   ||
            formatId == "image/jxl"    ||
            formatId == "image/x-exr")
            return kBudgetHdr;

        // Standard raster
        if (formatId == "image/jpeg"   ||
            formatId == "image/png"    ||
            formatId == "image/webp"   ||
            formatId == "image/bmp"    ||
            formatId == "image/tiff"   ||
            formatId == "image/gif"    ||
            formatId == "image/x-tga"  ||
            formatId == "image/x-dds"  ||
            formatId == "image/qoi")
            return kBudgetRaster;

        // PDF
        if (formatId == "application/pdf")
            return kBudgetPdf;

        // Archives
        if (formatId == "application/zip"   ||
            formatId == "application/x-rar" ||
            formatId == "application/x-7z"  ||
            formatId == "application/x-cbz" ||
            formatId == "application/x-cbr")
            return kBudgetArchive;

        // Vector
        if (formatId == "image/svg+xml")
            return kBudgetVector;

        // Icon / font
        if (formatId == "image/x-icon"    ||
            formatId == "image/vnd.microsoft.icon" ||
            formatId == "font/ttf"         ||
            formatId == "font/otf")
            return kBudgetIcon;

        // Video
        if (formatId == "video/mp4"  ||
            formatId == "video/x-matroska" ||
            formatId == "video/x-msvideo" ||
            formatId == "video/quicktime")
            return kBudgetVideo;

        // Fallback for unknown formats
        return kBudgetUnknown;
    }

private:
    PerFormatMemoryBudget() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_PER_FORMAT_MEMORY_BUDGET_H
