// ChunkedDecodeEngine.h — Segmented Image Decode Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes large images in horizontal strips, enabling progressive rendering
// and reducing peak memory by processing one chunk at a time.
//
#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ChunkDescriptor
{
    uint32_t offsetY = 0;
    uint32_t height = 0;
    uint32_t strideBytes = 0;
    const uint8_t* data = nullptr;
};

struct ChunkedDecodeConfig
{
    uint32_t chunkHeight = 64;
    uint32_t maxPendingChunks = 4;
    bool enableParallelChunks = false;
    uint64_t memoryBudgetBytes = 32ULL * 1024 * 1024;
};

struct ChunkedDecodeStats
{
    uint32_t totalChunks = 0;
    uint32_t completedChunks = 0;
    uint32_t failedChunks = 0;
    uint64_t bytesProcessed = 0;
    double avgChunkMs = 0.0;
};

class ChunkedDecodeEngine
{
  public:
    using ChunkCallback = std::function<void(const ChunkDescriptor&)>;

    void Configure(const ChunkedDecodeConfig& config)
    {
        m_config = config;
    }

    uint32_t CalculateChunkCount(uint32_t imageHeight) const
    {
        if (m_config.chunkHeight == 0)
            return 1;
        return (imageHeight + m_config.chunkHeight - 1) / m_config.chunkHeight;
    }

    ChunkDescriptor GetChunk(uint32_t index, uint32_t imageWidth, uint32_t imageHeight, uint32_t bpp,
                             const uint8_t* fullData) const
    {
        ChunkDescriptor cd;
        cd.offsetY = index * m_config.chunkHeight;
        cd.height =
            (cd.offsetY + m_config.chunkHeight > imageHeight) ? (imageHeight - cd.offsetY) : m_config.chunkHeight;
        cd.strideBytes = imageWidth * (bpp / 8);
        cd.data = fullData + static_cast<size_t>(cd.offsetY) * cd.strideBytes;
        return cd;
    }

    bool FitsInBudget(uint32_t imageWidth, uint32_t bpp) const
    {
        uint64_t chunkBytes =
            static_cast<uint64_t>(imageWidth) * (bpp / 8) * m_config.chunkHeight * m_config.maxPendingChunks;
        return chunkBytes <= m_config.memoryBudgetBytes;
    }

    ChunkedDecodeStats GetStats() const
    {
        return m_stats;
    }

  private:
    ChunkedDecodeConfig m_config;
    ChunkedDecodeStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
