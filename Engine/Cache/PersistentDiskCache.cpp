//==============================================================================
// PersistentDiskCache.cpp
// Persistent disk cache — L1 in-memory map + L2 SQLite (winsqlite3.dll)
// Sprint S302: Implement real SQLite L2 cache (ADR A40 — Phase 1 P0)
//==============================================================================

#include "PersistentDiskCache.h"
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// winsqlite3.dll dynamic loading (Windows 10+ built-in — zero external dep)
//==============================================================================
namespace {

// Minimal sqlite3 opaque type aliases for dynamic loading
using sqlite3_t      = void;
using sqlite3_stmt_t = void;

// Function pointer types (matching sqlite3 public API signatures)
using pfn_open_v2        = int(*)(const char*, sqlite3_t**, int, const char*);
using pfn_close          = int(*)(sqlite3_t*);
using pfn_exec           = int(*)(sqlite3_t*, const char*, int(*)(void*,int,char**,char**), void*, char**);
using pfn_prepare_v2     = int(*)(sqlite3_t*, const char*, int, sqlite3_stmt_t**, const char**);
using pfn_step           = int(*)(sqlite3_stmt_t*);
using pfn_finalize       = int(*)(sqlite3_stmt_t*);
using pfn_reset          = int(*)(sqlite3_stmt_t*);
using pfn_bind_text      = int(*)(sqlite3_stmt_t*, int, const char*, int, void(*)(void*));
using pfn_bind_int64     = int(*)(sqlite3_stmt_t*, int, long long);
using pfn_bind_double    = int(*)(sqlite3_stmt_t*, int, double);
using pfn_bind_blob      = int(*)(sqlite3_stmt_t*, int, const void*, int, void(*)(void*));
using pfn_column_int     = int(*)(sqlite3_stmt_t*, int);
using pfn_column_int64   = long long(*)(sqlite3_stmt_t*, int);
using pfn_column_text    = const unsigned char*(*)(sqlite3_stmt_t*, int);
using pfn_column_bytes   = int(*)(sqlite3_stmt_t*, int);
using pfn_column_blob    = const void*(*)(sqlite3_stmt_t*, int);
using pfn_column_double  = double(*)(sqlite3_stmt_t*, int);
using pfn_errmsg         = const char*(*)(sqlite3_t*);
using pfn_free           = void(*)(void*);

// SQLITE constants (avoid including sqlite3.h)
constexpr int WSQL_OK            = 0;
constexpr int WSQL_ROW           = 100;
constexpr int WSQL_DONE          = 101;
constexpr int WSQL_OPEN_RW_CR    = 0x00000006; // SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE
// SQLITE_TRANSIENT = special value: cast of -1
static void (* const WSQL_TRANSIENT)(void*) = reinterpret_cast<void(*)(void*)>(static_cast<intptr_t>(-1));

struct WinSqlite3 {
    HMODULE          hDll           = nullptr;
    pfn_open_v2      open_v2        = nullptr;
    pfn_close        close_         = nullptr;
    pfn_exec         exec           = nullptr;
    pfn_prepare_v2   prepare_v2     = nullptr;
    pfn_step         step           = nullptr;
    pfn_finalize     finalize       = nullptr;
    pfn_reset        reset          = nullptr;
    pfn_bind_text    bind_text      = nullptr;
    pfn_bind_int64   bind_int64     = nullptr;
    pfn_bind_double  bind_double    = nullptr;
    pfn_bind_blob    bind_blob      = nullptr;
    pfn_column_int   column_int     = nullptr;
    pfn_column_int64 column_int64   = nullptr;
    pfn_column_text  column_text    = nullptr;
    pfn_column_bytes column_bytes   = nullptr;
    pfn_column_blob  column_blob    = nullptr;
    pfn_column_double column_double = nullptr;
    pfn_errmsg       errmsg         = nullptr;
    pfn_free         free_          = nullptr;
    bool             loaded         = false;
};

WinSqlite3& Sql() {
    static WinSqlite3 s_sql;
    return s_sql;
}

#define LOAD_SQL_FN(sym, field) \
    s.field = reinterpret_cast<decltype(s.field)>(GetProcAddress(s.hDll, (sym))); \
    if (!s.field) { FreeLibrary(s.hDll); s.hDll = nullptr; return false; }

bool LoadWinSqlite3() {
    WinSqlite3& s = Sql();
    if (s.loaded) return true;
    s.hDll = LoadLibraryW(L"winsqlite3.dll");
    if (!s.hDll) return false;
    LOAD_SQL_FN("sqlite3_open_v2",     open_v2)
    LOAD_SQL_FN("sqlite3_close",       close_)
    LOAD_SQL_FN("sqlite3_exec",        exec)
    LOAD_SQL_FN("sqlite3_prepare_v2",  prepare_v2)
    LOAD_SQL_FN("sqlite3_step",        step)
    LOAD_SQL_FN("sqlite3_finalize",    finalize)
    LOAD_SQL_FN("sqlite3_reset",       reset)
    LOAD_SQL_FN("sqlite3_bind_text",   bind_text)
    LOAD_SQL_FN("sqlite3_bind_int64",  bind_int64)
    LOAD_SQL_FN("sqlite3_bind_double", bind_double)
    LOAD_SQL_FN("sqlite3_bind_blob",   bind_blob)
    LOAD_SQL_FN("sqlite3_column_int",  column_int)
    LOAD_SQL_FN("sqlite3_column_int64",column_int64)
    LOAD_SQL_FN("sqlite3_column_text", column_text)
    LOAD_SQL_FN("sqlite3_column_bytes",column_bytes)
    LOAD_SQL_FN("sqlite3_column_blob", column_blob)
    LOAD_SQL_FN("sqlite3_column_double",column_double)
    LOAD_SQL_FN("sqlite3_errmsg",      errmsg)
    LOAD_SQL_FN("sqlite3_free",        free_)
    s.loaded = true;
    return true;
}
#undef LOAD_SQL_FN

// Helper: convert wstring to UTF-8
std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], len, nullptr, nullptr);
    return s;
}

} // anonymous namespace

