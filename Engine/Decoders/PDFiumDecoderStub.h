//==============================================================================
// ExplorerLens Engine — PDFium decoder stub (Sprint S248)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 / §7 — PDF backend: PDFium accelerated to Phase 2.
//==============================================================================
//
// Header-only scaffold declaring the PDFium-backed thumbnail decoder surface.
// Replaces the MuPDF branch that v5.0 had queued for Phase 3. PDFium has:
//   * Better licensing (Apache 2.0 + BSD-3) vs MuPDF AGPL
//   * Active upstream (Chromium)
//   * Native CMake + static-lib build
//
// This file does not link PDFium yet — that lands in S26x when vcpkg triplet
// work is complete. What it DOES do:
//   * Locks the decoder interface shape (Init/Close/Probe/Decode)
//   * Defines a rendering options POD for SSIM-stable output
//   * Declares configuration limits the engine enforces before calling PDFium
//==============================================================================
#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

/// <summary>Rendering strategy for the embedded page.</summary>
enum class PDFRenderMode : std::uint8_t
{
    FIRST_PAGE          = 0,
    LARGEST_EMBEDDED    = 1,   // use biggest embedded thumbnail if present
    FULL_RASTERISATION  = 2
};

/// <summary>Render-options POD — stable across the COM boundary.</summary>
struct PDFDecodeOptions
{
    std::uint32_t  thumbWidthPx  = 256;
    std::uint32_t  thumbHeightPx = 256;
    std::uint32_t  pageIndex     = 0;
    PDFRenderMode  renderMode    = PDFRenderMode::FIRST_PAGE;
    bool           antialias     = true;
    bool           renderForms   = false;
    bool           renderAnnots  = false;
    bool           greyscale     = false;
    std::uint32_t  maxPagesToScan = 8;   // for LARGEST_EMBEDDED
};

/// <summary>Policy limits the engine enforces before PDFium is invoked.</summary>
inline constexpr std::uint64_t kPDFMaxFileBytes    = 512ull * 1024ull * 1024ull;
inline constexpr std::uint32_t kPDFMaxPagesScanned = 64;
inline constexpr std::uint32_t kPDFMaxThumbDim     = 4096;

/// <summary>Initialisation result — used during DLL startup.</summary>
enum class PDFInitStatus : std::uint8_t
{
    UNINITIALISED  = 0,
    READY          = 1,
    LOAD_FAILED    = 2,
    VERSION_MISMATCH = 3,
    DISABLED_BY_POLICY = 4
};

/// <summary>Version reported by PDFium — populated at Init time.</summary>
struct PDFiumVersionInfo
{
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t build = 0;
    std::string_view branch;   // "chromium-xxx"
};

static_assert(std::is_trivially_copyable_v<PDFDecodeOptions>,
              "PDFDecodeOptions must be trivially copyable");

} // namespace Engine
} // namespace ExplorerLens
