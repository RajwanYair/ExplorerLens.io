// LoopTuner.h — Compile-time and runtime loop optimization helpers
// Copyright (c) 2026 ExplorerLens Project
//
// Provides MSVC loop unrolling hints, UNROLL macros, cache-oblivious tiling
// for image row processing (TileProcessor), and operation batching for
// reducing function call overhead in the hot decode pipeline.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace ExplorerLens {
namespace Pipeline {

// ============================================================================
// MSVC Loop Optimization Hints
// ============================================================================

/// Hint MSVC to parallelize the following loop with N iterations
/// Usage: LENS_LOOP_PARALLEL(8) for (...) { ... }
#ifdef _MSC_VER
    #define LENS_LOOP_PARALLEL(N) __pragma(loop(hint_parallel(N)))
    #define LENS_LOOP_NO_VECTORIZE __pragma(loop(no_vector))
    #define LENS_LOOP_IVDEP __pragma(loop(ivdep))
#else
    #define LENS_LOOP_PARALLEL(N)
    #define LENS_LOOP_NO_VECTORIZE
    #define LENS_LOOP_IVDEP
#endif

// ============================================================================
// Manual Unroll Macros
// ============================================================================

/// Unroll a loop body 2 times
/// Usage: LENS_UNROLL_2(i, count, { dst[i] = src[i] * scale; })
#define LENS_UNROLL_2(var, limit, body)                \
    {                                                  \
        size_t var##_end2 = (limit) & ~size_t(1);      \
        size_t var = 0;                                \
        for (; var < var##_end2; var += 2) {           \
            {                                          \
                body                                   \
            }                                          \
            {                                          \
                size_t var##_save = var;               \
                var = var##_save + 1;                  \
                body var = var##_save;                 \
            }                                          \
        }                                              \
        for (var = var##_end2; var < (limit); ++var) { \
            body                                       \
        }                                              \
    }

/// Unroll a loop body 4 times — processes 4 elements per iteration
#define LENS_UNROLL_4(idx, count, body)                \
    do {                                               \
        const size_t _u4_count = (count);              \
        const size_t _u4_end = _u4_count & ~size_t(3); \
        size_t idx = 0;                                \
        for (; idx < _u4_end; idx += 4) {              \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {body} idx -= 3;                           \
        }                                              \
        for (idx = _u4_end; idx < _u4_count; ++idx) {  \
            body                                       \
        }                                              \
    } while (0)

/// Unroll a loop body 8 times — maximum throughput for tight loops
#define LENS_UNROLL_8(idx, count, body)                \
    do {                                               \
        const size_t _u8_count = (count);              \
        const size_t _u8_end = _u8_count & ~size_t(7); \
        size_t idx = 0;                                \
        for (; idx < _u8_end; idx += 8) {              \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {                                          \
                body                                   \
            }                                          \
            ++idx;                                     \
            {body} idx -= 7;                           \
        }                                              \
        for (idx = _u8_end; idx < _u8_count; ++idx) {  \
            body                                       \
        }                                              \
    } while (0)

// ============================================================================
// Cache-Oblivious Tiling for Image Row Processing
// ============================================================================

/// Tile configuration for cache-oblivious processing
struct TileConfig
{
    size_t tileWidth = 64;         // Pixels per tile (X)
    size_t tileHeight = 64;        // Rows per tile (Y)
    size_t cacheLineBytes = 64;    // Cache line size
    size_t l1CacheBytes = 32768;   // L1 cache size (32 KB)
    size_t l2CacheBytes = 262144;  // L2 cache size (256 KB)

    /// Auto-configure tile size based on pixel format
    static TileConfig ForPixelFormat(size_t bytesPerPixel, size_t imageWidth)
    {
        TileConfig cfg;
        // Target: tile fits in L1 cache
        size_t rowBytes = imageWidth * bytesPerPixel;
        if (rowBytes > 0) {
            cfg.tileWidth = (std::min)(imageWidth, cfg.l1CacheBytes / (bytesPerPixel * cfg.tileHeight));
            if (cfg.tileWidth < 8)
                cfg.tileWidth = 8;
            // Round to cache-line-friendly boundary
            size_t pixelsPerLine = cfg.cacheLineBytes / bytesPerPixel;
            if (pixelsPerLine > 0) {
                cfg.tileWidth = ((cfg.tileWidth + pixelsPerLine - 1) / pixelsPerLine) * pixelsPerLine;
            }
        }
        return cfg;
    }
};

/// Tile descriptor passed to processing callbacks
struct TileRegion
{
    size_t startX = 0;
    size_t startY = 0;
    size_t width = 0;
    size_t height = 0;
};

/// Statistics for tile processing
struct TileProcessorStats
{
    uint64_t tilesProcessed = 0;
    uint64_t totalPixels = 0;
    uint64_t totalRows = 0;

