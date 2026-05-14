// Engine/Core/CorpusExpansionPhase4.h
// ExplorerLens Engine — S389 (Phase 4, Sprint 9)
//
// Purpose:
//   Phase 4 corpus expansion tracker — monitors progress toward 500 CC0 test files.
//   Phase 4 exit criterion: "Corpus expanded to 500 CC0 files"
//
//   Tracks by family:
//   - RASTER   (JPEG, PNG, BMP, TIFF, WebP, …)   target ≥ 150
//   - CAMERA   (RAW/DNG: CR2, NEF, ARW, …)        target ≥  80
//   - DOCUMENT (PDF, XPS, CBZ, …)                  target ≥  50
//   - MEDIA    (GIF, APNG, ICO, …)                 target ≥  50
//   - VECTOR   (SVG, EMF, WMF)                     target ≥  40
//   - ARCHIVE  (ZIP, 7z, TAR, …)                   target ≥  30
//   - MISC     (STL, glTF, EXR, …)                 target ≥  30
//
//   Builds on CorpusManifestBuilder (S380) — this module provides the Phase 4
//   gap analysis, prioritized download queue, and CI gating.

#pragma once
#ifndef EXPLORERLENS_ENGINE_CORPUSEXPANSIONPHASE4_H
#define EXPLORERLENS_ENGINE_CORPUSEXPANSIONPHASE4_H

#include <cstdint>
#include <cstddef>

namespace ExplorerLens::Engine {

// ─── Corpus families ─────────────────────────────────────────────────────────

enum class CorpusFamily : uint8_t {
    RASTER    = 0,
    CAMERA    = 1,
    DOCUMENT  = 2,
    MEDIA     = 3,
    VECTOR    = 4,
    ARCHIVE   = 5,
    MISC      = 6,
    COUNT     = 7,
};

// ─── Phase 4 gap entry ───────────────────────────────────────────────────────

struct CorpusGapRecord final {
    CorpusFamily family         = CorpusFamily::RASTER;
    uint32_t     currentCount   = 0;
    uint32_t     targetCount    = 0;
    const char*  suggestedUrl   = nullptr;  // best CC0 source for this family

    uint32_t Gap() const noexcept {
        return currentCount < targetCount ? targetCount - currentCount : 0u;
    }
    bool IsCritical() const noexcept { return Gap() >= kCriticalGapThreshold; }
    bool IsMet()      const noexcept { return Gap() == 0; }

    static constexpr uint32_t kCriticalGapThreshold = 20u;
};

// ─── Per-family stats ────────────────────────────────────────────────────────

struct CorpusFamilyStats final {
    CorpusFamily family         = CorpusFamily::RASTER;
    uint32_t     currentCount   = 0;
    uint32_t     targetCount    = 0;
    uint32_t     ccZeroVerified = 0;
    uint32_t     withSsim       = 0;
};

// ─── Phase 4 overall stats ───────────────────────────────────────────────────

struct CorpusPhase4Stats final {
    uint32_t           totalFiles       = 0;
    uint32_t           phase4Target     = 500u;
    uint32_t           ccZeroVerified   = 0;
    uint32_t           withSsimBaseline = 0;
    CorpusFamilyStats  families[static_cast<uint8_t>(CorpusFamily::COUNT)];
    uint32_t           gapCount         = 0;    // families below target

    bool MeetsPhase4Target() const noexcept { return totalFiles >= 500u; }
    float CoveragePercent()  const noexcept {
        if (phase4Target == 0) return 0.0f;
        return static_cast<float>(totalFiles) / static_cast<float>(phase4Target) * 100.0f;
    }
};

// ─── Download queue entry ────────────────────────────────────────────────────

struct CorpusDownloadEntry final {
    char         url[512]       = {};
    char         sha256[65]     = {};   // 64 hex + NUL
    CorpusFamily family         = CorpusFamily::MISC;
    bool         scheduled      = false;
    bool         completed      = false;
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct CorpusExpansionConfig final {
    uint32_t     phase4Target     = 500u;
    uint32_t     downloadParallel = 4u;
    bool         verifyChecksums  = true;
    bool         dryRun           = false;    // log only, do not download

    static constexpr CorpusExpansionConfig Default() noexcept {
        return CorpusExpansionConfig{};
    }

    static constexpr CorpusExpansionConfig CI() noexcept {
        CorpusExpansionConfig c{};
        c.downloadParallel = 1u;
        c.verifyChecksums  = true;
        c.dryRun           = false;
        return c;
    }
};

// ─── Tracker ─────────────────────────────────────────────────────────────────

class CorpusExpansionPhase4 final {
public:
    CorpusExpansionPhase4() = default;
    ~CorpusExpansionPhase4() = default;

    CorpusExpansionPhase4(const CorpusExpansionPhase4&) = delete;
    CorpusExpansionPhase4& operator=(const CorpusExpansionPhase4&) = delete;

