// PersistentCacheManager.h — Persistent thumbnail cache with pluggable backends
// Copyright (c) 2026 ExplorerLens Project
//
// Provides CacheBackend, InvalidationPolicy enums and PersistentCacheManager
// for managing on-disk thumbnail caches with configurable eviction policies.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

enum class CacheBackend : uint8_t {
    Hybrid = 0,
    SQLite = 1,
    LevelDB = 2,
    MemoryMap = 3,
    COUNT
};

enum class InvalidationPolicy : uint8_t {
    USNJournal = 0,
    FileWatcher = 1,
    Timer = 2,
    Explicit = 3,
    Hybrid = 4,
    COUNT
};

struct PersistentCacheConfig
{
    uint32_t maxMemoryMB = 256;
    uint32_t maxDiskMB = 2048;
    uint32_t maxEntries = 50000;
    CacheBackend backend = CacheBackend::Hybrid;
    InvalidationPolicy policy = InvalidationPolicy::USNJournal;
};

class PersistentCacheManager
{
  public:
    static const wchar_t* BackendName(CacheBackend b) noexcept
    {
        switch (b) {
            case CacheBackend::Hybrid:
                return L"Hybrid";
            case CacheBackend::SQLite:
                return L"SQLite";
            case CacheBackend::LevelDB:
                return L"LevelDB";
            case CacheBackend::MemoryMap:
                return L"Memory Map";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* PolicyName(InvalidationPolicy p) noexcept
    {
        switch (p) {
            case InvalidationPolicy::USNJournal:
                return L"USN Journal";
            case InvalidationPolicy::FileWatcher:
                return L"File Watcher";
            case InvalidationPolicy::Timer:
                return L"Timer";
            case InvalidationPolicy::Explicit:
                return L"Explicit";
            case InvalidationPolicy::Hybrid:
                return L"Hybrid";
            default:
                return L"Unknown";
        }
    }

    static bool ValidateConfig(const PersistentCacheConfig& cfg) noexcept
    {
        return cfg.maxMemoryMB > 0 && cfg.maxDiskMB > 0;
    }

    static double CalculateHitRate(uint64_t hits, uint64_t misses) noexcept
    {
        uint64_t total = hits + misses;
        return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
    }

    static constexpr size_t BackendCount() noexcept
    {
        return static_cast<size_t>(CacheBackend::COUNT);
    }

    static constexpr size_t PolicyCount() noexcept
    {
        return static_cast<size_t>(InvalidationPolicy::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
