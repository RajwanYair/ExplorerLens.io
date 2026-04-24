//==============================================================================
// ExplorerLens Engine — SSIM validator (Sprint S243)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §11 — SSIM CI automation (Phase 1 blocker).
//==============================================================================
//
// Purpose:
//   Golden-image regression gate. After a decoder runs against a corpus file,
//   compare its output to the stored baseline thumbnail with SSIM. Scores
//   below the format-specific threshold fail the gate.
//
// Header-only scaffold. The SSIM kernel itself lives in
// `Engine/AI/ImageQualityAssessor*.h` (already registered). This header is
// the _policy_ layer used by CI:
//
//   * Minimum-score thresholds per format family
//   * Structured result record the test runner serialises to JSON
//   * Gate decision with explainable reason codes
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Format families with their own SSIM threshold. JPEG is lossy so we allow
/// looser scores; PNG/BMP are pixel-exact so we demand near-perfect match.
/// </summary>
enum class SSIMFormatFamily : std::uint8_t
{
    GENERIC   = 0,
    JPEG      = 1,
    PNG       = 2,
    WEBP      = 3,
    AVIF_HEIF = 4,
    RAW       = 5,
    PDF       = 6,
    VECTOR    = 7,  // SVG / PDF vector
    THREE_D   = 8   // STL / glTF / OBJ
};

/// <summary>
/// Default thresholds tuned from Session 9 corpus data. Values are the
/// _minimum acceptable_ SSIM score (range [0.0, 1.0], higher is better).
/// </summary>
inline constexpr double SSIMThresholdFor(SSIMFormatFamily family) noexcept
{
    switch (family)
    {
        case SSIMFormatFamily::JPEG:      return 0.92;
        case SSIMFormatFamily::PNG:       return 0.99;
        case SSIMFormatFamily::WEBP:      return 0.93;
        case SSIMFormatFamily::AVIF_HEIF: return 0.90;
        case SSIMFormatFamily::RAW:       return 0.85;  // tonemap varies
        case SSIMFormatFamily::PDF:       return 0.88;
        case SSIMFormatFamily::VECTOR:    return 0.80;  // rasterisation diffs
        case SSIMFormatFamily::THREE_D:   return 0.75;  // view-angle sensitive
        default:                          return 0.90;
    }
}

/// <summary>Reason a single validation entry passed or failed.</summary>
enum class SSIMGateReason : std::uint8_t
{
    PASS                = 0,
    FAIL_BELOW_THRESHOLD = 1,
    FAIL_DIMENSION_MISMATCH = 2,
    FAIL_DECODE_ERROR   = 3,
    FAIL_MISSING_BASELINE = 4,
    SKIP_CORPUS_MISSING = 5
};

/// <summary>Single validation record — one corpus file, one decoder output.</summary>
struct SSIMValidationRecord
{
    std::string_view  corpusPath;          // relative to data/corpus/
    std::string_view  baselinePath;        // relative to data/baselines/
    SSIMFormatFamily  family = SSIMFormatFamily::GENERIC;
    double            score = 0.0;
    double            threshold = 0.0;
    SSIMGateReason    reason = SSIMGateReason::FAIL_DECODE_ERROR;
    std::uint32_t     widthPx = 0;
    std::uint32_t     heightPx = 0;
    std::uint64_t     decodeDurationUs = 0;
};

/// <summary>Aggregated gate verdict returned to the CI runner.</summary>
struct SSIMGateSummary
{
    std::uint32_t totalFiles   = 0;
    std::uint32_t passed       = 0;
    std::uint32_t failed       = 0;
    std::uint32_t skipped      = 0;
    double        worstScore   = 1.0;
    double        meanScore    = 1.0;
};

/// <summary>Apply the threshold policy to a raw score.</summary>
inline SSIMGateReason EvaluateSSIMScore(double score,
                                        SSIMFormatFamily family,
                                        bool baselinePresent) noexcept
{
    if (!baselinePresent)             return SSIMGateReason::FAIL_MISSING_BASELINE;
    if (score < 0.0 || score > 1.0)   return SSIMGateReason::FAIL_DECODE_ERROR;
    return (score >= SSIMThresholdFor(family))
        ? SSIMGateReason::PASS
        : SSIMGateReason::FAIL_BELOW_THRESHOLD;
}

/// <summary>Schema version string emitted into CI JSON — bump on breaking change.</summary>
inline constexpr std::string_view kSSIMValidationSchema = "lens.ssim-gate.v1";

} // namespace Engine
} // namespace ExplorerLens
