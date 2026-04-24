// ============================================================================
// SqliteThumbnailSchema.h -- S251 / ROADMAP v6.0 B2, B3, H15
//
// SQLite L2 thumbnail cache schema scaffolding (Phase 2).  Header-only: keeps
// the schema text + column metadata together so the CI schema-doc generator
// and the runtime can share one source of truth.  The actual SQLite binding
// code (`sqlite3_open_v2`, prepared statements, WAL tuning) lands once the
// Phase 2 sqlite3 vcpkg port is wired in.
//
// Schema version 1.  Bump `kSqliteCacheSchemaVersion` + add an ALTER in the
// migration table below when extending.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>

namespace ExplorerLens::Engine {

inline constexpr uint32_t kSqliteCacheSchemaVersion = 1;

// Column descriptor -- `<name>` / `<sql-type>` pairs describing `thumbnails`.
struct SqliteCacheColumn
{
    const char* name;
    const char* sqlType;
    bool        primaryKey;
    bool        notNull;
};

inline constexpr SqliteCacheColumn kSqliteCacheColumns[] = {
    { "path_hash",   "INTEGER", true,  true  },
    { "thumb_size",  "INTEGER", true,  true  },
    { "format",      "TEXT",    false, true  },
    { "decoder_ver", "INTEGER", false, true  },
    { "width",       "INTEGER", false, true  },
    { "height",      "INTEGER", false, true  },
    { "mtime",       "INTEGER", false, true  },
    { "file_size",   "INTEGER", false, true  },
    { "color_space", "TEXT",    false, false },
    { "phash",       "INTEGER", false, false },
    { "palette",     "BLOB",    false, false },
    { "pixel_data",  "BLOB",    false, true  },
    { "created_at",  "INTEGER", false, true  },
    { "last_access", "INTEGER", false, true  },
};

// Canonical CREATE TABLE statement.  Matches ROADMAP §12 exactly.
inline constexpr const char* kSqliteCacheCreateSql =
    "CREATE TABLE IF NOT EXISTS thumbnails ("
    "  path_hash   INTEGER NOT NULL,"
    "  thumb_size  INTEGER NOT NULL,"
    "  format      TEXT    NOT NULL,"
    "  decoder_ver INTEGER NOT NULL,"
    "  width       INTEGER NOT NULL,"
    "  height      INTEGER NOT NULL,"
    "  mtime       INTEGER NOT NULL,"
    "  file_size   INTEGER NOT NULL,"
    "  color_space TEXT,"
    "  phash       INTEGER,"
    "  palette     BLOB,"
    "  pixel_data  BLOB    NOT NULL,"
    "  created_at  INTEGER NOT NULL,"
    "  last_access INTEGER NOT NULL,"
    "  PRIMARY KEY (path_hash, thumb_size)"
    ") WITHOUT ROWID, STRICT;"
    "CREATE INDEX IF NOT EXISTS idx_last_access ON thumbnails(last_access);"
    "CREATE INDEX IF NOT EXISTS idx_phash       ON thumbnails(phash);";

// Pragmas tuned for a thumbnail cache (WAL + synchronous NORMAL + 64 MiB mmap).
inline constexpr const char* kSqliteCachePragmas =
    "PRAGMA journal_mode=WAL;"
    "PRAGMA synchronous=NORMAL;"
    "PRAGMA foreign_keys=ON;"
    "PRAGMA mmap_size=67108864;"
    "PRAGMA temp_store=MEMORY;";

// Budgets (ROADMAP §12 -- L1 in-memory LRU + L2 on-disk).
inline constexpr size_t kSqliteCacheL1BytesDefault = 64ull  * 1024 * 1024;   // 64 MiB
inline constexpr size_t kSqliteCacheL2BytesDefault = 512ull * 1024 * 1024;   // 512 MiB

struct SqliteCacheSchema
{
    uint32_t    version        = kSqliteCacheSchemaVersion;
    size_t      l1Budget       = kSqliteCacheL1BytesDefault;
    size_t      l2Budget       = kSqliteCacheL2BytesDefault;
    const char* createSql      = kSqliteCacheCreateSql;
    const char* tunePragmas    = kSqliteCachePragmas;
};

// Compile-time sanity checks.
static_assert(sizeof(kSqliteCacheColumns) / sizeof(kSqliteCacheColumns[0]) == 14,
              "SqliteThumbnailSchema: 14 columns expected");
static_assert(kSqliteCacheL1BytesDefault < kSqliteCacheL2BytesDefault,
              "SqliteThumbnailSchema: L1 must be smaller than L2");

} // namespace ExplorerLens::Engine
