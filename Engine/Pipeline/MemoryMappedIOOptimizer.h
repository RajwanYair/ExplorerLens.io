#pragma once
// ============================================================================
// MemoryMappedIOOptimizer.h — Zero-Copy Memory-Mapped File I/O (Sprint 551)
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE:
//   Memory-mapped file I/O for zero-copy file reading. Uses Windows
//   CreateFileW / CreateFileMappingW / MapViewOfFile to map file contents
//   directly into the process address space, avoiding read() syscall
//   overhead and extra buffer copies. Includes an internal LRU cache of
//   recently mapped files and partial-range mapping with proper alignment
//   to the system allocation granularity.
//
// CLASSES:
//   - MappedFile: Lightweight descriptor holding the mapped data pointer,
//     file size, and OS handles (hFile, hMapping).
//   - MappingStats: Counters for maps created, cache hits, total bytes
//     mapped, and peak concurrent mappings.
//   - MemoryMappedIOOptimizer: Full memory-mapped I/O engine with LRU
//     cache, range mapping, dynamic PrefetchVirtualMemory loading, and
//     SRWLOCK-based thread safety.
//
// INPUTS:
//   - File paths (std::wstring) for MapFile() / MapFileRange()
//   - Offset and length for partial range mappings
//
// OUTPUTS:
//   - MappedFile with const uint8_t* data pointer for zero-copy reads
//   - MappingStats via GetStats()
//
// THREAD SAFETY:
//   All public methods are thread-safe via Windows SRWLOCK (exclusive for
//   writes, shared for read-only stats queries).
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Memory mapping strategy
enum class MappingStrategy : uint8_t {
    ReadOnly = 0,
    CopyOnWrite,
    LargePages,
    Prefaulted,
    Sequential,
    COUNT
};

/// File access pattern hint
enum class MMapAccessPattern : uint8_t {
    Sequential = 0,
    Random,
    HeaderOnly,
    Streaming,
    COUNT
};

struct MMapFileInfo {
    uint64_t fileSize = 0;
    uint64_t mappedSize = 0;
    uint64_t viewSize = 0;
    uint64_t alignment = 65536;
    bool     isLargePages = false;
    bool     isPrefaulted = false;
    MappingStrategy strategy = MappingStrategy::ReadOnly;
};

struct MMapStats {
    uint64_t filesOpened = 0;
    uint64_t totalBytesMapped = 0;
    uint64_t pageFaults = 0;
    double   avgMapTimeUs = 0.0;
    double   savedCopyBytes = 0.0;
};

/// Lightweight descriptor for a memory-mapped file region.
struct MappedFile {
    const uint8_t* data = nullptr;   ///< Pointer to mapped data
    size_t         size = 0;         ///< Size of the mapped region in bytes
    HANDLE         hFile = INVALID_HANDLE_VALUE;
    HANDLE         hMapping = nullptr;
    const void* viewBase = nullptr;   ///< Actual MapViewOfFile return (for unmap)
};

/// Aggregate mapping statistics.
struct MappingStats {
    uint64_t mapsCreated = 0;
    uint64_t cacheHits = 0;
    uint64_t totalBytesMapped = 0;
    uint64_t peakConcurrentMappings = 0;
};

/// Memory-mapped I/O engine with LRU cache and SRWLOCK thread safety.
class MemoryMappedIOOptimizer {
public:
    // ====================================================================
    // Backward-compatible static API (v14)
    // ====================================================================

    static constexpr size_t StrategyCount() {
        return static_cast<size_t>(MappingStrategy::COUNT);
    }

    static constexpr size_t PatternCount() {
        return static_cast<size_t>(MMapAccessPattern::COUNT);
    }

    static inline const wchar_t* StrategyName(MappingStrategy s) {
        switch (s) {
        case MappingStrategy::ReadOnly:    return L"Read-Only";
        case MappingStrategy::CopyOnWrite: return L"Copy-on-Write";
        case MappingStrategy::LargePages:  return L"Large Pages (2MB)";
        case MappingStrategy::Prefaulted:  return L"Prefaulted";
        case MappingStrategy::Sequential:  return L"Sequential Hint";
        default:                           return L"Unknown";
        }
    }

    static inline const wchar_t* PatternName(MMapAccessPattern p) {
        switch (p) {
        case MMapAccessPattern::Sequential: return L"Sequential";
        case MMapAccessPattern::Random:     return L"Random";
        case MMapAccessPattern::HeaderOnly: return L"Header Only";
        case MMapAccessPattern::Streaming:  return L"Streaming";
        default:                            return L"Unknown";
        }
    }

    /// Calculate aligned mapping offset (Windows requires allocation
    /// granularity alignment, typically 64 KB).
    static inline uint64_t AlignOffset(uint64_t offset,
        uint64_t granularity = 65536) {
        return (offset / granularity) * granularity;
    }