//==============================================================================
// CRC32 table (IEEE 802.3)
//==============================================================================
static const uint32_t s_crc32Table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
    0x79DCB8A4, 0xE0D5E91B, 0x97D2D988, 0x09B64C2B, 0x7EB17CBE, 0xE7B82D09, 0x90BF1D3D, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
    0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
    0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F6B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
    0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0D69, 0x086D3D2B, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
    0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
    0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7822, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1,
    0x4B04D447, 0xD20D85FD, 0xA50AB56B};

//==============================================================================
// Construction / Destruction
//==============================================================================
PersistentDiskCache::PersistentDiskCache() = default;

PersistentDiskCache::PersistentDiskCache(const DiskCacheConfig& config) : m_config(config) {}

PersistentDiskCache::~PersistentDiskCache()
{
    Close();
}

//==============================================================================
// Open / Close
//==============================================================================
bool PersistentDiskCache::Open()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (m_isOpen)
        return true;

    if (m_config.cacheDirPath.empty()) {
        // Default to %LOCALAPPDATA%\ExplorerLens\Cache
        wchar_t buf[MAX_PATH] = {};
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) > 0) {
            m_config.cacheDirPath = std::wstring(buf) + L"\\ExplorerLens\\Cache";
        } else {
            m_config.cacheDirPath = L"C:\\ExplorerLensCache";
        }
    }

    // Ensure directory exists
    CreateDirectoryW(m_config.cacheDirPath.c_str(), nullptr);

    // Open SQLite L2 database
    std::wstring dbPath = m_config.cacheDirPath + L"\\catalog.db";
    m_sqliteLoaded = OpenSqliteDb(dbPath);

    m_isOpen = true;
    return true;
}

bool PersistentDiskCache::OpenSqliteDb(const std::wstring& dbPath)
{
    if (!LoadWinSqlite3()) return false;

    auto& sql = Sql();
    std::string utf8Path = WideToUtf8(dbPath);
    if (utf8Path.empty()) return false;

    int rc = sql.open_v2(utf8Path.c_str(),
                         reinterpret_cast<sqlite3_t**>(&m_sqliteDb),
                         WSQL_OPEN_RW_CR, nullptr);
    if (rc != WSQL_OK) { m_sqliteDb = nullptr; return false; }

    // WAL mode + pragmas for performance (ADR A40 schema)
    const char* pragmas =
        "PRAGMA journal_mode = WAL;"
        "PRAGMA synchronous = NORMAL;"
        "PRAGMA cache_size = -8000;"
        "PRAGMA mmap_size = 67108864;"
        "CREATE TABLE IF NOT EXISTS thumbnail_cache ("
        "  id         INTEGER PRIMARY KEY,"
        "  file_path  TEXT    NOT NULL,"
        "  cache_key  TEXT    NOT NULL UNIQUE,"
        "  file_mtime INTEGER NOT NULL DEFAULT 0,"
        "  file_size  INTEGER NOT NULL DEFAULT 0,"
        "  width      INTEGER NOT NULL,"
        "  height     INTEGER NOT NULL,"
        "  data_blob  BLOB,"
        "  data_size  INTEGER NOT NULL DEFAULT 0,"
        "  crc32      INTEGER NOT NULL DEFAULT 0,"
        "  decoder_id TEXT,"
        "  decode_ms  REAL    NOT NULL DEFAULT 0,"
        "  created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
        "  last_hit   INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
        "  hit_count  INTEGER NOT NULL DEFAULT 0"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_cache_key  ON thumbnail_cache(cache_key);"
        "CREATE INDEX IF NOT EXISTS idx_last_hit   ON thumbnail_cache(last_hit);"
        "CREATE INDEX IF NOT EXISTS idx_file_path  ON thumbnail_cache(file_path);";

    char* errMsg = nullptr;
    rc = sql.exec(static_cast<sqlite3_t*>(m_sqliteDb), pragmas, nullptr, nullptr, &errMsg);
    if (errMsg) sql.free_(errMsg);
    if (rc != WSQL_OK) { sql.close_(static_cast<sqlite3_t*>(m_sqliteDb)); m_sqliteDb = nullptr; return false; }

    return true;
}

