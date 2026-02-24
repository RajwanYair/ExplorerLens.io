//==============================================================================
// ExplorerLens Engine - Thumbnail Cache Implementation
// Engine v1.0.0 - Persistent Disk-Backed Cache with LRU Eviction
// Copyright (c) 2026 ExplorerLens Project
//
// FEATURES:
// - Disk-backed persistent cache (survives application restarts)
// - LRU (Least Recently Used) eviction policy
// - MD5-based cache keys with file metadata
// - PNG compression for efficient storage
// - Thread-safe operations with mutex protection
// - Automatic cleanup to maintain size limits (default: 500 MB)
// - Windows LOCALAPPDATA storage location
//
// CACHE EVICTION POLICY:
// The cache uses a pure LRU algorithm:
// 1. Each cache hit updates the entry's "last accessed" timestamp
// 2. When cache size exceeds maxSizeMB, oldest entries are evicted first
// 3. Eviction removes entire cache files (not partial)
// 4. MD5 collision handling: overwrites on collision (statistically rare)
//
// PERFORMANCE:
// - Cache hit: <1ms (file exists check + PNG decompress)
// - Cache miss: ~15-30ms (decode + PNG compress + disk write)
// - Eviction: ~5-10ms per entry (file delete + metadata update)
// - Typical hit rate: 60-80% (varies by user access patterns)
//
// STORAGE LOCATION:
// %LOCALAPPDATA%\ExplorerLens\cache\
// Example: C:\Users\<name>\AppData\Local\ExplorerLens\cache\
//
// CACHE KEY FORMAT:
// MD5(filepath + filesize + lastmodified + width + height)
// Example: "a3f5c2d1b4e6... .png"
//
// TUNING GUIDELINES:
// - For SSD: maxSizeMB=1000 (fast access, large cache)
// - For HDD: maxSizeMB=500 (balanced)
// - For slow drives: maxSizeMB=250 (minimize disk I/O)
// - High thumbnail churn: increase maxSizeMB
// - Memory-constrained: decrease maxSizeMB
//==============================================================================

#pragma once

#include "../Core/ICacheProvider.h"
#include <windows.h>
#include <string>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Persistent thumbnail cache with LRU eviction
/// 
/// Stores thumbnails as PNG files in %LOCALAPPDATA%\ExplorerLens\cache\
/// Uses MD5 hashing for cache keys and LRU policy for eviction.
//==============================================================================
class ThumbnailCache : public ICacheProvider {
public:
    ThumbnailCache();
    ~ThumbnailCache() override;

    // ICacheProvider interface
    bool Exists(const wchar_t* filePath, uint32_t width, uint32_t height) const override;
    HRESULT Get(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* outBitmap) override;
    HRESULT Put(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP hBitmap) override;
    HRESULT Remove(const wchar_t* filePath) override;
    HRESULT Clear() override;
    HRESULT GetStats(uint32_t* outEntryCount, uint32_t* outTotalSizeMB) const override;

    // Additional configuration methods
    HRESULT Initialize(uint32_t maxSizeMB = 500);
    void Shutdown();

    // Enhanced cache configuration
    enum class CompressionLevel {
        None = 0,         // No compression (fastest, largest size)
        Fast = 3,         // Fast compression (good for testing)
        Balanced = 6,     // Default PNG compression
        Maximum = 9       // Maximum compression (slowest, smallest size)
    };
    
    void SetCompressionLevel(CompressionLevel level);
    CompressionLevel GetCompressionLevel() const { return m_compressionLevel; }
    
    // Cache optimization
    HRESULT OptimizeCache();  // Recompress existing entries with current settings
    HRESULT DefragmentCache(); // Remove gaps, reorganize entries
    
    // Extended statistics (beyond ICacheProvider interface)
    struct CacheStatistics {
        uint64_t hitCount;
        uint64_t missCount;
        uint64_t evictionCount;
        uint32_t entryCount;
        uint32_t totalSizeMB;
        double hitRate;  // calculated: hits / (hits + misses)
    };
    
    void GetDetailedStats(CacheStatistics* outStats) const;
    void ResetStatistics();  //Reset hit/miss/eviction counters

private:
    // Cache key generation
    std::wstring GenerateKey(const wchar_t* filePath, uint32_t width, uint32_t height) const;
    std::wstring MD5Hash(const std::wstring& input) const;
    
    // File operations
    std::wstring GetCacheDirectory() const;
    std::wstring GetCachePath(const std::wstring& cacheKey) const;
    bool SaveBitmapToFile(HBITMAP hBitmap, const std::wstring& filePath);
    HBITMAP LoadBitmapFromFile(const std::wstring& filePath);
    
    // File metadata extraction
    struct FileMetadata {
        uint64_t fileSize;
        uint64_t lastModified;
    };
    bool GetFileMetadata(const wchar_t* filePath, FileMetadata& metadata) const;
    
    // Cache management
    void UpdateAccessTime(const std::wstring& key);
    void EvictLRU();
    bool IsInitialized() const { return m_initialized; }
    
    // Cache entry tracking
    struct CacheEntryInfo {
        std::wstring filePath;
        uint64_t size;
        uint64_t lastAccessTime;
        uint64_t creationTime;
    };
    
    // Member variables
    bool m_initialized;
    uint32_t m_maxSizeMB;
    uint64_t m_currentSizeBytes;
    std::wstring m_cacheDirectory;
    CompressionLevel m_compressionLevel;
    
    // Cache metadata (in-memory tracking)
    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, CacheEntryInfo> m_entries;
    
    // Statistics
    mutable uint64_t m_hitCount;
    mutable uint64_t m_missCount;
    uint64_t m_evictionCount;
};

// Factory function
extern "C" {
    __declspec(dllexport) ICacheProvider* CreateThumbnailCache();
}

} // namespace Engine
} // namespace ExplorerLens

