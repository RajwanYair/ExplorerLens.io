// LargePageAllocator.h — Large Page (2MB/1GB) Memory Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Uses Windows Large Pages (2MB) for bitmap buffers to reduce TLB misses
// during decode operations. Falls back to standard pages if SeLockMemory
// privilege is not available.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class PageSize : uint8_t {
    Standard4K,   // 4 KB pages (default)
    Large2MB,     // 2 MB large pages
    Huge1GB,      // 1 GB huge pages (if supported)
    COUNT
};

struct LargePageStats {
    uint64_t largePageAllocs = 0;
    uint64_t standardFallbacks = 0;
    uint64_t totalBytesLarge = 0;
    uint64_t totalBytesStd = 0;
    bool privilegeAvailable = false;
    size_t minLargePageSize = 2 * 1024 * 1024;
};

class LargePageAllocator {
public:
    void Initialize() {
        m_stats.minLargePageSize = 2 * 1024 * 1024; // GetLargePageMinimum()
        m_stats.privilegeAvailable = CheckPrivilege();
    }

    void* Allocate(size_t size) {
        if (m_stats.privilegeAvailable && size >= m_stats.minLargePageSize) {
            m_stats.largePageAllocs++;
            m_stats.totalBytesLarge += size;
            return nullptr; // Stub: VirtualAlloc with MEM_LARGE_PAGES
        }
        m_stats.standardFallbacks++;
        m_stats.totalBytesStd += size;
        return nullptr; // Stub: standard VirtualAlloc
    }

    void Free(void* ptr, size_t size) {
        (void)ptr; (void)size;
        // Stub: VirtualFree
    }

    const LargePageStats& Stats() const { return m_stats; }
    bool IsLargePageAvailable() const { return m_stats.privilegeAvailable; }

    void SetMinSize(size_t sz) { m_stats.minLargePageSize = sz; }

    static const wchar_t* PageSizeName(PageSize p) {
        switch (p) {
        case PageSize::Standard4K: return L"Standard4K";
        case PageSize::Large2MB:   return L"Large2MB";
        case PageSize::Huge1GB:    return L"Huge1GB";
        default: return L"Unknown";
        }
    }
    static size_t PageSizeCount() { return static_cast<size_t>(PageSize::COUNT); }

private:
    bool CheckPrivilege() {
        // Stub: check SeLockMemoryPrivilege
        return false;
    }
    LargePageStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
