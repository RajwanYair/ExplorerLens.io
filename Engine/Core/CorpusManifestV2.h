//==============================================================================
// ExplorerLens Engine — Corpus manifest v2 schema (Sprint S244)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §11 — Corpus 150 files (Phase 1 blocker).
//==============================================================================
//
// Purpose:
//   Strongly-typed view over `data/corpus/MANIFEST.v2.json` entries. v1 stored
//   only (path, sha256). v2 adds:
//       * license    — CC0 / CC-BY / public-domain string
//       * source_url — originating page for attribution
//       * family     — maps to SSIMFormatFamily
//       * min_size   — minimum decoder-rendered thumb dimension for SSIM
//       * notes      — free-form, used by corpus agent
//
// Header-only. JSON parsing lives in `Engine/Core/LensFormatsCommand.h` sibling
// infrastructure; this file defines the in-memory record type.
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "SSIMValidator.h"

namespace ExplorerLens {
namespace Engine {

/// <summary>Licensing class — drives NOTICE file regeneration.</summary>
enum class CorpusLicense : std::uint8_t
{
    UNKNOWN       = 0,
    PUBLIC_DOMAIN = 1,  // no rights reserved / CC0
    CC_BY         = 2,  // attribution required
    CC_BY_SA      = 3,
    LENS_OWNED    = 4,  // synthesised for ExplorerLens
    TEST_FIXTURE  = 5   // engineered / empty / boundary
};

/// <summary>
/// Single corpus entry. Mirrors one row in MANIFEST.v2.json. All string fields
/// are <c>string_view</c>s into the manifest blob owned by the manager — the
/// manager must outlive the entry.
/// </summary>
struct CorpusEntryV2
{
    std::string_view path;          // relative to data/corpus/
    std::string_view sha256;        // 64 hex chars
    std::string_view sourceUrl;     // http(s)://…, empty when LENS_OWNED
    std::string_view notes;         // free-form engineering note
    std::uint64_t    fileBytes   = 0;
    CorpusLicense    license     = CorpusLicense::UNKNOWN;
    SSIMFormatFamily family      = SSIMFormatFamily::GENERIC;
    std::uint32_t    minThumbPx  = 128;
};

/// <summary>Manifest-level stats — emitted to CI summary.</summary>
struct CorpusManifestSummary
{
    std::uint32_t totalFiles    = 0;
    std::uint32_t byFamily[9]   = {};
    std::uint32_t hashMismatches = 0;
    std::uint32_t licenseUnknown = 0;
};

/// <summary>Schema version string.</summary>
inline constexpr std::string_view kCorpusManifestSchema = "lens.corpus-manifest.v2";

/// <summary>Phase-target row counts from ROADMAP §11.</summary>
inline constexpr std::uint32_t kCorpusPhase1Target = 150;
inline constexpr std::uint32_t kCorpusPhase2Target = 300;
inline constexpr std::uint32_t kCorpusPhase3Target = 500;

/// <summary>Helper: reject entries with missing license/source on CI.</summary>
inline bool IsCorpusEntryCompliant(const CorpusEntryV2& e) noexcept
{
    if (e.path.empty() || e.sha256.size() != 64) return false;
    if (e.license == CorpusLicense::UNKNOWN)     return false;
    if (e.license == CorpusLicense::CC_BY || e.license == CorpusLicense::CC_BY_SA)
        return !e.sourceUrl.empty();
    return true;
}

} // namespace Engine
} // namespace ExplorerLens
