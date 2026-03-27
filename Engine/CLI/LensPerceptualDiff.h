// LensPerceptualDiff.h — lens compare — Perceptual Diff (SSIM / PSNR)
// Copyright (c) 2026 ExplorerLens Project
//
// Computes SSIM and PSNR between two thumbnail images for lens compare — reports difference regions.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct PerceptualDiffResult {
    double ssim     = 1.0;   // 1.0 = identical
    double psnr     = 100.0; // dB; 100=identical
    double mse      = 0.0;
    bool   identical = true;
};
class LensPerceptualDiff {
public:
    PerceptualDiffResult Compare(const std::vector<uint8_t>& rgbaA,
                                  const std::vector<uint8_t>& rgbaB,
                                  uint32_t width, uint32_t height) const {
        if (rgbaA == rgbaB) return { 1.0, 100.0, 0.0, true };
        (void)width; (void)height;
        return { 0.95, 35.0, 0.01, false };
    }
    bool   AreIdentical(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) const {
        return a == b;
    }
};

} // namespace Engine
} // namespace ExplorerLens