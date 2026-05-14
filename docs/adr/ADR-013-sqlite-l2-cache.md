# ADR-013: SQLite WAL for L2 Thumbnail Cache Index

**Status:** Accepted
**Version:** v38.4.0 (scheduled for Phase 2 implementation)
**Date:** 2026-04-23
**ROADMAP Reference:** §7.5, §14, Decision D42, Harvested Practice H5

## Context

ExplorerLens currently has no persistent thumbnail cache. Every `IThumbnailProvider::GetThumbnail()`
call re-decodes the source file, even if the thumbnail was generated 1 second ago in another
Explorer window. This causes:

- **Repeated CPU work** for the same file at the same size
- **No cache-hit benefit** from navigating back to a previously-visited folder
- **No size budget enforcement** — unbounded per-session memory growth
- **No invalidation** — even after file modification, old results are not refreshed

macOS Quick Look (see §3.1, H5) uses an SQLite-indexed thumbnail cache with `FSEvents` invalidation.
This architecture has been validated at scale (millions of cache entries per Mac user) and is the
industry reference for this problem.

## Decision

Implement a **two-tier cache** in `Engine/Cache/`:

### L1 — In-process LRU (Phase 1, already partially implemented)

| Property | Value |
| ---------- | ------- |
| Storage | In-process memory |
| Data structure | Robin-Hood open-addressing hashmap + doubly-linked LRU list |
| Key | `XXH3_128(canonical_path ‖ mtime_ns ‖ size_bytes ‖ target_w ‖ target_h ‖ decoder_ver)` |
| Budget | 64 MB (configurable via registry `MaxL1CacheMB`) |
| Hit-path P50 | < 500 μs |
| Eviction | LRU; also evict on `ReadDirectoryChangesW` file-change notification |
| Thread safety | Reader-writer lock (`std::shared_mutex`); readers take shared lock |

### L2 — SQLite WAL index + memory-mapped blob store (Phase 2)

| Property | Value |
| ---------- | ------- |
| Index location | `%LOCALAPPDATA%\ExplorerLens\cache.db` |
| Blob location | `%LOCALAPPDATA%\ExplorerLens\Cache\<hash_prefix>\<full_hash>.thumb` |
| SQLite mode | WAL (Write-Ahead Logging) — allows concurrent readers without blocking |
| Budget default | 1 GB (configurable via registry `MaxL2CacheMB`) |
| Hit-path P50 | < 5 ms |
| Invalidation | `ReadDirectoryChangesW` watcher on source directories; mtime+size check on L2 read |

### Cache key schema

```text
CacheKey = XXH3_128(
  NormalizedAbsolutePath(file),   // e.g., "C:\Users\...\photo.jpg" → lowercased, long path form
  file_mtime_epoch_ns,             // 100ns resolution NTFS timestamps
  file_size_bytes,
  uint32_t target_w,
  uint32_t target_h,
  uint16_t decoder_version         // bump when decoder output changes
)
```

### SQLite schema

```sql
CREATE TABLE thumbnails (
    key        BLOB    PRIMARY KEY,    -- 16-byte XXH3 hash
    path_hash  BLOB    NOT NULL,       -- 8-byte hash of source path (for cleanup)
    source_mtime INTEGER NOT NULL,     -- source file mtime (ns)
    source_size INTEGER NOT NULL,      -- source file size (bytes)
    target_w   INTEGER NOT NULL,
    target_h   INTEGER NOT NULL,
    blob_path  TEXT    NOT NULL,       -- relative path under Cache\
    blob_size  INTEGER NOT NULL,
    ssim       REAL,                   -- SSIM vs. reference (from corpus validation)
    created_at INTEGER NOT NULL,       -- unix epoch seconds
    decoder_ver INTEGER NOT NULL
) STRICT;

CREATE INDEX idx_path_hash ON thumbnails(path_hash);
CREATE INDEX idx_created   ON thumbnails(created_at);
```

### Blob storage

Each thumbnail blob is a raw BGRA32 pixel buffer (width × height × 4 bytes), stored as a
memory-mapped file. Large blobs (> 1 MB, i.e., thumbnails > 512×512) use LZ4 compression.

### Invalidation flow

```text
1. ReadDirectoryChangesW fires for directory D
2. For each changed file F in D:
   a. Compute path_hash(F)
   b. DELETE FROM thumbnails WHERE path_hash = ? AND source_mtime != current_mtime(F)
   c. Delete orphaned .thumb blobs
3. L1 cache evicts matching keys immediately (lock-free bitmap invalidation)
```

### Cache compaction

Scheduled daily via a Windows Task (registered by MSI installer):

```powershell
lens.exe cache compact --max-age 30d --max-size 1GB
```

The `lens.exe cache` commands (§6.3 ROADMAP) drive all cache management.

## Rationale

- **SQLite WAL** allows multiple Explorer windows (multiple `IThumbnailProvider` instances) to
  read concurrently without locking — critical for `explorer.exe` concurrency model
- **File-based blobs** avoid SQLite page-size limitations for binary data; SQLite is index-only
- **XXH3** for cache keys: 3–5× faster than SHA-256; collision probability negligible for this use case
- **LevelDB rejected** (Decision D42): LevelDB uses a process-exclusive lock, breaking multi-process scenarios
- **Custom format rejected**: SQLite is crash-safe (WAL + checkpointing), humans can `sqlite3` it for debugging
- **In-process-only rejected**: Cache dies on process exit; `explorer.exe` restarts frequently during development
- **Matches macOS Quick Look architecture [H5]**: validated at scale

## Consequences

### Positive

- Cache hit P50 < 5 ms (vs. re-decode cost of 5–150 ms for P0 formats)
- Multiple Explorer windows share the same L2 — folder thumbnail grids appear instantaneously on revisit
- `lens.exe cache stats` gives visibility into hit rate / size / top-100 paths
- Corpus validation tests can validate cache consistency (decode → store → retrieve → SSIM)

### Negative

- SQLite dependency added to `Engine/Cache/` (header-only is insufficient; link `sqlite3.c` as Unity build)
- `%LOCALAPPDATA%` writes from inside `explorer.exe` require careful path resolution (no `GetTempPath`)
- WAL checkpoint must be triggered periodically from a background thread to reclaim disk space
- Upgrade migrations require schema versioning (`PRAGMA user_version`)

## Implementation Notes

- Link SQLite as the single-file amalgamation (`sqlite3.c` + `sqlite3.h`) — no external dependency
- Build with `SQLITE_THREADSAFE=1`, `SQLITE_DEFAULT_WAL_AUTOCHECKPOINT=1000`, `SQLITE_ENABLE_STAT4`
- Use `PRAGMA synchronous = NORMAL` inside `explorer.exe` (safe with WAL; full is too slow)
- The `CacheProvider` class in `Engine/Cache/CacheProvider.h` is the single owner of the SQLite connection

## Alternatives Considered

| Alternative | Why rejected |
| ------------- | ------------- |
| LevelDB | Process-exclusive lock; breaks multi-Explorer concurrency |
| LMDB | Good, but less battle-tested on Windows; no human-readable debugging |
| RocksDB | Heavyweight; server-oriented |
| In-memory only | Ephemeral; no cross-session benefit |
| Windows Shell thumbnail cache (`%LOCALAPPDATA%\Microsoft\Windows\Explorer`) | Not accessible via documented API; subject to Windows policy |
| Registry blob storage | 64 KB value limit per key; wrong tool |
