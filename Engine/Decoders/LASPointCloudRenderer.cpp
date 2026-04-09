// LASPointCloudRenderer.cpp — LAS/LAZ Point Cloud Top-Down Density Renderer
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/LASPointCloudRenderer.h"
#include "Decoders/FITSZScaleStretch.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <vector>

namespace ExplorerLens { namespace Engine {

// LAS 1.x Public Header Block (partial — first 107 bytes).
#pragma pack(push, 1)
struct LASPublicHeader {
    char     fileSignature[4];    // "LASF"
    uint16_t fileSourceID;
    uint16_t globalEncoding;
    uint32_t projectID1;
    uint16_t projectID2, projectID3;
    uint8_t  projectID4[8];
    uint8_t  versionMajor, versionMinor;
    char     systemIdentifier[32];
    char     generatingSoftware[32];
    uint16_t fileCreationDay, fileCreationYear;
    uint16_t headerSize;
    uint32_t offsetToPointData;
    uint32_t numVarLenRecords;
    uint8_t  pointDataFormatID;
    uint16_t pointDataRecordLength;
    uint32_t legacyNumPoints;
    uint32_t legacyNumPointsByReturn[5];
    double   xScale, yScale, zScale;
    double   xOffset, yOffset, zOffset;
    double   maxX, minX, maxY, minY, maxZ, minZ;
};
#pragma pack(pop)

bool LASPointCloudRenderer::IsLAS(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 4) return false;
    return std::memcmp(data, "LASF", 4) == 0;
}

uint64_t LASPointCloudRenderer::ProbePointCount(
    const uint8_t* lasData, size_t lasSize) noexcept
{
    if (!IsLAS(lasData, lasSize) || lasSize < sizeof(LASPublicHeader)) return 0;
    const auto* h = reinterpret_cast<const LASPublicHeader*>(lasData);
    return h->legacyNumPoints;
}

LASRenderResult LASPointCloudRenderer::Render(
    const uint8_t*          lasData,
    size_t                  lasSize,
    const LASRenderOptions& opts) const noexcept
{
    LASRenderResult result{};
    if (!IsLAS(lasData, lasSize) || lasSize < sizeof(LASPublicHeader)) return result;

    const auto* h = reinterpret_cast<const LASPublicHeader*>(lasData);
    const uint64_t totalPts = h->legacyNumPoints;
    if (totalPts == 0 || h->pointDataRecordLength == 0) return result;

    const uint32_t W = opts.outputWidth;
    const uint32_t H = opts.outputHeight;

    const double rangeX = h->maxX - h->minX;
    const double rangeY = h->maxY - h->minY;
    if (rangeX < 1e-10 || rangeY < 1e-10) return result;

    result.minZ = h->minZ; result.maxZ = h->maxZ;

    // Sub-sample stride to cap at maxPoints.
    const uint64_t stride = std::max(uint64_t{1}, totalPts / opts.maxPoints);

    std::vector<uint32_t> density(static_cast<size_t>(W) * H, 0u);
    uint32_t densityMax = 0;

    const uint8_t* ptr    = lasData + h->offsetToPointData;
    const uint8_t* endPtr = lasData + lasSize;
    const uint16_t recLen = h->pointDataRecordLength;
    uint64_t count = 0;

    for (uint64_t i = 0; i < totalPts && ptr + recLen <= endPtr; i += stride, ptr += recLen * stride) {
        int32_t rawX, rawY;
        std::memcpy(&rawX, ptr,     4);
        std::memcpy(&rawY, ptr + 4, 4);
        const double wx = rawX * h->xScale + h->xOffset;
        const double wy = rawY * h->yScale + h->yOffset;
        const uint32_t px = static_cast<uint32_t>((wx - h->minX) / rangeX * (W - 1));
        const uint32_t py = static_cast<uint32_t>((wy - h->minY) / rangeY * (H - 1));
        if (px < W && py < H) {
            const uint32_t d = ++density[(H - 1 - py) * W + px];
            if (d > densityMax) densityMax = d;
        }
        ++count;
    }

    result.pointsRead  = count;
    result.totalPoints = totalPts;
    result.width  = W;
    result.height = H;
    result.pixelsBGRA.resize(static_cast<size_t>(W) * H * 4u, 0);

    if (densityMax == 0) return result;

    uint8_t* dst = result.pixelsBGRA.data();
    for (uint32_t i = 0; i < W * H; ++i) {
        if (density[i] > 0) {
            const float t = static_cast<float>(density[i]) / static_cast<float>(densityMax);
            const uint32_t bgra = FITSZScaleStretch::HeatMapBGRA(t);
            std::memcpy(dst + i * 4, &bgra, 4);
        } else {
            dst[i*4+0] = dst[i*4+1] = dst[i*4+2] = 0x10;
            dst[i*4+3] = 0xFF;
        }
    }

    result.success = true;
    return result;
}

}} // namespace ExplorerLens::Engine
