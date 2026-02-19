#pragma once
// Sprint 168 — Hot-Mode Directory Engine
// Background index, change notification watcher, pre-warm thumbnail generation
// for directories with > 50 images (Explorer hot paths).

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace DarkThumbs::Memory {

// ─── Hot-mode thresholds ─────────────────────────────────────────────────────

struct HotModeThresholds {
    uint32_t    minFilesForHotMode  { 50 };   // directory qualifies at >= 50 images
    uint32_t    maxConcurrentPreWarm{ 4 };    // pre-warm worker count
    uint32_t    prewarmBatchSize    { 16 };   // files per pre-warm batch
    uint32_t    indexRefreshSecs    { 30 };   // re-index if directory is older than this
    size_t      vramBudgetBytes     { 128ULL * 1024 * 1024 };  // 128 MB hot-mode budget

    static HotModeThresholds Default() { return {}; }

    static HotModeThresholds AggressiveLowPower() {
        HotModeThresholds t;
        t.minFilesForHotMode    = 100;
        t.maxConcurrentPreWarm  = 2;
        t.prewarmBatchSize      = 8;
        return t;
    }
};

// ─── Directory index entry ───────────────────────────────────────────────────

struct DirectoryIndexEntry {
    std::string     filePath;
    uint64_t        sizeBytes       { 0 };
    uint64_t        lastModified    { 0 };  // FILETIME epoch
    bool            thumbnailCached { false };
    bool            prewarmPending  { false };
};

// ─── Directory snapshot ───────────────────────────────────────────────────────

struct DirectorySnapshot {
    std::string                         directoryPath;
    std::vector<DirectoryIndexEntry>    entries;
    uint64_t                            indexTimestamp  { 0 };
    bool                                hotMode         { false };

    uint32_t TotalFiles() const { return static_cast<uint32_t>(entries.size()); }
    uint32_t CachedCount() const {
        uint32_t n = 0;
        for (const auto& e : entries) if (e.thumbnailCached) ++n;
        return n;
    }

    double CacheHitRate() const {
        return TotalFiles() > 0 ? 100.0 * CachedCount() / TotalFiles() : 0.0;
    }
};

// ─── Change notification ─────────────────────────────────────────────────────

enum class DirChangeType : uint32_t {
    FileAdded   = 0,
    FileRemoved = 1,
    FileChanged = 2,
    DirRenamed  = 3,
};

struct DirChangeEvent {
    DirChangeType   type;
    std::string     path;
    uint64_t        timestamp { 0 };
};

// ─── Pre-warm result ─────────────────────────────────────────────────────────

struct PreWarmBatchResult {
    uint32_t    filesAttempted  { 0 };
    uint32_t    filesSucceeded  { 0 };
    uint32_t    filesFailed     { 0 };
    double      totalMs         { 0.0 };
    size_t      bytesUsed       { 0 };

    double AvgMsPerFile() const {
        return filesAttempted > 0 ? totalMs / filesAttempted : 0.0;
    }
};

// ─── Hot-mode directory engine ───────────────────────────────────────────────

class HotModeDirectoryEngine {
public:
    using ChangeCallback = std::function<void(const DirChangeEvent&)>;

    explicit HotModeDirectoryEngine(HotModeThresholds thresholds = HotModeThresholds::Default())
        : m_thresholds(std::move(thresholds)) {}

    bool IsHotModeDirectory(const DirectorySnapshot& snap) const {
        return snap.TotalFiles() >= m_thresholds.minFilesForHotMode;
    }

    DirectorySnapshot IndexDirectory(const std::string& path) const {
        DirectorySnapshot snap;
        snap.directoryPath = path;
        snap.indexTimestamp = 0;  // stub: real impl calls FindFirstFileEx
        snap.hotMode = false;
        return snap;
    }

    PreWarmBatchResult PreWarmBatch(DirectorySnapshot& snap,
                                    uint32_t startIndex,
                                    uint32_t count) const {
        PreWarmBatchResult result;
        uint32_t end = startIndex + count;
        if (end > snap.TotalFiles()) end = snap.TotalFiles();

        for (uint32_t i = startIndex; i < end; ++i) {
            ++result.filesAttempted;
            // stub: real impl calls DarkThumbsEngine::DecodeThumbnail
            snap.entries[i].thumbnailCached = true;
            snap.entries[i].prewarmPending  = false;
            result.bytesUsed += 4 * 256 * 256;  // 256 KB per thumbnail BGRA estimate
            ++result.filesSucceeded;
        }
        result.totalMs = result.filesAttempted * 5.0;  // 5 ms/file estimate
        return result;
    }

    void RegisterChangeCallback(ChangeCallback cb) {
        m_changeCallback = std::move(cb);
    }

    const HotModeThresholds& Thresholds() const { return m_thresholds; }

private:
    HotModeThresholds   m_thresholds;
    ChangeCallback      m_changeCallback;
};

} // namespace DarkThumbs::Memory
