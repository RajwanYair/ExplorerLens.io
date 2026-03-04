// PrefetchHintEngine.h — CPU prefetch hints for decode pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Issues CPU prefetch hints (_mm_prefetch) before decode operations to warm
// caches. Supports sequential, stride-based, and adaptive prefetch strategies.
// SIMD feature checks ensure prefetch intrinsics are available at runtime.
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <intrin.h>

namespace ExplorerLens {
namespace Engine {

/// Prefetch hint strategy for different access patterns (distinct from Pipeline::PrefetchStrategy)
enum class PrefetchHintStrategy : uint8_t {
    None       = 0,  // No prefetching
    Sequential = 1,  // Linear sequential access (file headers, row scans)
    Stride     = 2,  // Strided access (interleaved channels, tiled images)
    Adaptive   = 3   // Runtime-adaptive based on observed patterns
};

/// Prefetch locality hint (maps to _MM_HINT values)
enum class PrefetchLocality : uint8_t {
    NonTemporal = 0,  // _MM_HINT_NTA — bypass all caches
    L3          = 1,  // _MM_HINT_T2  — prefetch to L3
    L2          = 2,  // _MM_HINT_T1  — prefetch to L2
    L1          = 3   // _MM_HINT_T0  — prefetch to L1 (hottest)
};

/// Prefetch hint configuration (distinct from Pipeline::PrefetchConfig)
struct PrefetchHintConfig {
    PrefetchHintStrategy strategy = PrefetchHintStrategy::Sequential;
    PrefetchLocality locality = PrefetchLocality::L1;
    size_t prefetchDistance    = 4;    // Lines ahead to prefetch
    size_t strideBytes        = 0;    // Stride for Stride strategy
    size_t cacheLineBytes     = 64;   // Cache line size
    bool   enabled            = true;

    /// Create config for file header prefetch (first N bytes)
    static PrefetchHintConfig ForFileHeader(size_t headerSize = 4096) {
        PrefetchHintConfig cfg;
        cfg.strategy = PrefetchHintStrategy::Sequential;
        cfg.locality = PrefetchLocality::L1;
        cfg.prefetchDistance = (headerSize + 63) / 64;
        return cfg;
    }

    /// Create config for pixel row scanning
    static PrefetchHintConfig ForPixelRows(size_t rowBytes, size_t rowsAhead = 4) {
        PrefetchHintConfig cfg;
        cfg.strategy = PrefetchHintStrategy::Stride;
        cfg.locality = PrefetchLocality::L2;
        cfg.strideBytes = rowBytes;
        cfg.prefetchDistance = rowsAhead;
        return cfg;
    }

    /// Create config for cache metadata access
    static PrefetchHintConfig ForCacheMetadata() {
        PrefetchHintConfig cfg;
        cfg.strategy = PrefetchHintStrategy::Sequential;
        cfg.locality = PrefetchLocality::L1;
        cfg.prefetchDistance = 2;
        return cfg;
    }
};

/// Statistics for prefetch hint operations (distinct from Pipeline::PrefetchStats)
struct PrefetchHintStats {
    uint64_t hintCount = 0;       // Total prefetch hints issued
    uint64_t bytesTouched = 0;    // Total bytes prefetched
    uint64_t skippedNull = 0;     // Skipped due to null pointer
    uint64_t skippedDisabled = 0; // Skipped due to disabled config

    void Reset() {
        hintCount = 0;
        bytesTouched = 0;
        skippedNull = 0;
        skippedDisabled = 0;
    }
};

/// CPU feature detection for prefetch support
struct PrefetchCapabilities {
    bool hasSSE    = false;
    bool hasSSE2   = false;
    bool hasPrefetchW = false;  // PREFETCHW (AMD, Intel Broadwell+)

    static PrefetchCapabilities Detect() {
        PrefetchCapabilities caps;
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);
        caps.hasSSE  = (cpuInfo[3] & (1 << 25)) != 0;
        caps.hasSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
        // Extended features for PREFETCHW
        __cpuid(cpuInfo, 0x80000001);
        caps.hasPrefetchW = (cpuInfo[2] & (1 << 8)) != 0;
        return caps;
    }
};

/// Prefetch hint engine — issues CPU prefetch hints for decode pipeline
class PrefetchHintEngine {
public:
    PrefetchHintEngine() : m_caps(PrefetchCapabilities::Detect()) {}

    explicit PrefetchHintEngine(const PrefetchHintConfig& config)
        : m_config(config), m_caps(PrefetchCapabilities::Detect()) {}

    /// Issue prefetch hints for a memory region
    void PrefetchRegion(const void* baseAddr, size_t regionBytes) {
        if (!m_config.enabled) {
            m_stats.skippedDisabled++;
            return;
        }
        if (!baseAddr) {
            m_stats.skippedNull++;
            return;
        }
        if (!m_caps.hasSSE) return;

        const char* ptr = static_cast<const char*>(baseAddr);
        size_t lines = (regionBytes + m_config.cacheLineBytes - 1) / m_config.cacheLineBytes;
        size_t prefetchLines = (std::min)(lines, m_config.prefetchDistance);

        for (size_t i = 0; i < prefetchLines; ++i) {
            IssuePrefetch(ptr + i * m_config.cacheLineBytes);
        }
        m_stats.hintCount += prefetchLines;
        m_stats.bytesTouched += prefetchLines * m_config.cacheLineBytes;
    }

