// SSIMComparator.h — Structural Similarity Index Measure for decoder output validation
// Copyright (c) 2026 ExplorerLens Project
//
// Computes SSIM and MS-SSIM between two BGRA32 thumbnail buffers.
// Used by FormatValidationRunner to detect regressions in decoder output.
//
#pragma once
#include <cstdint>
#include <span>
#include <stdexcept>

namespace ExplorerLens {
namespace Engine {

struct SSIMResult {
    float luminance;    // similarity in luminance     (0–1)
    float contrast;     // similarity in contrast      (0–1)
    float structure;    // similarity in structure     (0–1)
    float ssim;         // overall SSIM = l * c * s    (0–1)
    bool  isIdentical;  // ssim >= 0.999
};

// Thresholds for pass/fail in regression tests
constexpr float SSIM_PASS_THRESHOLD     = 0.95f;
constexpr float SSIM_IDENTICAL_THRESHOLD = 0.999f;

class SSIMComparator {
public:
    // Compare two BGRA32 buffers of the same dimensions.
    // Throws std::invalid_argument if sizes don't match or are too small.
    // windowSize: Gaussian window (default 11, must be odd)
    static SSIMResult Compare(std::span<const uint8_t> a,
                              std::span<const uint8_t> b,
                              uint32_t width,
                              uint32_t height,
                              uint32_t windowSize = 11);

    // Compute per-channel SSIM separately for R, G, B (ignores alpha)
    struct ChannelSSIM { float r, g, b, luminance; };
    static ChannelSSIM CompareChannels(std::span<const uint8_t> a,
                                       std::span<const uint8_t> b,
                                       uint32_t width,
                                       uint32_t height,
                                       uint32_t windowSize = 11);

    // Returns true if SSIM passes the given threshold
    static bool PassesThreshold(std::span<const uint8_t> a,
                                 std::span<const uint8_t> b,
                                 uint32_t width, uint32_t height,
                                 float threshold = SSIM_PASS_THRESHOLD);

private:
    // Build a 1-D Gaussian kernel of size n and standard deviation sigma
    static std::vector<float> GaussianKernel(uint32_t n, float sigma = 1.5f);

    // Single-channel SSIM on a float luminance plane
    static float SSIMSingleChannel(const std::vector<float>& plane1,
                                   const std::vector<float>& plane2,
                                   uint32_t width, uint32_t height,
                                   std::span<const float> kernel,
                                   uint32_t kSize);
};

} // namespace Engine
} // namespace ExplorerLens