    static CorpusExpansionPhase4& Global() noexcept {
        static CorpusExpansionPhase4 s_instance;
        return s_instance;
    }

    void Configure(const CorpusExpansionConfig& config) noexcept { m_config = config; }

    // Analyze current corpus directory and populate stats
    CorpusPhase4Stats Analyze(const char* corpusDir = nullptr) noexcept;

    // Find families below their Phase 4 targets; returns count written to outGaps
    uint32_t FindGaps(CorpusGapRecord* outGaps, uint32_t maxGaps) const noexcept;

    // Queue a file for download (checked by CI gap script)
    bool QueueDownload(const char* url, const char* sha256, CorpusFamily family) noexcept;

    // Run the download queue (max kDownloadQueueMax entries)
    uint32_t ProcessQueue() noexcept;

    uint32_t PendingDownloads() const noexcept { return m_queuePending; }
    uint32_t TotalAdded()       const noexcept { return m_totalAdded; }

    const CorpusExpansionConfig& Config() const noexcept { return m_config; }

private:
    static constexpr uint32_t kDownloadQueueMax = 256u;

    CorpusExpansionConfig  m_config{};
    CorpusDownloadEntry    m_queue[kDownloadQueueMax]{};
    uint32_t               m_queueSize    = 0;
    uint32_t               m_queuePending = 0;
    uint32_t               m_totalAdded   = 0;

    // Per-family current counts (updated by Analyze())
    uint32_t               m_counts[static_cast<uint8_t>(CorpusFamily::COUNT)]{};

    // Phase 4 per-family targets
    static constexpr uint32_t kFamilyTargets[static_cast<uint8_t>(CorpusFamily::COUNT)] = {
        150u, // RASTER
         80u, // CAMERA
         50u, // DOCUMENT
         50u, // MEDIA
         40u, // VECTOR
         30u, // ARCHIVE
         30u, // MISC
    };
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline CorpusPhase4Stats CorpusExpansionPhase4::Analyze(const char* /*corpusDir*/) noexcept {
    CorpusPhase4Stats s{};
    s.phase4Target = m_config.phase4Target;
    uint32_t gap = 0;
    for (uint8_t i = 0; i < static_cast<uint8_t>(CorpusFamily::COUNT); ++i) {
        s.families[i].family       = static_cast<CorpusFamily>(i);
        s.families[i].currentCount = m_counts[i];
        s.families[i].targetCount  = kFamilyTargets[i];
        s.totalFiles += m_counts[i];
        if (m_counts[i] < kFamilyTargets[i]) ++gap;
    }
    s.gapCount = gap;
    return s;
}

inline uint32_t CorpusExpansionPhase4::FindGaps(
    CorpusGapRecord* outGaps, uint32_t maxGaps) const noexcept
{
    if (!outGaps || maxGaps == 0) return 0;
    uint32_t written = 0;
    for (uint8_t i = 0; i < static_cast<uint8_t>(CorpusFamily::COUNT) && written < maxGaps; ++i) {
        if (m_counts[i] < kFamilyTargets[i]) {
            outGaps[written].family       = static_cast<CorpusFamily>(i);
            outGaps[written].currentCount = m_counts[i];
            outGaps[written].targetCount  = kFamilyTargets[i];
            ++written;
        }
    }
    return written;
}

inline bool CorpusExpansionPhase4::QueueDownload(
    const char* url, const char* sha256, CorpusFamily family) noexcept
{
    if (!url || m_queueSize >= kDownloadQueueMax) return false;
    auto& e = m_queue[m_queueSize++];
    uint32_t i = 0;
    while (url[i] && i < 511u) { e.url[i] = url[i]; ++i; } e.url[i] = '\0';
    if (sha256) {
        uint32_t j = 0;
        while (sha256[j] && j < 64u) { e.sha256[j] = sha256[j]; ++j; } e.sha256[j] = '\0';
    }
    e.family    = family;
    e.scheduled = true;
    ++m_queuePending;
    return true;
}

inline uint32_t CorpusExpansionPhase4::ProcessQueue() noexcept {
    uint32_t processed = 0;
    for (uint32_t i = 0; i < m_queueSize; ++i) {
        if (m_queue[i].scheduled && !m_queue[i].completed) {
            // Stub: real impl downloads via WinHTTP
            m_queue[i].completed = true;
            ++m_totalAdded;
            ++processed;
        }
    }
    if (processed > 0) {
        m_queuePending = (m_queuePending >= processed) ? m_queuePending - processed : 0;
    }
    return processed;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kCorpusPhase4TotalTarget       = 500u;
static constexpr uint32_t kCorpusPhase4CriticalGap       = 20u;
static constexpr const char* kCorpusDefaultManifestName  = "MANIFEST.json";

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CORPUSEXPANSIONPHASE4_H
