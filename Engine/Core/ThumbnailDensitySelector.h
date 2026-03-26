// ThumbnailDensitySelector.h — Per-DPI Thumbnail Density Cache Key Selector
// Copyright (c) 2026 ExplorerLens Project
//
// Chooses the optimal physical pixel size and cache key suffix for a thumbnail
// request based on the target monitor's DPI, scale factor, and shell hint.
//
#pragma once

#include <cstdint>
#include <string>
#include "DPIScalingPolicy.h"

namespace ExplorerLens {
namespace Engine {

// ---- Thumbnail Density Request ----------------------------------------------

struct DensityRequest {
    uint32_t logicalSize   = 256;   // Shell-requested logical size (px)
    uint32_t monitorDPI    = 96;    // DPI of the target monitor
    float    scaleFactor   = 1.0f;  // Pre-computed (monitorDPI / 96.0)
    bool     isHDRDisplay  = false;
    std::string filePath;           // Used as cache key base
};

// ---- Density Selection Result -----------------------------------------------

struct DensityResult {
    uint32_t physicalWidth    = 256;
    uint32_t physicalHeight   = 256;
    std::string cacheKey;           // "<hash>@<scaleSuffix>_<sizeN>"
    DPIBucket   dpiBucket     = DPIBucket::DPI_96;
    bool        needsUpscale  = false;  // src < dst — invoke HiDPIScaler
};

// ---- ThumbnailDensitySelector -----------------------------------------------

class ThumbnailDensitySelector {
public:
    explicit ThumbnailDensitySelector(DPIScalingPolicy policy = DPIScalingPolicy::Default());

    // Select optimal physical size + cache key for a density request.
    DensityResult Select(const DensityRequest& req) const;

    // Build a stable cache key from a file path + physical size + DPI bucket.
    static std::string BuildCacheKey(
        const std::string& filePath,
        uint32_t           physicalSize,
        DPIBucket          bucket);

    // Check if an existing cached size can serve a new DPI request (±10% tolerance).
    static bool CanServeRequest(
        uint32_t cachedPhysicalSize,
        uint32_t requestedPhysicalSize,
        float    toleranceFraction = 0.10f);

    void SetPolicy(DPIScalingPolicy policy) { m_policy = policy; }
    const DPIScalingPolicy& Policy() const  { return m_policy; }

private:
    DPIScalingPolicy m_policy;
};

} // namespace Engine
} // namespace ExplorerLens
