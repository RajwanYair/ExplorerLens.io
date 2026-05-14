// Engine/Core/CorpusManifestBuilder.h
// ExplorerLens Engine — S380
//
// Purpose:
//   Corpus expansion manifest builder for 300 CC0 test files (Phase 3 target).
//   Phase 3 exit criterion: "Corpus expanded to 300 CC0 files".
//
//   Manages:
//   - MANIFEST.json read/write (each entry: file path, SHA-256, format, ssim baseline)
//   - Gap detection: which decoder families have <N corpus files
//   - CC0 source registry: known public-domain file download sources
//   - Corpus health report: total files, per-family counts, SSIM coverage

#pragma once
#ifndef EXPLORERLENS_ENGINE_CORPUSMANIFESTBUILDER_H
#define EXPLORERLENS_ENGINE_CORPUSMANIFESTBUILDER_H

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Status ──────────────────────────────────────────────────────────────────

enum class CorpusManifestStatus : uint8_t {
    OK                  = 0,
    FILE_NOT_FOUND      = 1,
    PARSE_ERROR         = 2,
    SHA256_MISMATCH     = 3,
    DUPLICATE_ENTRY     = 4,
    IO_ERROR            = 5,
    CORPUS_BELOW_TARGET = 6,
};

// ─── Format family (mirrors FormatFamilyKind from S368) ──────────────────────

enum class CorpusFamily : uint8_t {
    UNKNOWN   = 0,
    IMAGE     = 1,
    MODERN    = 2,
    CAMERA    = 3,
    DOCUMENT  = 4,
    MEDIA     = 5,
};

// ─── Per-file corpus entry ───────────────────────────────────────────────────

struct CorpusEntry final {
    const char*  relativePath   = nullptr;  // e.g. "data/corpus/image/test.png"
    char         sha256[65]     = {};        // 64 hex + NUL
    CorpusFamily family         = CorpusFamily::IMAGE;
    const char*  extension      = nullptr;  // e.g. "png"
    const char*  sourceUrl      = nullptr;  // CC0 download URL
    double       ssimBaseline   = 0.0;
    bool         isCc0          = true;
    bool         hasBaseline    = false;

    bool IsValid() const noexcept {
        if (!relativePath || relativePath[0] == '\0') return false;
        size_t len = 0; while (sha256[len]) ++len;
        return len == 64 && isCc0;
    }
};

// ─── Per-family statistics ───────────────────────────────────────────────────

struct CorpusFamilyStats final {
    CorpusFamily family         = CorpusFamily::UNKNOWN;
    uint32_t     fileCount      = 0;
    uint32_t     targetCount    = 0;
    uint32_t     withBaseline   = 0;
    bool         meetsTarget    = false;

    uint32_t Gap() const noexcept {
        return fileCount >= targetCount ? 0 : targetCount - fileCount;
    }
};

// ─── Corpus health report ────────────────────────────────────────────────────

struct CorpusHealthReport final {
    uint32_t           totalFiles      = 0;
    uint32_t           phase3Target    = 300;
    uint32_t           withBaseline    = 0;
    uint32_t           familyGaps      = 0;
    CorpusFamilyStats  families[6];    // one per CorpusFamily value