    void Reset()
    {
        tilesProcessed = 0;
        totalPixels = 0;
        totalRows = 0;
    }
};

/// Callback type for processing a tile
using TileCallback = std::function<void(const TileRegion&)>;

/// Cache-oblivious tile processor for image data
class TileProcessor
{
  public:
    TileProcessor() = default;
    explicit TileProcessor(const TileConfig& config) : m_config(config) {}

    /// Process an image in tiles for cache-friendly access
    void ProcessImage(size_t imageWidth, size_t imageHeight, const TileCallback& callback)
    {
        if (!callback || imageWidth == 0 || imageHeight == 0)
            return;

        for (size_t tileY = 0; tileY < imageHeight; tileY += m_config.tileHeight) {
            size_t tH = (std::min)(m_config.tileHeight, imageHeight - tileY);
            for (size_t tileX = 0; tileX < imageWidth; tileX += m_config.tileWidth) {
                size_t tW = (std::min)(m_config.tileWidth, imageWidth - tileX);
                TileRegion region{tileX, tileY, tW, tH};
                callback(region);
                m_stats.tilesProcessed++;
                m_stats.totalPixels += tW * tH;
            }
            m_stats.totalRows += tH;
        }
    }

    /// Process rows in batches for simple row-oriented processing
    void ProcessRowBatches(size_t totalRows, size_t batchSize,
                           const std::function<void(size_t startRow, size_t rowCount)>& callback)
    {
        if (!callback || totalRows == 0)
            return;
        if (batchSize == 0)
            batchSize = m_config.tileHeight;

        for (size_t row = 0; row < totalRows; row += batchSize) {
            size_t count = (std::min)(batchSize, totalRows - row);
            callback(row, count);
            m_stats.totalRows += count;
        }
    }

    /// Get tile configuration
    const TileConfig& GetConfig() const
    {
        return m_config;
    }

    /// Set tile configuration
    void SetConfig(const TileConfig& config)
    {
        m_config = config;
    }

    /// Get processing statistics
    const TileProcessorStats& GetStats() const
    {
        return m_stats;
    }

    /// Reset statistics
    void ResetStats()
    {
        m_stats.Reset();
    }

    /// Calculate optimal tile count for an image
    static size_t CalculateTileCount(size_t imageWidth, size_t imageHeight, const TileConfig& config)
    {
        size_t tilesX = (imageWidth + config.tileWidth - 1) / config.tileWidth;
        size_t tilesY = (imageHeight + config.tileHeight - 1) / config.tileHeight;
        return tilesX * tilesY;
    }

  private:
    TileConfig m_config;
    TileProcessorStats m_stats;
};

// ============================================================================
// Operation Batching — Reduce Function Call Overhead
// ============================================================================

/// Batched operation descriptor
struct BatchOp
{
    uint32_t opType = 0;  // Operation type tag
    size_t offset = 0;    // Offset into data buffer
    size_t length = 0;    // Number of elements
};

/// Operation batcher — collects operations and executes them in bulk
template <size_t MaxOps = 64>
class OperationBatcher
{
  public:
    OperationBatcher() = default;

    /// Queue an operation for batched execution
    bool Enqueue(uint32_t opType, size_t offset, size_t length)
    {
        if (m_count >= MaxOps)
            return false;
        m_ops[m_count] = {opType, offset, length};
        m_count++;
        return true;
    }

    /// Execute all queued operations via callback
    void Flush(const std::function<void(const BatchOp&)>& executor)
    {
        if (!executor) {
            m_count = 0;
            return;
        }
        for (size_t i = 0; i < m_count; ++i) {
            executor(m_ops[i]);
        }
        m_totalFlushed += m_count;
        m_flushCount++;
        m_count = 0;
    }

    /// Get pending operation count
    size_t Pending() const
    {
        return m_count;
    }

    /// Get total operations flushed
    uint64_t TotalFlushed() const
    {
        return m_totalFlushed;
    }

    /// Get number of flush calls
    uint64_t FlushCount() const
    {
        return m_flushCount;
    }

    /// Check if batch is full
    bool IsFull() const
    {
        return m_count >= MaxOps;
    }

    /// Clear without executing
    void Clear()
    {
        m_count = 0;
    }

    /// Maximum batch size
    static constexpr size_t MaxBatchSize()
    {
        return MaxOps;
    }

  private:
    std::array<BatchOp, MaxOps> m_ops{};
    size_t m_count = 0;
    uint64_t m_totalFlushed = 0;
    uint64_t m_flushCount = 0;
};

}  // namespace Pipeline
}  // namespace ExplorerLens