void PersistentDiskCache::CloseSqliteDb()
{
    if (m_sqliteDb && m_sqliteLoaded) {
        Sql().close_(static_cast<sqlite3_t*>(m_sqliteDb));
        m_sqliteDb = nullptr;
    }
    m_sqliteLoaded = false;
}

void PersistentDiskCache::Close()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen)
        return;
    CloseSqliteDb();
    m_isOpen = false;
}

bool PersistentDiskCache::IsOpen() const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    return m_isOpen;
}

//==============================================================================
// Put / Get / Contains / Remove
//==============================================================================
bool PersistentDiskCache::Put(const std::wstring& filePath, uint32_t width, uint32_t height, const uint8_t* data,
                              uint32_t dataSize, double decodeCostMs, const std::wstring& formatName)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen || !data || dataSize == 0)
        return false;

    uint32_t sz = (width > height) ? width : height;
    auto key = GenerateCacheKey(filePath, sz);

    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    // L1 in-memory update
    CacheEntry entry;
    entry.filePath       = filePath;
    entry.cacheKey       = key;
    entry.width          = width;
    entry.height         = height;
    entry.dataSize       = dataSize;
    entry.crc32          = ComputeCRC32(data, dataSize);
    entry.decodeCostMs   = decodeCostMs;
    entry.formatName     = formatName;
    entry.state          = CacheEntryState::Valid;
    entry.accessCount    = 0;
    entry.cacheWriteTime = now;
    entry.lastAccessTime = now;

    if (m_index.size() >= m_config.maxEntries)
        RunEviction();

    m_index[key] = entry;

    // L2 SQLite persist
    SqlitePut(key, filePath, width, height, data, dataSize, decodeCostMs, formatName);

    return true;
}

bool PersistentDiskCache::SqlitePut(const std::wstring& key, const std::wstring& filePath,
                                    uint32_t width, uint32_t height,
                                    const uint8_t* data, uint32_t dataSize,
                                    double decodeCostMs, const std::wstring& formatName)
{
    if (!m_sqliteDb || !m_sqliteLoaded) return false;

    auto& sql = Sql();
    const char* sql_str =
        "INSERT OR REPLACE INTO thumbnail_cache"
        " (file_path, cache_key, width, height, data_blob, data_size, crc32, decoder_id, decode_ms,"
        "  created_at, last_hit, hit_count)"
        " VALUES (?,?,?,?,?,?,?,?,?,strftime('%s','now'),strftime('%s','now'),0);";

    sqlite3_stmt_t* stmt = nullptr;
    int rc = sql.prepare_v2(static_cast<sqlite3_t*>(m_sqliteDb), sql_str, -1, &stmt, nullptr);
    if (rc != WSQL_OK || !stmt) return false;

    std::string utf8Path = WideToUtf8(filePath);
    std::string utf8Key  = WideToUtf8(key);
    std::string utf8Fmt  = WideToUtf8(formatName);
    uint32_t crc = ComputeCRC32(data, dataSize);

    sql.bind_text(stmt,  1, utf8Path.c_str(), -1, WSQL_TRANSIENT);
    sql.bind_text(stmt,  2, utf8Key.c_str(),  -1, WSQL_TRANSIENT);
    sql.bind_int64(stmt, 3, static_cast<long long>(width));
    sql.bind_int64(stmt, 4, static_cast<long long>(height));
    sql.bind_blob(stmt,  5, data, static_cast<int>(dataSize), WSQL_TRANSIENT);
    sql.bind_int64(stmt, 6, static_cast<long long>(dataSize));
    sql.bind_int64(stmt, 7, static_cast<long long>(crc));
    sql.bind_text(stmt,  8, utf8Fmt.c_str(),  -1, WSQL_TRANSIENT);
    sql.bind_double(stmt,9, decodeCostMs);

    sql.step(stmt);
    sql.finalize(stmt);
    return true;
}

