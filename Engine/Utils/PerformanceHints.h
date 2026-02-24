//==============================================================================
// ExplorerLens Engine - Performance Hints
// Copyright (c) 2026 - ExplorerLens Project
// Task B9: Performance optimization hints and prefetching
//==============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <atomic>

namespace ExplorerLens {
namespace Engine {
namespace Performance {

    /// <summary>
    /// CPU cache optimization hints
    /// </summary>
    namespace CacheHints {
        // Prefetch data into cache (read-only access expected)
        inline void PrefetchForRead(const void* addr) {
            _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0);
        }

        // Prefetch with non-temporal hint (will not be reused)
        inline void PrefetchNonTemporal(const void* addr) {
            _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_NTA);
        }

        // Cache line size (64 bytes on x64)
        constexpr size_t CacheLineSize = 64;

        // Align data to cache line boundary
        template<typename T>
        inline T* AlignToCacheLine(T* ptr) {
            uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
            uintptr_t aligned = (addr + CacheLineSize - 1) & ~(CacheLineSize - 1);
            return reinterpret_cast<T*>(aligned);
        }
    }

    /// <summary>
    /// Memory allocation hints
    /// </summary>
    namespace MemoryHints {
        // Allocate aligned memory for SIMD operations
        inline void* AllocateAligned(size_t size, size_t alignment = 32) {
            return _aligned_malloc(size, alignment);
        }

        inline void FreeAligned(void* ptr) {
            _aligned_free(ptr);
        }

        // Large page allocation hint (for buffers > 2MB)
        inline void* AllocateLargePages(size_t size) {
            // Try to get large page minimum size
            SIZE_T minSize = GetLargePageMinimum();
            if (minSize > 0 && size >= minSize) {
                return VirtualAlloc(nullptr, size, 
                    MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES,
                    PAGE_READWRITE);
            }
            return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        }

        inline void FreeLargePages(void* ptr) {
            if (ptr) VirtualFree(ptr, 0, MEM_RELEASE);
        }
    }

    /// <summary>
    /// File I/O optimization hints
    /// </summary>
    namespace IOHints {
        // Open file with optimized flags for sequential read
        inline HANDLE OpenForSequentialRead(const wchar_t* filePath) {
            return CreateFileW(
                filePath,
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                nullptr
            );
        }

        // Open file with optimized flags for random access
        inline HANDLE OpenForRandomRead(const wchar_t* filePath) {
            return CreateFileW(
                filePath,
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                nullptr
            );
        }

        // Open file with overlapped I/O for async operations
        inline HANDLE OpenForAsyncRead(const wchar_t* filePath) {
            return CreateFileW(
                filePath,
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                nullptr
            );
        }
    }

    /// <summary>
    /// Thread scheduling hints
    /// </summary>
    namespace ThreadHints {
        // Set thread to background priority for non-critical work
        inline void SetBackgroundPriority() {
            SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
        }

        // Restore normal thread priority
        inline void RestoreNormalPriority() {
            SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
        }

        // Set thread affinity to E-cores (efficiency cores) if available
        inline void SetEfficiencyCoreAffinity() {
            // On hybrid CPUs, E-cores are typically the higher-numbered cores
            // This is a simplified heuristic
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            
            if (sysInfo.dwNumberOfProcessors > 8) {
                // Assume P-cores are 0-7, E-cores are 8+
                DWORD_PTR mask = 0;
                for (DWORD i = 8; i < sysInfo.dwNumberOfProcessors; ++i) {
                    mask |= (1ULL << i);
                }
                SetThreadAffinityMask(GetCurrentThread(), mask);
            }
        }
    }

    /// <summary>
    /// SIMD optimization hints
    /// </summary>
    namespace SIMDHints {
        // Check if data is aligned for SIMD operations
        inline bool IsAligned(const void* ptr, size_t alignment = 16) {
            return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
        }

        // Get optimal SIMD alignment for current CPU
        inline size_t GetOptimalAlignment() {
            // AVX-512: 64 bytes, AVX2: 32 bytes, SSE: 16 bytes
            return 32;  // AVX2 is most common
        }
    }

} // namespace Performance
} // namespace Engine
} // namespace ExplorerLens