    /// Recommend mapping strategy based on file size.
    static inline MappingStrategy RecommendStrategy(uint64_t fileSize) {
        if (fileSize < 64u * 1024u)
            return MappingStrategy::ReadOnly;
        if (fileSize < 16u * 1024u * 1024u)
            return MappingStrategy::Sequential;
        return MappingStrategy::LargePages;
    }

    // ====================================================================
    // Sprint 551: Full memory-mapped I/O with LRU cache
    // ====================================================================

    /// Construct with a maximum number of cached mappings (LRU eviction).
    explicit MemoryMappedIOOptimizer(size_t maxCached = 16)
        : m_maxCached(maxCached) {
        InitializeSRWLock(&m_lock);
    }

    ~MemoryMappedIOOptimizer() {
        AcquireSRWLockExclusive(&m_lock);
        for (auto& pair : m_cache) {
            CloseMappingHandles(pair.second.file);
        }
        m_cache.clear();
        m_lruOrder.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    MemoryMappedIOOptimizer(const MemoryMappedIOOptimizer&) = delete;
    MemoryMappedIOOptimizer& operator=(const MemoryMappedIOOptimizer&) = delete;

    /// Map an entire file read-only. Results are cached in the LRU.
    /// Returns an empty MappedFile on failure.
    inline MappedFile MapFile(const std::wstring& path) {
        AcquireSRWLockExclusive(&m_lock);

        // Check LRU cache
        auto it = m_cache.find(path);
        if (it != m_cache.end()) {
            // Cache hit — move to front of LRU
            m_lruOrder.erase(it->second.lruIter);
            m_lruOrder.push_front(path);
            it->second.lruIter = m_lruOrder.begin();
            m_stats.cacheHits++;
            MappedFile result = it->second.file;
            ReleaseSRWLockExclusive(&m_lock);
            return result;
        }

        // Cache miss — create new mapping (hold lock; mapping is fast)
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            ReleaseSRWLockExclusive(&m_lock);
            return {};
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
            CloseHandle(hFile);
            ReleaseSRWLockExclusive(&m_lock);
            return {};
        }

        HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY,
            static_cast<DWORD>(static_cast<uint64_t>(fileSize.QuadPart) >> 32),
            static_cast<DWORD>(static_cast<uint64_t>(fileSize.QuadPart) & 0xFFFFFFFFu),
            nullptr);
        if (!hMapping) {
            CloseHandle(hFile);
            ReleaseSRWLockExclusive(&m_lock);
            return {};
        }

        void* view = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
        if (!view) {
            CloseHandle(hMapping);
            CloseHandle(hFile);
            ReleaseSRWLockExclusive(&m_lock);
            return {};
        }

        MappedFile result;
        result.data = static_cast<const uint8_t*>(view);
        result.size = static_cast<size_t>(fileSize.QuadPart);
        result.hFile = hFile;
        result.hMapping = hMapping;
        result.viewBase = view;

        // Evict LRU entries if at capacity
        while (m_cache.size() >= m_maxCached) {
            EvictOldest();
        }

        // Insert into cache and LRU list
        m_lruOrder.push_front(path);
        CacheEntry entry;
        entry.file = result;
        entry.lruIter = m_lruOrder.begin();
        m_cache[path] = entry;