bool PersistentDiskCache::Get(const std::wstring& filePath, uint32_t& width, uint32_t& height,
                              std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen) {
        m_totalMisses++;
        return false;
    }

    // L1: check in-memory index first (fast path)
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        auto it = m_index.find(key);
        if (it != m_index.end() && it->second.state == CacheEntryState::Valid) {
            auto& entry = it->second;
            entry.lastAccessTime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            entry.accessCount++;
            width  = entry.width;
            height = entry.height;
            m_totalHits++;
            // Note: pixel data is in SQLite blob; retrieve via SqliteGet if needed
            return true;
        }
    }

    // L2: check SQLite (disk-persistent)
    if (SqliteGet(filePath, width, height, data)) {
        m_totalHits++;
        return true;
    }

    m_totalMisses++;
    return false;
}

bool PersistentDiskCache::SqliteGet(const std::wstring& filePath, uint32_t& width, uint32_t& height,
                                    std::vector<uint8_t>& data)
{
    if (!m_sqliteDb || !m_sqliteLoaded) return false;

    auto& sql = Sql();
    const char* sql_str =
        "SELECT cache_key, width, height, data_blob, data_size, crc32, decode_ms"
        " FROM thumbnail_cache WHERE file_path = ?"
        " ORDER BY last_hit DESC LIMIT 1;";

    sqlite3_stmt_t* stmt = nullptr;
    int rc = sql.prepare_v2(static_cast<sqlite3_t*>(m_sqliteDb), sql_str, -1, &stmt, nullptr);
    if (rc != WSQL_OK || !stmt) return false;

    std::string utf8Path = WideToUtf8(filePath);
    sql.bind_text(stmt, 1, utf8Path.c_str(), -1, WSQL_TRANSIENT);

    bool found = false;
    if (sql.step(stmt) == WSQL_ROW) {
        width  = static_cast<uint32_t>(sql.column_int(stmt, 1));
        height = static_cast<uint32_t>(sql.column_int(stmt, 2));
        int blobBytes = sql.column_bytes(stmt, 3);
        const void* blobPtr = sql.column_blob(stmt, 3);
        if (blobPtr && blobBytes > 0) {
            data.resize(static_cast<size_t>(blobBytes));
            memcpy(data.data(), blobPtr, static_cast<size_t>(blobBytes));
        }
        found = true;

        // Update last_hit in background (best-effort)
        const char* upd =
            "UPDATE thumbnail_cache SET last_hit = strftime('%s','now'), hit_count = hit_count + 1"
            " WHERE file_path = ?;";
        sqlite3_stmt_t* updStmt = nullptr;
        if (sql.prepare_v2(static_cast<sqlite3_t*>(m_sqliteDb), upd, -1, &updStmt, nullptr) == WSQL_OK) {
            sql.bind_text(updStmt, 1, utf8Path.c_str(), -1, WSQL_TRANSIENT);
            sql.step(updStmt);
            sql.finalize(updStmt);
        }

        // Promote to L1 index
        CacheEntry entry;
        entry.filePath       = filePath;
        entry.width          = width;
        entry.height         = height;
        entry.dataSize       = static_cast<uint32_t>(data.size());
        entry.state          = CacheEntryState::Valid;
        entry.accessCount    = 1;
        auto now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        entry.cacheWriteTime = now;
        entry.lastAccessTime = now;
        auto key = GenerateCacheKey(filePath, (width > height) ? width : height);
        entry.cacheKey = key;
        m_index[key] = std::move(entry);
    }

    sql.finalize(stmt);
    return found;
}

bool PersistentDiskCache::Contains(const std::wstring& filePath) const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        auto it = m_index.find(key);
        if (it != m_index.end() && it->second.state == CacheEntryState::Valid)
            return true;
    }
    return false;
}

bool PersistentDiskCache::Remove(const std::wstring& filePath)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    bool removed = false;
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        if (m_index.erase(key) > 0)
            removed = true;
    }
    // Also remove from SQLite
    if (m_sqliteDb && m_sqliteLoaded) {
        auto& sql = Sql();
        const char* del_str = "DELETE FROM thumbnail_cache WHERE file_path = ?;";
        sqlite3_stmt_t* stmt = nullptr;
        if (sql.prepare_v2(static_cast<sqlite3_t*>(m_sqliteDb), del_str, -1, &stmt, nullptr) == WSQL_OK) {
            std::string utf8Path = WideToUtf8(filePath);
            sql.bind_text(stmt, 1, utf8Path.c_str(), -1, WSQL_TRANSIENT);
            sql.step(stmt);
            sql.finalize(stmt);
        }
    }
    return removed;
}

//==============================================================================
// Invalidate / Eviction
//==============================================================================
uint32_t PersistentDiskCache::InvalidateStale()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    uint32_t invalidated = 0;
    uint64_t ttlSeconds = static_cast<uint64_t>(m_config.entryTTLHours) * 3600;

    for (auto& [key, entry] : m_index) {
        if (entry.state == CacheEntryState::Valid && now - entry.cacheWriteTime > ttlSeconds) {
            entry.state = CacheEntryState::Expired;
            invalidated++;
        }
    }
    return invalidated;
}