    /// Prefetch file header bytes (first N bytes of a buffer)
    void PrefetchFileHeader(const void* fileData, size_t headerSize = 4096) {
        if (!m_config.enabled || !fileData) {
            if (!fileData) m_stats.skippedNull++;
            else m_stats.skippedDisabled++;
            return;
        }
        if (!m_caps.hasSSE) return;

        const char* ptr = static_cast<const char*>(fileData);
        size_t lines = (headerSize + m_config.cacheLineBytes - 1) / m_config.cacheLineBytes;

        for (size_t i = 0; i < lines; ++i) {
            _mm_prefetch(ptr + i * m_config.cacheLineBytes, _MM_HINT_T0);
        }
        m_stats.hintCount += lines;
        m_stats.bytesTouched += lines * m_config.cacheLineBytes;
    }

    /// Prefetch pixel rows ahead of the current decode position
    void PrefetchPixelRows(const void* rowBase, size_t rowBytes, size_t rowsAhead) {
        if (!m_config.enabled || !rowBase) {
            if (!rowBase) m_stats.skippedNull++;
            else m_stats.skippedDisabled++;
            return;
        }
        if (!m_caps.hasSSE) return;

        const char* ptr = static_cast<const char*>(rowBase);
        for (size_t row = 0; row < rowsAhead; ++row) {
            const char* rowPtr = ptr + row * rowBytes;
            size_t linesPerRow = (rowBytes + m_config.cacheLineBytes - 1) / m_config.cacheLineBytes;
            for (size_t line = 0; line < linesPerRow; ++line) {
                _mm_prefetch(rowPtr + line * m_config.cacheLineBytes, _MM_HINT_T1);
            }
            m_stats.hintCount += linesPerRow;
            m_stats.bytesTouched += linesPerRow * m_config.cacheLineBytes;
        }
    }

    /// Prefetch cache metadata entries
    void PrefetchCacheEntry(const void* entryAddr, size_t entrySize) {
        if (!m_config.enabled || !entryAddr) {
            if (!entryAddr) m_stats.skippedNull++;
            else m_stats.skippedDisabled++;
            return;
        }
        if (!m_caps.hasSSE) return;

        const char* ptr = static_cast<const char*>(entryAddr);
        size_t lines = (entrySize + m_config.cacheLineBytes - 1) / m_config.cacheLineBytes;

        for (size_t i = 0; i < lines; ++i) {
            _mm_prefetch(ptr + i * m_config.cacheLineBytes, _MM_HINT_T0);
        }
        m_stats.hintCount += lines;
        m_stats.bytesTouched += lines * m_config.cacheLineBytes;
    }

    /// Stride-based prefetch for interleaved data
    void PrefetchStrided(const void* baseAddr, size_t stride, size_t count) {
        if (!m_config.enabled || !baseAddr || stride == 0) {
            if (!baseAddr) m_stats.skippedNull++;
            else m_stats.skippedDisabled++;
            return;
        }
        if (!m_caps.hasSSE) return;

        const char* ptr = static_cast<const char*>(baseAddr);
        size_t prefetchCount = (std::min)(count, m_config.prefetchDistance);

        for (size_t i = 0; i < prefetchCount; ++i) {
            IssuePrefetch(ptr + i * stride);
        }
        m_stats.hintCount += prefetchCount;
        m_stats.bytesTouched += prefetchCount * m_config.cacheLineBytes;
    }

    /// Get current configuration
    const PrefetchHintConfig& GetConfig() const { return m_config; }

    /// Update configuration
    void SetConfig(const PrefetchHintConfig& config) { m_config = config; }

    /// Get statistics
    const PrefetchHintStats& GetStats() const { return m_stats; }

    /// Reset statistics
    void ResetStats() { m_stats.Reset(); }

    /// Get detected capabilities
    const PrefetchCapabilities& GetCapabilities() const { return m_caps; }

    /// Get strategy name
    static const char* StrategyName(PrefetchHintStrategy strategy) {
        switch (strategy) {
            case PrefetchHintStrategy::None:       return "None";
            case PrefetchHintStrategy::Sequential: return "Sequential";
            case PrefetchHintStrategy::Stride:     return "Stride";
            case PrefetchHintStrategy::Adaptive:   return "Adaptive";
            default:                               return "Unknown";
        }
    }

private:
    /// Issue a single prefetch hint at the configured locality level
    void IssuePrefetch(const char* addr) {
        switch (m_config.locality) {
            case PrefetchLocality::NonTemporal:
                _mm_prefetch(addr, _MM_HINT_NTA);
                break;
            case PrefetchLocality::L3:
                _mm_prefetch(addr, _MM_HINT_T2);
                break;
            case PrefetchLocality::L2:
                _mm_prefetch(addr, _MM_HINT_T1);
                break;
            case PrefetchLocality::L1:
            default:
                _mm_prefetch(addr, _MM_HINT_T0);
                break;
        }
    }

    PrefetchHintConfig    m_config;
    PrefetchHintStats     m_stats;
    PrefetchCapabilities  m_caps;
};

} // namespace Engine
} // namespace ExplorerLens
