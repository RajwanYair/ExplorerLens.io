// Enhanced Multi-Level Cache System for DarkThumbs
// Implements memory cache + disk cache with preloading

#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>
#include <vector>
#include <chrono>
#include <algorithm>
#include "error_logger.h"
#include "performance_profiler.h"

namespace DarkThumbs {

struct CacheEntry {
    std::string key;
    std::vector<BYTE> data;
    size_t dataSize;
    std::chrono::system_clock::time_point accessTime;
    std::chrono::system_clock::time_point createTime;
    int width;
    int height;
    uint32_t accessCount;
    bool isPinned;

    CacheEntry() : dataSize(0), width(0), height(0), accessCount(0), isPinned(false) {}
};

class EnhancedCacheManager {
private:
    // Memory cache (LRU)
    std::unordered_map<std::string, std::shared_ptr<CacheEntry>> m_memoryCache;
    std::list<std::string> m_lruList;
    std::mutex m_memoryMutex;
    
    // Disk cache path
    std::wstring m_diskCachePath;
    std::mutex m_diskMutex;
    
    // Configuration
    size_t m_maxMemoryCacheSize;  // bytes
    size_t m_maxMemoryCacheCount;
    size_t m_currentMemoryCacheSize;
    
    size_t m_maxDiskCacheSize;    // bytes
    size_t m_currentDiskCacheSize;
    
    bool m_enableMemoryCache;
    bool m_enableDiskCache;
    bool m_enablePreloading;
    
    // Statistics
    std::atomic<uint64_t> m_memoryHits;
    std::atomic<uint64_t> m_diskHits;
    std::atomic<uint64_t> m_misses;
    std::atomic<uint64_t> m_evictions;

    EnhancedCacheManager() 
        : m_maxMemoryCacheSize(100 * 1024 * 1024)  // 100 MB default
        , m_maxMemoryCacheCount(500)
        , m_currentMemoryCacheSize(0)
        , m_maxDiskCacheSize(1024 * 1024 * 1024)  // 1 GB default
        , m_currentDiskCacheSize(0)
        , m_enableMemoryCache(true)
        , m_enableDiskCache(true)
        , m_enablePreloading(false)
        , m_memoryHits(0)
        , m_diskHits(0)
        , m_misses(0)
        , m_evictions(0)
    {
        LoadConfiguration();
        InitializeDiskCache();
    }

    void LoadConfiguration() {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\DarkThumbs\\Cache", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD value = 0;
            DWORD size = sizeof(DWORD);
            
            if (RegQueryValueExW(hKey, L"MemoryCacheSizeMB", nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                m_maxMemoryCacheSize = static_cast<size_t>(value) * 1024 * 1024;
            }
            
            if (RegQueryValueExW(hKey, L"DiskCacheSizeMB", nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                m_maxDiskCacheSize = static_cast<size_t>(value) * 1024 * 1024;
            }
            
            if (RegQueryValueExW(hKey, L"EnableMemoryCache", nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                m_enableMemoryCache = (value != 0);
            }
            
            if (RegQueryValueExW(hKey, L"EnableDiskCache", nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                m_enableDiskCache = (value != 0);
            }
            
            if (RegQueryValueExW(hKey, L"EnablePreloading", nullptr, nullptr, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
                m_enablePreloading = (value != 0);
            }
            
            RegCloseKey(hKey);
        }
    }

    void InitializeDiskCache() {
        wchar_t localAppData[MAX_PATH];
        if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData) == S_OK) {
            m_diskCachePath = localAppData;
            m_diskCachePath += L"\\DarkThumbs\\cache";
            
            if (m_enableDiskCache) {
                SHCreateDirectoryExW(nullptr, m_diskCachePath.c_str(), nullptr);
                CalculateDiskCacheSize();
            }
        }
    }

    void CalculateDiskCacheSize() {
        PROFILE_SCOPE("Cache_CalculateDiskSize");
        
        std::lock_guard<std::mutex> lock(m_diskMutex);
        m_currentDiskCacheSize = 0;

        WIN32_FIND_DATAW findData;
        std::wstring searchPath = m_diskCachePath + L"\\*.png";
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    LARGE_INTEGER fileSize;
                    fileSize.LowPart = findData.nFileSizeLow;
                    fileSize.HighPart = findData.nFileSizeHigh;
                    m_currentDiskCacheSize += fileSize.QuadPart;
                }
            } while (FindNextFileW(hFind, &findData));
            FindClose(hFind);
        }
    }

    void EvictLRU() {
        PROFILE_SCOPE("Cache_EvictLRU");
        
        if (m_lruList.empty()) return;

        auto key = m_lruList.back();
        m_lruList.pop_back();
        
        auto it = m_memoryCache.find(key);
        if (it != m_memoryCache.end()) {
            if (!it->second->isPinned) {
                m_currentMemoryCacheSize -= it->second->dataSize;
                m_memoryCache.erase(it);
                m_evictions++;
                
                DT_LOG_DEBUG(LogCategory::CACHE, "Evicted cache entry: " + key);
            }
        }
    }