uint32_t PersistentDiskCache::RunEviction()
{
    // Note: caller must hold m_cacheMutex
    uint32_t targetEvict = static_cast<uint32_t>(m_index.size() / 10);
    if (targetEvict == 0) targetEvict = 1;

    uint32_t evicted = 0;
    switch (m_config.evictionStrategy) {
        case EvictionStrategy::CostAware:
            evicted = EvictCostAware(targetEvict);
            break;
        case EvictionStrategy::Hybrid:
            evicted  = EvictLRU(targetEvict / 2 + 1);
            evicted += EvictCostAware(targetEvict - evicted);
            break;
        default:
            evicted = EvictLRU(targetEvict);
            break;
    }

    // Mirror eviction to SQLite: delete oldest LRU entries beyond budget
    if (m_sqliteDb && m_sqliteLoaded) {
        auto& sql = Sql();
        const char* evict_sql =
            "DELETE FROM thumbnail_cache WHERE id IN ("
            "  SELECT id FROM thumbnail_cache ORDER BY last_hit ASC LIMIT ?"
            ");";
        sqlite3_stmt_t* stmt = nullptr;
        if (sql.prepare_v2(static_cast<sqlite3_t*>(m_sqliteDb), evict_sql, -1, &stmt, nullptr) == WSQL_OK) {
            sql.bind_int64(stmt, 1, static_cast<long long>(targetEvict));
            sql.step(stmt);
            sql.finalize(stmt);
        }
    }

    m_evictedEntries += evicted;
    return evicted;
}

uint32_t PersistentDiskCache::EvictLRU(uint32_t count)
{
    if (m_index.empty() || count == 0)
        return 0;

    std::vector<std::wstring> keys;
    keys.reserve(m_index.size());
    for (auto& [key, entry] : m_index)
        keys.push_back(key);

    std::sort(keys.begin(), keys.end(),
              [this](const auto& a, const auto& b) {
                  return m_index.at(a).lastAccessTime < m_index.at(b).lastAccessTime;
              });

    uint32_t evicted = 0;
    for (const auto& key : keys) {
        if (evicted >= count) break;
        m_index.erase(key);
        evicted++;
    }
    return evicted;
}

uint32_t PersistentDiskCache::EvictCostAware(uint32_t count)
{
    if (m_index.empty() || count == 0)
        return 0;

    std::vector<std::pair<std::wstring, double>> scores;
    scores.reserve(m_index.size());
    for (auto& [key, entry] : m_index)
        scores.emplace_back(key, CalculateEvictionScore(entry));

    std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    uint32_t evicted = 0;
    for (const auto& [key, score] : scores) {
        if (evicted >= count) break;
        m_index.erase(key);
        evicted++;
    }
    return evicted;
}

double PersistentDiskCache::CalculateEvictionScore(const CacheEntry& entry) const
{
    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    double ageFactor  = static_cast<double>(now - entry.lastAccessTime) / 3600.0;
    double freqFactor = (entry.accessCount > 0) ? (1.0 / static_cast<double>(entry.accessCount)) : 10.0;
    double costFactor = (entry.decodeCostMs > 0)
                        ? (1.0 / (entry.decodeCostMs * m_config.costWeightFactor)) : 1.0;
    double sizeFactor = static_cast<double>(entry.dataSize) / (1024.0 * 1024.0);
    return ageFactor * freqFactor * costFactor + sizeFactor * 0.1;
}

//==============================================================================
// Cache warming
//==============================================================================
bool PersistentDiskCache::StartWarming(const WarmingRequest& request)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen || !m_config.enableWarming || request.directoryPath.empty())
        return false;
    // Full implementation (Phase 2): enumerate directory, queue decode tasks
    return true;
}

void PersistentDiskCache::StopWarming()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    // Full implementation (Phase 2): signal warming thread to stop
}

//==============================================================================
// Compact / Stats
//==============================================================================
bool PersistentDiskCache::Compact()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen)
        return false;

    // Remove non-valid entries from L1
    for (auto it = m_index.begin(); it != m_index.end();) {
        if (it->second.state != CacheEntryState::Valid)
            it = m_index.erase(it);
        else
            ++it;
    }

    // VACUUM SQLite to reclaim disk space
    if (m_sqliteDb && m_sqliteLoaded) {
        char* errMsg = nullptr;
        Sql().exec(static_cast<sqlite3_t*>(m_sqliteDb), "VACUUM;", nullptr, nullptr, &errMsg);
        if (errMsg) Sql().free_(errMsg);
    }
    return true;
}