        m_stats.mapsCreated++;
        m_stats.totalBytesMapped += result.size;
        if (m_cache.size() > m_stats.peakConcurrentMappings) {
            m_stats.peakConcurrentMappings = m_cache.size();
        }

        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    /// Map a sub-range of a file with proper alignment to the system
    /// allocation granularity. The returned data pointer is adjusted to the
    /// requested offset. The caller MUST call UnmapFile() on the result.
    /// Range mappings are NOT cached.
    inline MappedFile MapFileRange(const std::wstring& path,
        uint64_t offset, size_t length) {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return {};

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize)) {
            CloseHandle(hFile);
            return {};
        }
        uint64_t totalSize = static_cast<uint64_t>(fileSize.QuadPart);
        if (offset >= totalSize) {
            CloseHandle(hFile);
            return {};
        }

        // Query system allocation granularity for proper alignment
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        uint64_t gran = static_cast<uint64_t>(si.dwAllocationGranularity);
        uint64_t alignedOffset = (offset / gran) * gran;
        size_t extraBytes = static_cast<size_t>(offset - alignedOffset);

        // Clamp mapped length to file extent
        size_t mapLen = length + extraBytes;
        if (alignedOffset + mapLen > totalSize) {
            mapLen = static_cast<size_t>(totalSize - alignedOffset);
        }

        HANDLE hMapping = CreateFileMappingW(hFile, nullptr, PAGE_READONLY,
            0, 0, nullptr);
        if (!hMapping) {
            CloseHandle(hFile);
            return {};
        }

        DWORD offHigh = static_cast<DWORD>(alignedOffset >> 32);
        DWORD offLow = static_cast<DWORD>(alignedOffset & 0xFFFFFFFFu);
        void* view = MapViewOfFile(hMapping, FILE_MAP_READ,
            offHigh, offLow, mapLen);
        if (!view) {
            CloseHandle(hMapping);
            CloseHandle(hFile);
            return {};
        }

        MappedFile result;
        result.data = static_cast<const uint8_t*>(view) + extraBytes;
        result.size = (length <= mapLen - extraBytes)
            ? length : (mapLen - extraBytes);
        result.hFile = hFile;
        result.hMapping = hMapping;
        result.viewBase = view;  // actual base for UnmapViewOfFile

        AcquireSRWLockExclusive(&m_lock);
        m_stats.mapsCreated++;
        m_stats.totalBytesMapped += result.size;
        ReleaseSRWLockExclusive(&m_lock);

        return result;
    }

    /// Unmap a file. For cached mappings (from MapFile), this removes the
    /// entry from the cache. For uncached range mappings, this directly
    /// releases OS resources.
    inline void UnmapFile(MappedFile& file) {
        if (!file.viewBase && !file.data) return;

        AcquireSRWLockExclusive(&m_lock);

        // Check if this mapping is in the cache (compare viewBase pointers)
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->second.file.viewBase == file.viewBase) {
                m_lruOrder.erase(it->second.lruIter);
                CloseMappingHandles(it->second.file);
                m_cache.erase(it);
                file = {};
                ReleaseSRWLockExclusive(&m_lock);
                return;
            }
        }

        ReleaseSRWLockExclusive(&m_lock);

        // Not in cache — close directly
        CloseMappingHandles(file);
        file = {};
    }

    /// Hint the OS to prefetch a region of a mapped file into physical memory.
    /// Uses PrefetchVirtualMemory (Win8+), dynamically loaded at runtime.
    /// Returns false if the API is unavailable or the parameters are invalid.
    inline bool PrefetchRegion(const MappedFile& file,
        size_t offset, size_t prefetchSize) {
        if (!file.data || offset + prefetchSize > file.size) return false;

        // Dynamically load PrefetchVirtualMemory (available on Win8+)
        using PrefetchVMFn = BOOL(WINAPI*)(HANDLE, ULONG_PTR,
            PWIN32_MEMORY_RANGE_ENTRY, ULONG);

        static PrefetchVMFn pfn = []() -> PrefetchVMFn {
            HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
            if (!hKernel) return nullptr;
            return reinterpret_cast<PrefetchVMFn>(
                GetProcAddress(hKernel, "PrefetchVirtualMemory"));
            }();

        if (!pfn) return false;

        WIN32_MEMORY_RANGE_ENTRY entry;
        entry.VirtualAddress = const_cast<void*>(
            static_cast<const void*>(file.data + offset));
        entry.NumberOfBytes = prefetchSize;

        return pfn(GetCurrentProcess(), 1, &entry, 0) != FALSE;
    }

    /// Set the maximum number of cached file mappings. Excess entries are
    /// evicted (LRU order) immediately.
    inline void SetMaxCachedMappings(size_t maxCached) {
        AcquireSRWLockExclusive(&m_lock);
        m_maxCached = maxCached;
        while (m_cache.size() > m_maxCached) {
            EvictOldest();
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Return aggregate mapping statistics.
    inline MappingStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        MappingStats s = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return s;
    }

    /// Number of currently cached mappings.
    inline size_t CachedCount() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        size_t n = m_cache.size();
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return n;
    }

private:
    struct CacheEntry {
        MappedFile file;
        std::list<std::wstring>::iterator lruIter;
    };

    /// Close OS handles for a mapping (UnmapViewOfFile + CloseHandle).
    static inline void CloseMappingHandles(MappedFile& f) {
        if (f.viewBase) {
            UnmapViewOfFile(const_cast<void*>(f.viewBase));
            f.viewBase = nullptr;
            f.data = nullptr;
        }
        if (f.hMapping) {
            CloseHandle(f.hMapping);
            f.hMapping = nullptr;
        }
        if (f.hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(f.hFile);
            f.hFile = INVALID_HANDLE_VALUE;
        }
        f.size = 0;
    }

    /// Evict the least-recently-used cache entry.
    inline void EvictOldest() {
        if (m_lruOrder.empty()) return;
        const std::wstring& oldest = m_lruOrder.back();
        auto it = m_cache.find(oldest);
        if (it != m_cache.end()) {
            CloseMappingHandles(it->second.file);
            m_cache.erase(it);
        }
        m_lruOrder.pop_back();
    }

    size_t                                       m_maxCached;
    std::unordered_map<std::wstring, CacheEntry>  m_cache;
    std::list<std::wstring>                       m_lruOrder;
    MappingStats                                  m_stats{};
    mutable SRWLOCK                               m_lock{};
};

} // namespace Engine
} // namespace ExplorerLens