    void MakeMostRecent(const std::string& key) {
        auto it = std::find(m_lruList.begin(), m_lruList.end(), key);
        if (it != m_lruList.end()) {
            m_lruList.erase(it);
        }
        m_lruList.push_front(key);
    }

    std::wstring GetDiskCachePath(const std::string& key) {
        // Convert key to safe filename using hash
        size_t hash = std::hash<std::string>{}(key);
        wchar_t filename[MAX_PATH];
        swprintf_s(filename, L"%s\\%016llx.png", m_diskCachePath.c_str(), hash);
        return filename;
    }

    bool LoadFromDisk(const std::string& key, std::vector<BYTE>& data) {
        PROFILE_SCOPE("Cache_LoadFromDisk");
        
        if (!m_enableDiskCache) return false;

        std::lock_guard<std::mutex> lock(m_diskMutex);
        
        std::wstring path = GetDiskCachePath(key);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize)) {
            CloseHandle(hFile);
            return false;
        }

        data.resize(static_cast<size_t>(fileSize.QuadPart));
        
        DWORD bytesRead = 0;
        bool success = ReadFile(hFile, data.data(), static_cast<DWORD>(fileSize.QuadPart), 
                               &bytesRead, nullptr);
        
        CloseHandle(hFile);
        
        if (success && bytesRead == fileSize.QuadPart) {
            m_diskHits++;
            DT_LOG_DEBUG(LogCategory::CACHE, "Loaded from disk: " + key);
            return true;
        }
        
        return false;
    }

    bool SaveToDisk(const std::string& key, const std::vector<BYTE>& data) {
        PROFILE_SCOPE("Cache_SaveToDisk");
        
        if (!m_enableDiskCache) return false;

        std::lock_guard<std::mutex> lock(m_diskMutex);
        
        // Check if we need to evict old disk cache entries
        while (m_currentDiskCacheSize + data.size() > m_maxDiskCacheSize) {
            if (!EvictOldestDiskEntry()) {
                break;  // Can't evict more
            }
        }

        std::wstring path = GetDiskCachePath(key);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (hFile == INVALID_HANDLE_VALUE) {
            DT_LOG_ERROR(LogCategory::CACHE, "Failed to create cache file: " + key);
            return false;
        }

        DWORD bytesWritten = 0;
        bool success = WriteFile(hFile, data.data(), static_cast<DWORD>(data.size()), 
                                &bytesWritten, nullptr);
        
        CloseHandle(hFile);
        
        if (success && bytesWritten == data.size()) {
            m_currentDiskCacheSize += data.size();
            DT_LOG_DEBUG(LogCategory::CACHE, "Saved to disk: " + key);
            return true;
        }
        
        return false;
    }

    bool EvictOldestDiskEntry() {
        PROFILE_SCOPE("Cache_EvictOldestDisk");
        
        WIN32_FIND_DATAW findData;
        std::wstring searchPath = m_diskCachePath + L"\\*.png";
        HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        }

        FILETIME oldestTime = { MAXDWORD, MAXDWORD };
        std::wstring oldestFile;
        LARGE_INTEGER oldestSize;
        
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (CompareFileTime(&findData.ftLastAccessTime, &oldestTime) < 0) {
                    oldestTime = findData.ftLastAccessTime;
                    oldestFile = m_diskCachePath + L"\\" + findData.cFileName;
                    oldestSize.LowPart = findData.nFileSizeLow;
                    oldestSize.HighPart = findData.nFileSizeHigh;
                }
            }
        } while (FindNextFileW(hFind, &findData));
        
        FindClose(hFind);
        
        if (!oldestFile.empty()) {
            if (DeleteFileW(oldestFile.c_str())) {
                m_currentDiskCacheSize -= oldestSize.QuadPart;
                return true;
            }
        }
        
        return false;
    }