DiskCacheStats PersistentDiskCache::GetStats() const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);

    DiskCacheStats stats;
    stats.totalEntries  = m_index.size();
    stats.maxDiskBytes  = m_config.maxDiskSizeMB * 1024 * 1024;
    stats.totalHits     = m_totalHits;
    stats.totalMisses   = m_totalMisses;
    stats.evictedEntries = m_evictedEntries;

    uint64_t totalSize = 0;
    for (auto& [key, entry] : m_index) {
        totalSize += entry.dataSize;
        switch (entry.state) {
            case CacheEntryState::Valid:      stats.validEntries++;     break;
            case CacheEntryState::Stale:      stats.staleEntries++;     break;
            case CacheEntryState::Corrupted:  stats.corruptedEntries++; break;
            default: break;
        }
    }
    stats.diskUsageBytes = totalSize;

    uint64_t totalReq  = m_totalHits + m_totalMisses;
    stats.hitRatePercent = (totalReq > 0) ? (static_cast<double>(m_totalHits) / totalReq * 100.0) : 0.0;
    stats.avgHitTimeMs   = (m_totalHits > 0)   ? (m_totalHitTimeMs  / m_totalHits)   : 0.0;
    stats.avgMissTimeMs  = (m_totalMisses > 0)  ? (m_totalMissTimeMs / m_totalMisses) : 0.0;

    return stats;
}

//==============================================================================
// Static helpers
//==============================================================================
const wchar_t* PersistentDiskCache::GetEvictionName(EvictionStrategy strategy)
{
    switch (strategy) {
        case EvictionStrategy::LRU:       return L"LRU";
        case EvictionStrategy::LFU:       return L"LFU";
        case EvictionStrategy::CostAware: return L"CostAware";
        case EvictionStrategy::SizeAware: return L"SizeAware";
        case EvictionStrategy::Hybrid:    return L"Hybrid";
        default:                          return L"Unknown";
    }
}

const wchar_t* PersistentDiskCache::GetEntryStateName(CacheEntryState state)
{
    switch (state) {
        case CacheEntryState::Valid:     return L"Valid";
        case CacheEntryState::Stale:     return L"Stale";
        case CacheEntryState::Corrupted: return L"Corrupted";
        case CacheEntryState::Expired:   return L"Expired";
        case CacheEntryState::Warming:   return L"Warming";
        default:                         return L"Unknown";
    }
}

uint32_t PersistentDiskCache::ComputeCRC32(const uint8_t* data, uint32_t size)
{
    if (!data || size == 0)
        return 0;
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < size; i++)
        crc = s_crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFF;
}

std::wstring PersistentDiskCache::GenerateCacheKey(const std::wstring& filePath, uint32_t thumbnailSize)
{
    // FNV-1a hash of UTF-16 path + thumbnail size
    uint64_t hash = 14695981039346656037ULL;
    for (wchar_t c : filePath) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    hash ^= static_cast<uint64_t>(thumbnailSize);
    hash *= 1099511628211ULL;

    wchar_t buf[32];
    swprintf_s(buf, 32, L"%016llX", hash);
    return std::wstring(buf);
}

}  // namespace Engine
}  // namespace ExplorerLens

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// CRC32 table (IEEE 802.3)
//==============================================================================
static const uint32_t s_crc32Table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
    0x79DCB8A4, 0xE0D5E91B, 0x97D2D988, 0x09B64C2B, 0x7EB17CBE, 0xE7B82D09, 0x90BF1D3D, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A,
    0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3,
    0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F6B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
    0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0D69, 0x086D3D2B, 0x91646C97, 0xE6635C01, 0x6B6B51F4,
    0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074,
    0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7822, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1,
    0x4B04D447, 0xD20D85FD, 0xA50AB56B};

//==============================================================================
// Construction / Destruction
//==============================================================================
PersistentDiskCache::PersistentDiskCache() = default;

PersistentDiskCache::PersistentDiskCache(const DiskCacheConfig& config) : m_config(config) {}

PersistentDiskCache::~PersistentDiskCache()
{
    Close();
}

//==============================================================================
// Open / Close
//==============================================================================
bool PersistentDiskCache::Open()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (m_isOpen)
        return true;

    if (m_config.cacheDirPath.empty()) {
        // Default to %LOCALAPPDATA%\ExplorerLens\Cache
        wchar_t buf[MAX_PATH] = {};
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) > 0) {
            m_config.cacheDirPath = std::wstring(buf) + L"\\ExplorerLens\\Cache";
        } else {
            m_config.cacheDirPath = L"C:\\ExplorerLensCache";
        }
    }

    m_isOpen = true;
    return true;
}

void PersistentDiskCache::Close()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen)
        return;
    m_isOpen = false;
}

bool PersistentDiskCache::IsOpen() const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    return m_isOpen;
}

