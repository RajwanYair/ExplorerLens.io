// Engine/Cache/L2SqliteCacheIndex.h
// ExplorerLens — SQLite WAL-mode L2 disk cache index (ROADMAP v8.0 Phase 2 — Cache §8.7)
// Sprint S338.
//
// Purpose:
//   Phase 2 completes the L2 cache layer described in §8.7 of the ROADMAP:
//
//     L2 — SQLite BLOB + PNG in %LOCALAPPDATA%\ExplorerLens\cache\
//          512 MB max, LRU eviction by last_hit
//
//   L2SqliteCacheIndex provides:
//     1. The schema definition (CREATE TABLE SQL) that matches docs/SBOM.json §11
//     2. Key types: CacheIndexEntry, SqliteIndexConfig, SqliteCacheQueryResult
//     3. A stub interface with Insert(), Lookup(), Evict(), PragmaStrings()
//
//   The full SQLite implementation (opening the DB, WAL mode, prepared statements)
//   is deferred to Phase 3.  This header completes the contract so the async cache
//   writer (AsyncCacheWriter, S325) has a typed target to call.
//
//   Schema (matches §11 in ROADMAP):
//     thumbnail_cache (id, file_path, file_mtime, file_size, phash, thumb_path,
//                      width, height, decoder_id, decode_ms, created_at, last_hit)
//     decode_errors   (id, file_ext, decoder_id, error_code, error_msg, created_at)
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_L2_SQLITE_CACHE_INDEX_H
#define EXPLORERLENS_ENGINE_L2_SQLITE_CACHE_INDEX_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <string>
#include <optional>
#include <array>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// CacheIndexEntry — one row in the thumbnail_cache table
// ---------------------------------------------------------------------------
struct CacheIndexEntry final {
    std::int64_t  id{};
    std::wstring  filePath;
    std::int64_t  fileMtime{};       ///< Unix timestamp (seconds)
    std::int64_t  fileSize{};        ///< Bytes
    std::uint64_t phash{};           ///< 8-byte perceptual hash (0 = uncalculated)
    std::wstring  thumbPath;         ///< Path to PNG file in cache dir
    std::uint32_t width{};
    std::uint32_t height{};
    std::string   decoderId;         ///< e.g., "libjpeg-turbo", "libheif"
    std::int32_t  decodeMs{};
    std::int64_t  createdAt{};       ///< Unix timestamp
    std::int64_t  lastHit{};         ///< Unix timestamp of last cache hit
    bool          valid{ false };
};

// ---------------------------------------------------------------------------
// SqliteCacheQueryResult
// ---------------------------------------------------------------------------
enum class SqliteCacheQueryResult : std::uint8_t {
    HIT            = 0,  ///< Entry found and returned
    MISS           = 1,  ///< No entry for this file path / mtime
    STALE          = 2,  ///< Entry exists but mtime has changed
    DB_NOT_OPEN    = 3,  ///< SQLite DB not yet initialised
    DB_ERROR       = 4,  ///< SQLite API returned an error
    PATH_EMPTY     = 5,  ///< Empty file path supplied
};

// ---------------------------------------------------------------------------
// SqliteIndexConfig
// ---------------------------------------------------------------------------
struct SqliteIndexConfig final {
    /// Default maximum cache size in bytes (512 MB)
    static constexpr std::int64_t kDefaultMaxBytes = 512LL * 1024 * 1024;
    /// SQLite page cache size (-8000 = 8 MB in pages)
    static constexpr int kCacheSizePages = -8000;
    /// Memory-mapped I/O (64 MB)
    static constexpr std::int64_t kMmapSize = 64LL * 1024 * 1024;

    std::wstring  dbPath;                         ///< Path to catalog.db file
    std::int64_t  maxCacheBytes = kDefaultMaxBytes;
    std::int32_t  maxEntries    = 100000;          ///< LRU eviction at this count
    bool          readOnly      = false;
};

// ---------------------------------------------------------------------------
// L2SqliteCacheIndex
// ---------------------------------------------------------------------------
class L2SqliteCacheIndex final {
public:
    /// Schema SQL strings (used by Initialise() to CREATE TABLE IF NOT EXISTS)
    static constexpr std::string_view kCreateThumbnailCacheTable =
        "CREATE TABLE IF NOT EXISTS thumbnail_cache ("
        "  id         INTEGER PRIMARY KEY,"
        "  file_path  TEXT    NOT NULL,"
        "  file_mtime INTEGER NOT NULL,"
        "  file_size  INTEGER NOT NULL,"
        "  phash      BLOB,"
        "  thumb_path TEXT,"
        "  width      INTEGER,"
        "  height     INTEGER,"
        "  decoder_id TEXT,"
        "  decode_ms  INTEGER,"
        "  created_at INTEGER DEFAULT (unixepoch()),"
        "  last_hit   INTEGER DEFAULT (unixepoch())"
        ");";

