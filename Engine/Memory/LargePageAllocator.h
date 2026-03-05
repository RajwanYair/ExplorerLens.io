// LargePageAllocator.h — Large Page (2MB/1GB) Memory Allocator
// Copyright (c) 2026 ExplorerLens Project
//
// Uses Windows Large Pages (2MB) for bitmap buffers to reduce TLB misses
// during decode operations. Falls back to standard pages if SeLockMemory
// privilege is not available. All allocations use VirtualAlloc/VirtualFree.
//
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

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
        m_stats.minLargePageSize = ::GetLargePageMinimum();
        if (m_stats.minLargePageSize == 0)
            m_stats.minLargePageSize = 2 * 1024 * 1024; // Default 2MB
        m_stats.privilegeAvailable = AcquireLargePagePrivilege();
    }

    void* Allocate(size_t size) {
        if (size == 0) return nullptr;

        // Try large pages if privilege is available and size warrants it
        if (m_stats.privilegeAvailable && size >= m_stats.minLargePageSize) {
            // Round up to large page boundary
            size_t rounded = (size + m_stats.minLargePageSize - 1)
                & ~(m_stats.minLargePageSize - 1);
            void* ptr = ::VirtualAlloc(
                nullptr, rounded,
                MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES,
                PAGE_READWRITE);
            if (ptr) {
                m_stats.largePageAllocs++;
                m_stats.totalBytesLarge += rounded;
                return ptr;
            }
            // Large page alloc failed — fall through to standard
        }

        // Standard allocation via VirtualAlloc (4K pages)
        void* ptr = ::VirtualAlloc(
            nullptr, size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);
        if (ptr) {
            m_stats.standardFallbacks++;
            m_stats.totalBytesStd += size;
        }
        return ptr;
    }

    void Free(void* ptr, size_t /*size*/) {
        if (ptr) {
            ::VirtualFree(ptr, 0, MEM_RELEASE);
        }
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
    // Attempt to enable SeLockMemoryPrivilege for the current process token
    static bool AcquireLargePagePrivilege() {
        HANDLE hToken = nullptr;
        if (!::OpenProcessToken(::GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
            return false;

        TOKEN_PRIVILEGES tp = {};
        if (!::LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &tp.Privileges[0].Luid)) {
            ::CloseHandle(hToken);
            return false;
        }
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        BOOL ok = ::AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr);
        DWORD err = ::GetLastError();
        ::CloseHandle(hToken);
        // AdjustTokenPrivileges returns TRUE even if it couldn't assign all
        return (ok && err == ERROR_SUCCESS);
    }

    LargePageStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
