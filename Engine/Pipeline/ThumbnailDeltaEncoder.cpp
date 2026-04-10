// ThumbnailDeltaEncoder.cpp — Thumbnail Delta Encoder
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailDeltaEncoder.h"
#include <cstring>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

ThumbnailDeltaEncoder::ThumbnailDeltaEncoder(const Config& cfg)
    : m_config(cfg)
{}

ThumbnailDelta ThumbnailDeltaEncoder::Encode(
    const uint8_t* prevBGRA,
    const uint8_t* currBGRA,
    uint32_t width, uint32_t height) const
{
    ThumbnailDelta delta;
    delta.width  = width;
    delta.height = height;

    if (!prevBGRA || !currBGRA || width == 0 || height == 0) {
        delta.isEmpty = true;
        return delta;
    }

    // Find bounding box of changed pixels.
    uint32_t minX = width, minY = height, maxX = 0, maxY = 0;
    uint32_t dirtyCount = 0;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint32_t idx = (y * width + x) * 4;
            if (prevBGRA[idx]   != currBGRA[idx]   ||
                prevBGRA[idx+1] != currBGRA[idx+1] ||
                prevBGRA[idx+2] != currBGRA[idx+2] ||
                prevBGRA[idx+3] != currBGRA[idx+3])
            {
                ++dirtyCount;
                if (x < minX) minX = x;
                if (y < minY) minY = y;
                if (x > maxX) maxX = x;
                if (y > maxY) maxY = y;
            }
        }
    }

    if (dirtyCount < m_config.minDirtyPixels) {
        delta.isEmpty = true;
        return delta;
    }

    delta.dirtyX = minX;
    delta.dirtyY = minY;
    delta.dirtyW = maxX - minX + 1;
    delta.dirtyH = maxY - minY + 1;
    delta.codec  = DeltaCodec::NONE;  // production: LZMA/ZSTD compress

    // Copy the dirty region as the raw payload.
    delta.payload.reserve(delta.dirtyW * delta.dirtyH * 4);
    for (uint32_t y = minY; y <= maxY; ++y) {
        const uint32_t rowOff = (y * width + minX) * 4;
        delta.payload.insert(
            delta.payload.end(),
            currBGRA + rowOff,
            currBGRA + rowOff + delta.dirtyW * 4);
    }

    const uint64_t fullSize = static_cast<uint64_t>(width) * height * 4;
    m_totalEncoded += fullSize;
    m_totalSaved   += fullSize - delta.payload.size();
    return delta;
}

bool ThumbnailDeltaEncoder::Decode(
    uint8_t*           targetBGRA,
    uint32_t           width, uint32_t height,
    const ThumbnailDelta& delta) const
{
    if (delta.isEmpty) return true;
    if (delta.width != width || delta.height != height) return false;
    if (delta.payload.empty()) return false;

    // Restore the dirty region into targetBGRA.
    const uint8_t* src = delta.payload.data();
    for (uint32_t y = delta.dirtyY; y < delta.dirtyY + delta.dirtyH; ++y) {
        if (y >= height) break;
        const uint32_t rowOff = (y * width + delta.dirtyX) * 4;
        const uint32_t rowBytes = (std::min)(delta.dirtyW, width - delta.dirtyX) * 4;
        if (rowOff + rowBytes > width * height * 4) break;
        memcpy(targetBGRA + rowOff, src, rowBytes);
        src += delta.dirtyW * 4;
    }
    return true;
}

uint64_t ThumbnailDeltaEncoder::TotalBytesEncoded() const { return m_totalEncoded; }
uint64_t ThumbnailDeltaEncoder::TotalBytesSaved()   const { return m_totalSaved;   }
void     ThumbnailDeltaEncoder::ResetStats()               { m_totalEncoded = 0; m_totalSaved = 0; }
const    ThumbnailDeltaEncoder::Config& ThumbnailDeltaEncoder::GetConfig() const { return m_config; }

}} // namespace ExplorerLens::Engine