public:
    static EnhancedCacheManager& Instance() {
        static EnhancedCacheManager instance;
        return instance;
    }

    bool Get(const std::string& key, std::vector<BYTE>& data, int& width, int& height) {
        PROFILE_SCOPE("Cache_Get");
        
        // Try memory cache first
        if (m_enableMemoryCache) {
            std::lock_guard<std::mutex> lock(m_memoryMutex);
            
            auto it = m_memoryCache.find(key);
            if (it != m_memoryCache.end()) {
                data = it->second->data;
                width = it->second->width;
                height = it->second->height;
                it->second->accessTime = std::chrono::system_clock::now();
                it->second->accessCount++;
                MakeMostRecent(key);
                m_memoryHits++;
                
                DT_LOG_DEBUG(LogCategory::CACHE, "Memory cache hit: " + key);
                return true;
            }
        }
        
        // Try disk cache
        if (LoadFromDisk(key, data)) {
            // Promote to memory cache
            Put(key, data, width, height);
            return true;
        }
        
        m_misses++;
        return false;
    }

    void Put(const std::string& key, const std::vector<BYTE>& data, int width, int height, bool pinned = false) {
        PROFILE_SCOPE("Cache_Put");
        
        if (m_enableMemoryCache) {
            std::lock_guard<std::mutex> lock(m_memoryMutex);
            
            // Check if already exists
            auto it = m_memoryCache.find(key);
            if (it != m_memoryCache.end()) {
                m_currentMemoryCacheSize -= it->second->dataSize;
            }
            
            // Evict if necessary
            while ((m_currentMemoryCacheSize + data.size() > m_maxMemoryCacheSize || 
                   m_memoryCache.size() >= m_maxMemoryCacheCount) &&
                   !m_lruList.empty()) {
                EvictLRU();
            }
            
            // Create new entry
            auto entry = std::make_shared<CacheEntry>();
            entry->key = key;
            entry->data = data;
            entry->dataSize = data.size();
            entry->width = width;
            entry->height = height;
            entry->createTime = std::chrono::system_clock::now();
            entry->accessTime = entry->createTime;
            entry->accessCount = 1;
            entry->isPinned = pinned;
            
            m_memoryCache[key] = entry;
            m_currentMemoryCacheSize += data.size();
            MakeMostRecent(key);
            
            DT_LOG_DEBUG(LogCategory::CACHE, "Added to memory cache: " + key);
        }
        
        // Also save to disk cache
        if (m_enableDiskCache) {
            SaveToDisk(key, data);
        }
    }

    void Remove(const std::string& key) {
        PROFILE_SCOPE("Cache_Remove");
        
        // Remove from memory
        if (m_enableMemoryCache) {
            std::lock_guard<std::mutex> lock(m_memoryMutex);
            
            auto it = m_memoryCache.find(key);
            if (it != m_memoryCache.end()) {
                m_currentMemoryCacheSize -= it->second->dataSize;
                m_memoryCache.erase(it);
                
                auto lruIt = std::find(m_lruList.begin(), m_lruList.end(), key);
                if (lruIt != m_lruList.end()) {
                    m_lruList.erase(lruIt);
                }
            }
        }
        
        // Remove from disk
        if (m_enableDiskCache) {
            std::lock_guard<std::mutex> lock(m_diskMutex);
            std::wstring path = GetDiskCachePath(key);
            DeleteFileW(path.c_str());
        }
    }

    void Clear() {
        PROFILE_SCOPE("Cache_Clear");
        
        // Clear memory
        if (m_enableMemoryCache) {
            std::lock_guard<std::mutex> lock(m_memoryMutex);
            m_memoryCache.clear();
            m_lruList.clear();
            m_currentMemoryCacheSize = 0;
        }
        
        // Clear disk
        if (m_enableDiskCache) {
            std::lock_guard<std::mutex> lock(m_diskMutex);
            
            WIN32_FIND_DATAW findData;
            std::wstring searchPath = m_diskCachePath + L"\\*.png";
            HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        std::wstring filePath = m_diskCachePath + L"\\" + findData.cFileName;
                        DeleteFileW(filePath.c_str());
                    }
                } while (FindNextFileW(hFind, &findData));
                FindClose(hFind);
            }
            
            m_currentDiskCacheSize = 0;
        }
        
        // Reset statistics
        m_memoryHits = 0;
        m_diskHits = 0;
        m_misses = 0;
        m_evictions = 0;
        
        DT_LOG_INFO(LogCategory::CACHE, "Cache cleared");
    }

    std::string GetStatistics() const {
        uint64_t totalRequests = m_memoryHits + m_diskHits + m_misses;
        double memoryHitRate = totalRequests > 0 ? (static_cast<double>(m_memoryHits) / totalRequests * 100.0) : 0.0;
        double totalHitRate = totalRequests > 0 ? (static_cast<double>(m_memoryHits + m_diskHits) / totalRequests * 100.0) : 0.0;
        
        std::ostringstream stats;
        stats << "Cache Statistics\n";
        stats << "================\n\n";
        stats << "Memory Cache:\n";
        stats << "  Size: " << (m_currentMemoryCacheSize / 1024.0 / 1024.0) << " MB / " 
              << (m_maxMemoryCacheSize / 1024.0 / 1024.0) << " MB\n";
        stats << "  Entries: " << m_memoryCache.size() << " / " << m_maxMemoryCacheCount << "\n";
        stats << "  Hits: " << m_memoryHits << " (" << std::fixed << std::setprecision(1) << memoryHitRate << "%)\n\n";
        
        stats << "Disk Cache:\n";
        stats << "  Size: " << (m_currentDiskCacheSize / 1024.0 / 1024.0) << " MB / " 
              << (m_maxDiskCacheSize / 1024.0 / 1024.0) << " MB\n";
        stats << "  Hits: " << m_diskHits << "\n\n";
        
        stats << "Overall:\n";
        stats << "  Total Requests: " << totalRequests << "\n";
        stats << "  Hit Rate: " << totalHitRate << "%\n";
        stats << "  Misses: " << m_misses << "\n";
        stats << "  Evictions: " << m_evictions << "\n";
        
        return stats.str();
    }
};

} // namespace DarkThumbs
