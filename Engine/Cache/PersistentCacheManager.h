//==============================================================================
// ExplorerLens Engine — Persistent Cache & USN Journal
// Activate PersistentDiskCache with USN journal cache invalidation.
// Cache warming on folder open, cross-session persistence.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Cache storage backend
enum class CacheBackend : uint8_t {
    Memory,         // In-memory LRU (fastest, lost on restart)
    SQLite,         // SQLite database (persistent, moderate speed)
    FileSystem,     // Direct file storage (persistent, fast for large)
    Hybrid          // Memory + SQLite (default recommended)
};

/// Cache invalidation policy
enum class InvalidationPolicy : uint8_t {
    USNJournal,     // Windows USN Change Journal — detects file changes
    FileTimestamp,  // Compare LastWriteTime
    ContentHash,    // SHA-256 of first N bytes
    Manual,         // User-triggered refresh
    Hybrid          // USN primary, timestamp fallback
};

/// Cache entry metadata
struct CacheEntryInfo {
    uint64_t    fileId          = 0;    // Stable file ID from USN
    uint64_t    usn             = 0;    // USN sequence number
    uint64_t    lastModified    = 0;    // FILETIME
    uint32_t    thumbnailWidth  = 0;
    uint32_t    thumbnailHeight = 0;
    uint32_t    dataSize        = 0;    // Cached thumbnail bytes
    std::wstring filePath;
    bool        valid           = false;
};

/// Cache statistics
struct PersistentCacheStats {
    uint64_t totalEntries   = 0;
    uint64_t memoryBytes    = 0;
    uint64_t diskBytes      = 0;
    uint64_t hits           = 0;
    uint64_t misses         = 0;
    uint64_t evictions      = 0;
    uint64_t invalidations  = 0;
    double   hitRate        = 0;    // 0.0-1.0
};

/// Persistent cache configuration
struct PersistentCacheConfig {
    CacheBackend        backend     = CacheBackend::Hybrid;
    InvalidationPolicy  policy      = InvalidationPolicy::Hybrid;
    uint64_t            maxMemoryMB = 128;
    uint64_t            maxDiskMB   = 512;
    uint32_t            maxEntries  = 50000;
    uint32_t            ttlHours    = 168;  // 7 days default
    bool                enableUSN   = true;
    bool                enableWarm  = true; // Pre-warm on folder open
    std::wstring        cacheDir;
};

/// Persistent cache manager
class PersistentCacheManager {
public:
    /// Backend name
    static const wchar_t* BackendName(CacheBackend b) {
        switch (b) {
            case CacheBackend::Memory:     return L"Memory";
            case CacheBackend::SQLite:     return L"SQLite";
            case CacheBackend::FileSystem: return L"FileSystem";
            case CacheBackend::Hybrid:     return L"Hybrid";
            default: return L"Unknown";
        }
    }

    /// Policy name
    static const wchar_t* PolicyName(InvalidationPolicy p) {
        switch (p) {
            case InvalidationPolicy::USNJournal:    return L"USN Journal";
            case InvalidationPolicy::FileTimestamp: return L"File Timestamp";
            case InvalidationPolicy::ContentHash:   return L"Content Hash";
            case InvalidationPolicy::Manual:        return L"Manual";
            case InvalidationPolicy::Hybrid:        return L"Hybrid";
            default: return L"Unknown";
        }
    }

    /// Validate config
    static bool ValidateConfig(const PersistentCacheConfig& cfg) {
        if (cfg.maxMemoryMB == 0 || cfg.maxMemoryMB > 4096) return false;
        if (cfg.maxDiskMB == 0 || cfg.maxDiskMB > 32768) return false;
        if (cfg.maxEntries == 0) return false;
        return true;
    }

    /// Calculate hit rate
    static double CalculateHitRate(uint64_t hits, uint64_t misses) {
        uint64_t total = hits + misses;
        if (total == 0) return 0.0;
        return static_cast<double>(hits) / total;
    }

    /// Backend count
    static constexpr size_t BackendCount() { return 4; }

    /// Policy count
    static constexpr size_t PolicyCount() { return 5; }
};

}} // namespace ExplorerLens::Engine

