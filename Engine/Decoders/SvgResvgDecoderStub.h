// ============================================================================
// SvgResvgDecoderStub.h -- S264 / ROADMAP v6.0 L8 (resvg replacement)
//
// Evaluation contract for replacing the current NanoSVG-based SVGRasterizer
// with Rust's resvg (linked via C ABI) which has far better spec coverage
// (masks, filters, text-on-path) and matches Chromium output.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class SvgResvgBackend : uint8_t
{
    NANOSVG_LEGACY  = 0,   // current v39.9.0
    RESVG_20        = 1,   // resvg 0.44.x (2026)
    RESVG_SKIA      = 2,   // resvg Skia backend (experimental)
};

enum class SvgResvgStatus : uint8_t
{
    OK                      = 0,
    PARSE_ERROR             = 1,
    UNSUPPORTED_FILTER      = 2,
    UNSUPPORTED_FONT        = 3,
    XLINK_LOOP_DETECTED     = 4,
    BUDGET_EXCEEDED         = 5,
    EXTERNAL_RESOURCE_BLOCKED = 6,   // <image href="http://..."> blocked by policy
};

struct SvgResvgFitBox
{
    uint32_t width   = 0;
    uint32_t height  = 0;
    bool     preserveAspect = true;
};

struct SvgResvgDecodeOptions
{
    SvgResvgBackend backend            = SvgResvgBackend::RESVG_20;
    SvgResvgFitBox  fit                = {};
    float           dpi                = 96.0f;
    bool            allowExternalRefs  = false;  // security: never during thumbnail
    bool            allowScripts       = false;  // <script> must be stripped
    bool            antialias          = true;
    uint32_t        budgetMs           = 120;
};

struct SvgResvgProbeResult
{
    SvgResvgStatus status       = SvgResvgStatus::OK;
    uint32_t       viewBoxWidth  = 0;
    uint32_t       viewBoxHeight = 0;
    uint32_t       filterCount   = 0;
    uint32_t       textCount     = 0;
    bool           hasExternalRefs = false;
    bool           hasScripts    = false;
};

// Refuse absurdly large SVGs before handing them to resvg.
inline constexpr uint32_t kSvgResvgMaxSourceBytes    = 64u * 1024u * 1024u;  // 64 MiB
inline constexpr uint32_t kSvgResvgMaxViewBoxSide    = 16384;
inline constexpr uint32_t kSvgResvgDefaultBudgetMs   = 120;
inline constexpr uint32_t kSvgResvgHardBudgetMs      = 500;

static_assert(std::is_trivially_copyable_v<SvgResvgDecodeOptions>,
              "SvgResvgDecodeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SvgResvgProbeResult>,
              "SvgResvgProbeResult must be trivially copyable");
static_assert(kSvgResvgDefaultBudgetMs < kSvgResvgHardBudgetMs,
              "SVG default budget < hard budget");

} // namespace ExplorerLens::Engine