    static constexpr std::string_view kCreateDecodeErrorsTable =
        "CREATE TABLE IF NOT EXISTS decode_errors ("
        "  id         INTEGER PRIMARY KEY,"
        "  file_ext   TEXT    NOT NULL,"
        "  decoder_id TEXT    NOT NULL,"
        "  error_code INTEGER,"
        "  error_msg  TEXT,"
        "  created_at INTEGER DEFAULT (unixepoch())"
        ");";

    static constexpr std::string_view kCreatePathIndex =
        "CREATE INDEX IF NOT EXISTS idx_path  ON thumbnail_cache(file_path);";

    static constexpr std::string_view kCreateMtimeIndex =
        "CREATE INDEX IF NOT EXISTS idx_mtime ON thumbnail_cache(file_mtime);";

    static constexpr std::string_view kCreateErrExtIndex =
        "CREATE INDEX IF NOT EXISTS idx_ext   ON decode_errors(file_ext);";

    /// WAL mode + synchronous=NORMAL pragma sequence (run after DB open)
    static constexpr std::array<std::string_view, 4> kWalPragmas = {{
        "PRAGMA journal_mode = WAL;",
        "PRAGMA synchronous  = NORMAL;",
        "PRAGMA cache_size   = -8000;",
        "PRAGMA mmap_size    = 67108864;",
    }};

    // ------------------------------------------------------------------
    // Public interface (Phase 3 implementation stubs)
    // ------------------------------------------------------------------

    explicit L2SqliteCacheIndex(SqliteIndexConfig cfg = {}) noexcept
        : m_cfg(std::move(cfg)), m_isOpen(false) {}

    ~L2SqliteCacheIndex() noexcept { Close(); }

    // Non-copyable
    L2SqliteCacheIndex(const L2SqliteCacheIndex&)            = delete;
    L2SqliteCacheIndex& operator=(const L2SqliteCacheIndex&) = delete;

    // Open() — Phase 3: opens SQLite DB, runs WAL pragmas, creates tables
    [[nodiscard]]
    bool Open() noexcept
    {
        // Phase 3 implementation: sqlite3_open_v2 → pragmas → CREATE TABLE IF NOT EXISTS
        m_isOpen = false;
        return false;  // stub
    }

    void Close() noexcept
    {
        // Phase 3 implementation: sqlite3_close_v2
        m_isOpen = false;
    }

    [[nodiscard]] bool IsOpen() const noexcept { return m_isOpen; }

    // Lookup() — Phase 3: SELECT WHERE file_path=? AND file_mtime=?
    [[nodiscard]]
    SqliteCacheQueryResult Lookup(std::wstring_view  filePath,
                                  std::int64_t       fileMtime,
                                  CacheIndexEntry&   out) const noexcept
    {
        (void)filePath; (void)fileMtime; (void)out;
        if (!m_isOpen) return SqliteCacheQueryResult::DB_NOT_OPEN;
        return SqliteCacheQueryResult::MISS;  // stub
    }

    // Insert() — Phase 3: INSERT OR REPLACE INTO thumbnail_cache
    [[nodiscard]]
    SqliteCacheQueryResult Insert(const CacheIndexEntry& entry) noexcept
    {
        (void)entry;
        if (!m_isOpen) return SqliteCacheQueryResult::DB_NOT_OPEN;
        return SqliteCacheQueryResult::DB_ERROR;  // stub
    }

    // EvictLRU() — Phase 3: DELETE WHERE id IN (SELECT id ... ORDER BY last_hit)
    std::uint32_t EvictLRU(std::uint32_t evictCount) noexcept
    {
        (void)evictCount;
        return 0u;  // stub
    }

    // ------------------------------------------------------------------
    // Constants for unit test assertions
    // ------------------------------------------------------------------
    [[nodiscard]] const SqliteIndexConfig& Config() const noexcept { return m_cfg; }
    static constexpr std::int64_t kDefaultMaxBytes = SqliteIndexConfig::kDefaultMaxBytes;
    static constexpr std::int32_t kCacheSizePages  = SqliteIndexConfig::kCacheSizePages;

private:
    SqliteIndexConfig m_cfg;
    bool              m_isOpen;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_L2_SQLITE_CACHE_INDEX_H