    bool MeetsPhase3Target() const noexcept { return totalFiles >= phase3Target; }
    bool IsGreen() const noexcept { return MeetsPhase3Target() && familyGaps == 0; }
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct CorpusManifestConfig final {
    const char*  manifestPath    = "data/corpus/MANIFEST.json";
    uint32_t     phase3Target    = 300;
    uint32_t     perFamilyMin    = 20;   // minimum files per family
    bool         validateSha256  = true;
    bool         warnOnGap       = true;

    static constexpr CorpusManifestConfig Default() noexcept {
        return CorpusManifestConfig{};
    }

    static constexpr CorpusManifestConfig Phase3() noexcept {
        CorpusManifestConfig c{};
        c.phase3Target = 300;
        c.perFamilyMin = 30;
        c.validateSha256 = true;
        return c;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class CorpusManifestBuilder final {
public:
    CorpusManifestBuilder() = default;
    ~CorpusManifestBuilder() = default;

    CorpusManifestBuilder(const CorpusManifestBuilder&) = delete;
    CorpusManifestBuilder& operator=(const CorpusManifestBuilder&) = delete;

    static CorpusManifestBuilder& Global() noexcept {
        static CorpusManifestBuilder s_instance;
        return s_instance;
    }

    void Configure(const CorpusManifestConfig& config) noexcept { m_config = config; }

    // Load existing MANIFEST.json
    CorpusManifestStatus Load(const char* manifestPath = nullptr) noexcept;

    // Add a new corpus entry
    CorpusManifestStatus Add(const CorpusEntry& entry) noexcept;

    // Write updated manifest to disk
    CorpusManifestStatus Save(const char* manifestPath = nullptr) noexcept;

    // Generate health report
    CorpusHealthReport GenerateReport() const noexcept;

    // List families that are below the per-family minimum
    uint32_t GapCount() const noexcept;

    // Total entries loaded
    uint32_t EntryCount() const noexcept { return m_entryCount; }

    // Check if corpus meets Phase 3 target
    bool MeetsPhase3Target() const noexcept { return m_entryCount >= m_config.phase3Target; }

    const CorpusManifestConfig& Config() const noexcept { return m_config; }

private:
    CorpusManifestConfig m_config{};
    uint32_t             m_entryCount = 0;
    bool                 m_loaded     = false;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline CorpusManifestStatus CorpusManifestBuilder::Load(const char* /*path*/) noexcept {
    m_loaded = true;
    return CorpusManifestStatus::OK;
}

inline CorpusManifestStatus CorpusManifestBuilder::Add(const CorpusEntry& entry) noexcept {
    if (!entry.IsValid()) return CorpusManifestStatus::PARSE_ERROR;
    ++m_entryCount;
    return CorpusManifestStatus::OK;
}

inline CorpusManifestStatus CorpusManifestBuilder::Save(const char* /*path*/) noexcept {
    return CorpusManifestStatus::OK;
}

inline CorpusHealthReport CorpusManifestBuilder::GenerateReport() const noexcept {
    CorpusHealthReport r{};
    r.totalFiles   = m_entryCount;
    r.phase3Target = m_config.phase3Target;
    // populate per-family stats stub
    for (uint8_t i = 0; i < 6; ++i) {
        r.families[i].family       = static_cast<CorpusFamily>(i);
        r.families[i].targetCount  = m_config.perFamilyMin;
    }
    return r;
}

inline uint32_t CorpusManifestBuilder::GapCount() const noexcept {
    auto r = GenerateReport();
    uint32_t gaps = 0;
    for (const auto& f : r.families) {
        if (!f.meetsTarget && f.targetCount > 0) ++gaps;
    }
    return gaps;
}

// ─── CC0 Source URLs (known public domain archives) ──────────────────────────

static constexpr const char* kCorpusSourcePixabay     = "https://pixabay.com/";
static constexpr const char* kCorpusSourceUnsplash     = "https://unsplash.com/";
static constexpr const char* kCorpusSourceOpenverse    = "https://openverse.org/";
static constexpr const char* kCorpusSourceRawPixels    = "https://www.rawpixel.com/";
static constexpr const char* kCorpusSourceWikiCommons  = "https://commons.wikimedia.org/";

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kCorpusPhase2Target  = 150u;
static constexpr uint32_t kCorpusPhase3Target  = 300u;
static constexpr uint32_t kCorpusPhase4Target  = 500u;
static constexpr uint32_t kCorpusPhase6Target  = 750u;
static constexpr uint32_t kCorpusPerFamilyMin  = 20u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CORPUSMANIFESTBUILDER_H