//==============================================================================
// Put / Get / Contains / Remove
//==============================================================================
bool PersistentDiskCache::Put(const std::wstring& filePath, uint32_t width, uint32_t height, const uint8_t* data,
                              uint32_t dataSize, double decodeCostMs, const std::wstring& formatName)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen || !data || dataSize == 0)
        return false;

    auto key = GenerateCacheKey(filePath, (width > height) ? width : height);

    CacheEntry entry;
    entry.filePath = filePath;
    entry.cacheKey = key;
    entry.width = width;
    entry.height = height;
    entry.dataSize = dataSize;
    entry.crc32 = ComputeCRC32(data, dataSize);
    entry.decodeCostMs = decodeCostMs;
    entry.formatName = formatName;
    entry.state = CacheEntryState::Valid;
    entry.accessCount = 0;

    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    entry.cacheWriteTime = now;
    entry.lastAccessTime = now;

    // Check disk budget before inserting
    if (m_index.size() >= m_config.maxEntries) {
        RunEviction();
    }

    m_index[key] = std::move(entry);
    return true;
}

bool PersistentDiskCache::Get(const std::wstring& filePath, uint32_t& width, uint32_t& height,
                              std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen) {
        m_totalMisses++;
        return false;
    }

    // Try common thumbnail sizes
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        auto it = m_index.find(key);
        if (it != m_index.end() && it->second.state == CacheEntryState::Valid) {
            auto& entry = it->second;
            entry.lastAccessTime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count());
            entry.accessCount++;
            width = entry.width;
            height = entry.height;
            // In full implementation, read blob from disk and verify CRC32
            data.resize(entry.dataSize, 0);
            m_totalHits++;
            return true;
        }
    }

    m_totalMisses++;
    return false;
}

bool PersistentDiskCache::Contains(const std::wstring& filePath) const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        auto it = m_index.find(key);
        if (it != m_index.end() && it->second.state == CacheEntryState::Valid) {
            return true;
        }
    }
    return false;
}

bool PersistentDiskCache::Remove(const std::wstring& filePath)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    bool removed = false;
    for (uint32_t sz : {256u, 512u, 128u, 1024u}) {
        auto key = GenerateCacheKey(filePath, sz);
        if (m_index.erase(key) > 0)
            removed = true;
    }
    return removed;
}

//==============================================================================
// Invalidate / Eviction
//==============================================================================
uint32_t PersistentDiskCache::InvalidateStale()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    uint32_t invalidated = 0;
    uint64_t ttlSeconds = static_cast<uint64_t>(m_config.entryTTLHours) * 3600;

    for (auto& [key, entry] : m_index) {
        if (entry.state == CacheEntryState::Valid) {
            if (now - entry.cacheWriteTime > ttlSeconds) {
                entry.state = CacheEntryState::Expired;
                invalidated++;
            }
        }
    }
    return invalidated;
}

uint32_t PersistentDiskCache::RunEviction()
{
    // Note: caller must hold m_cacheMutex
    uint32_t targetEvict = static_cast<uint32_t>(m_index.size() / 10);  // Evict ~10%
    if (targetEvict == 0)
        targetEvict = 1;

    uint32_t evicted = 0;
    switch (m_config.evictionStrategy) {
        case EvictionStrategy::CostAware:
            evicted = EvictCostAware(targetEvict);
            break;
        case EvictionStrategy::Hybrid:
            // Hybrid: half LRU, half cost-aware
            evicted = EvictLRU(targetEvict / 2 + 1);
            evicted += EvictCostAware(targetEvict - evicted);
            break;
        default:
            evicted = EvictLRU(targetEvict);
            break;
    }

    m_evictedEntries += evicted;
    return evicted;
}

uint32_t PersistentDiskCache::EvictLRU(uint32_t count)
{
    if (m_index.empty() || count == 0)
        return 0;

    // Sort entries by last access time (oldest first)
    std::vector<std::wstring> keys;
    keys.reserve(m_index.size());
    for (auto& [key, entry] : m_index) {
        keys.push_back(key);
    }

    std::sort(keys.begin(), keys.end(),
              [this](const auto& a, const auto& b) { return m_index[a].lastAccessTime < m_index[b].lastAccessTime; });

    uint32_t evicted = 0;
    for (const auto& key : keys) {
        if (evicted >= count)
            break;
        m_index.erase(key);
        evicted++;
    }
    return evicted;
}

uint32_t PersistentDiskCache::EvictCostAware(uint32_t count)
{
    if (m_index.empty() || count == 0)
        return 0;

    // Sort by eviction score (highest score = most evictable)
    std::vector<std::pair<std::wstring, double>> scores;
    scores.reserve(m_index.size());
    for (auto& [key, entry] : m_index) {
        scores.emplace_back(key, CalculateEvictionScore(entry));
    }

    std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    uint32_t evicted = 0;
    for (const auto& [key, score] : scores) {
        if (evicted >= count)
            break;
        m_index.erase(key);
        evicted++;
    }
    return evicted;
}

