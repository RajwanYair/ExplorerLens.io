// LASPointCloudRenderer.h — LAS/LAZ Point Cloud Top-Down Density Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Renders a top-down (nadir) point density map from LAS 1.0–1.4 / LAZ
// compressed point cloud files. Reads the binary LAS header and point records
// to accumulate XY bins into a density grid, then applies a heat-map palette.
// Supports LAS 1.2 RGB colour attribution when present.
// Target: 256×256 BGRA32 thumbnail; handles up to 10 M points in < 100 ms.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class LASColorMode : uint8_t {
    Density    = 0,   // Heat-map density (default)
    Intensity  = 1,   // Grayscale intensity from LAS Intensity field
    RGB        = 2,   // Use RGB if Format ID ≥ 2
    Elevation  = 3,   // Z-value elevation colormap
};

struct LASRenderOptions {
    uint32_t     outputWidth  = 256;
    uint32_t     outputHeight = 256;
    LASColorMode colorMode    = LASColorMode::Density;
    uint32_t     maxPoints    = 5'000'000;  // Sub-sample if larger
};

struct LASRenderResult {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width        = 0;
    uint32_t height       = 0;
    uint64_t pointsRead   = 0;
    uint64_t totalPoints  = 0;
    double   minZ = 0.0, maxZ = 0.0;
    bool     success      = false;
};

class LASPointCloudRenderer {
public:
    LASPointCloudRenderer()  = default;
    ~LASPointCloudRenderer() = default;

    // Render a LAS/LAZ file from a byte buffer.
    LASRenderResult Render(
        const uint8_t*          lasData,
        size_t                  lasSize,
        const LASRenderOptions& opts = {}) const noexcept;

    // Quick probe: read LAS header to get point count.
    static uint64_t ProbePointCount(
        const uint8_t* lasData, size_t lasSize) noexcept;

    // Check LAS file signature "LASF".
    static bool IsLAS(const uint8_t* data, size_t size) noexcept;
};

}} // namespace ExplorerLens::Engine
