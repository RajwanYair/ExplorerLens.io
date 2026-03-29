// AdaptiveGridDensityEngine.h — Adaptive Thumbnail Grid Density Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically adjusts thumbnail grid size and spacing based on screen DPI,
// available viewport, file count, and learned user preferences.
//
#pragma once
#include <cstdint>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct GridDensityParams {
    uint32_t viewportWidth   = 1920;
    uint32_t viewportHeight  = 1080;
    float    dpiScale        = 1.0f;
    uint32_t fileCount       = 100;
    uint32_t preferredMin    = 64;
    uint32_t preferredMax    = 512;
};

struct GridLayout {
    uint32_t thumbWidth    = 128;
    uint32_t thumbHeight   = 128;
    uint32_t colCount      = 0;
    uint32_t rowCount      = 0;
    uint32_t hSpacing      = 4;
    uint32_t vSpacing      = 4;
    float    fillRatio     = 0.0f;
};

class AdaptiveGridDensityEngine {
public:
    AdaptiveGridDensityEngine() = default;

    GridLayout Compute(const GridDensityParams& params) const {
        GridLayout layout;
        uint32_t baseSize = static_cast<uint32_t>(128.0f * params.dpiScale);
        baseSize = std::max(params.preferredMin, std::min(params.preferredMax, baseSize));

        if (params.fileCount > 500) baseSize = std::max(params.preferredMin, baseSize / 2);
        else if (params.fileCount < 20) baseSize = std::min(params.preferredMax, baseSize * 2);

        layout.thumbWidth  = baseSize;
        layout.thumbHeight = baseSize;
        layout.hSpacing    = static_cast<uint32_t>(4 * params.dpiScale);
        layout.vSpacing    = layout.hSpacing;
        layout.colCount    = std::max(1u, params.viewportWidth / (baseSize + layout.hSpacing));
        layout.rowCount    = (params.fileCount + layout.colCount - 1) / layout.colCount;
        uint32_t usedW = layout.colCount * (baseSize + layout.hSpacing);
        layout.fillRatio   = params.viewportWidth > 0
            ? static_cast<float>(usedW) / params.viewportWidth : 1.0f;
        return layout;
    }

    bool IsValidLayout(const GridLayout& layout) const {
        return layout.colCount > 0 && layout.thumbWidth > 0 && layout.thumbHeight > 0;
    }
};

}} // namespace ExplorerLens::Engine