double PersistentDiskCache::CalculateEvictionScore(const CacheEntry& entry) const
{
    auto now = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    double ageFactor = static_cast<double>(now - entry.lastAccessTime) / 3600.0;
    double freqFactor = (entry.accessCount > 0) ? (1.0 / static_cast<double>(entry.accessCount)) : 10.0;
    double costFactor = (entry.decodeCostMs > 0) ? (1.0 / (entry.decodeCostMs * m_config.costWeightFactor)) : 1.0;
    double sizeFactor = static_cast<double>(entry.dataSize) / (1024.0 * 1024.0);

    return ageFactor * freqFactor * costFactor + sizeFactor * 0.1;
}

//==============================================================================
// Cache warming
//==============================================================================
bool PersistentDiskCache::StartWarming(const WarmingRequest& request)
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen || !m_config.enableWarming)
        return false;
    if (request.directoryPath.empty())
        return false;
    // In full implementation: enumerate directory, decode files, insert into cache
    return true;
}

void PersistentDiskCache::StopWarming()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    // In full implementation: signal warming thread to stop
}

//==============================================================================
// Compact / Stats
//==============================================================================
bool PersistentDiskCache::Compact()
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (!m_isOpen)
        return false;

    // Remove non-valid entries
    uint32_t removed = 0;
    for (auto it = m_index.begin(); it != m_index.end();) {
        if (it->second.state != CacheEntryState::Valid) {
            it = m_index.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    return true;
}

DiskCacheStats PersistentDiskCache::GetStats() const
{
    std::lock_guard<std::mutex> lock(m_cacheMutex);

    DiskCacheStats stats;
    stats.totalEntries = m_index.size();
    stats.maxDiskBytes = m_config.maxDiskSizeMB * 1024 * 1024;
    stats.totalHits = m_totalHits;
    stats.totalMisses = m_totalMisses;
    stats.evictedEntries = m_evictedEntries;

    uint64_t totalSize = 0;
    for (auto& [key, entry] : m_index) {
        totalSize += entry.dataSize;
        switch (entry.state) {
            case CacheEntryState::Valid:
                stats.validEntries++;
                break;
            case CacheEntryState::Stale:
                stats.staleEntries++;
                break;
            case CacheEntryState::Corrupted:
                stats.corruptedEntries++;
                break;
            default:
                break;
        }
    }
    stats.diskUsageBytes = totalSize;

    uint64_t totalReq = m_totalHits + m_totalMisses;
    stats.hitRatePercent = (totalReq > 0) ? (static_cast<double>(m_totalHits) / totalReq * 100.0) : 0.0;
    stats.avgHitTimeMs = (m_totalHits > 0) ? (m_totalHitTimeMs / m_totalHits) : 0.0;
    stats.avgMissTimeMs = (m_totalMisses > 0) ? (m_totalMissTimeMs / m_totalMisses) : 0.0;

    return stats;
}

//==============================================================================
// Static helpers
//==============================================================================
const wchar_t* PersistentDiskCache::GetEvictionName(EvictionStrategy strategy)
{
    switch (strategy) {
        case EvictionStrategy::LRU:
            return L"LRU";
        case EvictionStrategy::LFU:
            return L"LFU";
        case EvictionStrategy::CostAware:
            return L"CostAware";
        case EvictionStrategy::SizeAware:
            return L"SizeAware";
        case EvictionStrategy::Hybrid:
            return L"Hybrid";
        default:
            return L"Unknown";
    }
}

const wchar_t* PersistentDiskCache::GetEntryStateName(CacheEntryState state)
{
    switch (state) {
        case CacheEntryState::Valid:
            return L"Valid";
        case CacheEntryState::Stale:
            return L"Stale";
        case CacheEntryState::Corrupted:
            return L"Corrupted";
        case CacheEntryState::Expired:
            return L"Expired";
        case CacheEntryState::Warming:
            return L"Warming";
        default:
            return L"Unknown";
    }
}

uint32_t PersistentDiskCache::ComputeCRC32(const uint8_t* data, uint32_t size)
{
    if (!data || size == 0)
        return 0;

    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < size; i++) {
        crc = s_crc32Table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

std::wstring PersistentDiskCache::GenerateCacheKey(const std::wstring& filePath, uint32_t thumbnailSize)
{
    // Simple hash-based key: FNV-1a of UTF-16 path + size
    uint64_t hash = 14695981039346656037ULL;
    for (wchar_t c : filePath) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    hash ^= static_cast<uint64_t>(thumbnailSize);
    hash *= 1099511628211ULL;

    wchar_t buf[32];
    swprintf_s(buf, 32, L"%016llX", hash);
    return std::wstring(buf);
}

}  // namespace Engine
}  // namespace ExplorerLens
