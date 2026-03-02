//==============================================================================
// ExplorerLens Engine — Smart Crop Engine
//
// Purpose:
//   Intelligent image cropping that preserves the most visually important
//   region of an image.  Uses a multi-map saliency pipeline (luminance,
//   contrast, edge, color uniqueness, center bias, skin-tone detection) to
//   score candidate crop rectangles and select the one with the highest
//   aggregate saliency.
//
// Classes:
//   SmartCropEngine — Thread-safe crop finder with configurable center-bias
//   weight, multi-scale evaluation, and top-N crop retrieval.
//
// Inputs:
//   - RGBA pixel buffer (uint8_t*), source width and height
//   - Target crop dimensions (targetWidth, targetHeight)
//
// Outputs:
//   - CropRect (x, y, w, h, score) for the best crop rectangle
//   - Vector of top-N CropRect candidates
//   - CropStats (count, avg saliency, avg time)
//
// Thread Safety:
//   All stateful counters are protected by SRWLOCK.
//
// Build:
//   Header-only, C++20, MSVC /W4 clean, no external dependencies.
//==============================================================================
#pragma once

#include <windows.h>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

/// A candidate crop rectangle with its saliency score.
struct CropRect {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    float    score = 0.0f;
};

/// Cumulative crop-operation statistics.
struct CropStats {
    uint64_t cropsComputed = 0;
    double   totalSaliency = 0.0;
    double   totalComputeMs = 0.0;
    double AvgSaliency()  const { return cropsComputed ? totalSaliency / static_cast<double>(cropsComputed) : 0.0; }
    double AvgComputeMs() const { return cropsComputed ? totalComputeMs / static_cast<double>(cropsComputed) : 0.0; }
};

/// Intelligent saliency-based image cropper.
class SmartCropEngine {
public:
    SmartCropEngine() {
        InitializeSRWLock(&m_statsLock);
    }

    /// Set center-bias weight (default 0.15). Must be in [0, 1].
    inline void SetCenterBias(float weight) {
        m_centerBiasWeight = (std::max)(0.0f, (std::min)(weight, 1.0f));
    }

    /// Find the single best crop rectangle.
    inline CropRect FindBestCrop(const uint8_t* rgbaData,
        uint32_t srcWidth, uint32_t srcHeight,
        uint32_t targetWidth, uint32_t targetHeight) {
        auto crops = FindTopCrops(rgbaData, srcWidth, srcHeight, targetWidth, targetHeight, 1);
        if (crops.empty()) return CropRect{ 0, 0, targetWidth, targetHeight, 0.0f };
        return crops.front();
    }

    /// Find the top-N crop rectangles ranked by saliency score.
    inline std::vector<CropRect> FindTopCrops(const uint8_t* rgbaData,
        uint32_t srcW, uint32_t srcH,
        uint32_t targetW, uint32_t targetH,
        uint32_t count) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        std::vector<CropRect> results;
        if (!rgbaData || srcW == 0 || srcH == 0 || targetW == 0 || targetH == 0)
            return results;

        // Ensure target aspect ratio fits in source
        float targetAspect = static_cast<float>(targetW) / static_cast<float>(targetH);
        uint32_t cropW, cropH;
        if (static_cast<float>(srcW) / static_cast<float>(srcH) > targetAspect) {
            cropH = srcH;
            cropW = static_cast<uint32_t>(srcH * targetAspect);
            cropW = (std::min)(cropW, srcW);
        }
        else {
            cropW = srcW;
            cropH = static_cast<uint32_t>(srcW / targetAspect);
            cropH = (std::min)(cropH, srcH);
        }
        if (cropW == 0 || cropH == 0) return results;

        // Build saliency map at quarter resolution for speed
        uint32_t qW = (std::max)(srcW / 4, 1u);
        uint32_t qH = (std::max)(srcH / 4, 1u);
        std::vector<float> saliency = BuildSaliencyMap(rgbaData, srcW, srcH, qW, qH);

        // Sliding window search on quarter-res saliency map
        uint32_t qCropW = (std::max)(cropW / 4, 1u);
        uint32_t qCropH = (std::max)(cropH / 4, 1u);
        qCropW = (std::min)(qCropW, qW);
        qCropH = (std::min)(qCropH, qH);

        uint32_t stepX = (std::max)((qW - qCropW) / 16, 1u);
        uint32_t stepY = (std::max)((qH - qCropH) / 16, 1u);

        struct Candidate { uint32_t qx, qy; float score; };
        std::vector<Candidate> candidates;
        candidates.reserve(256);

        for (uint32_t qy = 0; qy + qCropH <= qH; qy += stepY) {
            for (uint32_t qx = 0; qx + qCropW <= qW; qx += stepX) {
                float s = SumRegion(saliency, qW, qx, qy, qCropW, qCropH);
                candidates.push_back({ qx, qy, s });
            }
        }

        // Sort descending by score
        std::sort(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

        uint32_t n = (std::min)(count, static_cast<uint32_t>(candidates.size()));
        for (uint32_t i = 0; i < n; ++i) {
            CropRect cr;
            cr.x = (std::min)(candidates[i].qx * 4, srcW - cropW);
            cr.y = (std::min)(candidates[i].qy * 4, srcH - cropH);
            cr.width = cropW;
            cr.height = cropH;
            cr.score = candidates[i].score;
            results.push_back(cr);
        }

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.cropsComputed++;
        if (!results.empty()) m_stats.totalSaliency += results.front().score;
        m_stats.totalComputeMs += ms;
        ReleaseSRWLockExclusive(&m_statsLock);

        return results;
    }

    /// Retrieve cumulative statistics.
    inline CropStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        CropStats copy = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        return copy;
    }

private:
    // ---- Internal saliency-map construction --------------------------------

    /// Build a combined saliency map at (qW x qH) resolution.
    inline std::vector<float> BuildSaliencyMap(const uint8_t* rgba,
        uint32_t srcW, uint32_t srcH,
        uint32_t qW, uint32_t qH) const {
        const uint32_t qCount = qW * qH;
        std::vector<float> gray(qCount, 0.0f);
        std::vector<float> contrastMap(qCount, 0.0f);
        std::vector<float> edgeMap(qCount, 0.0f);
        std::vector<float> colorUniq(qCount, 0.0f);
        std::vector<float> centerBias(qCount, 0.0f);
        std::vector<float> skinMap(qCount, 0.0f);

        // Mean color of entire image (for color uniqueness)
        double meanR = 0.0, meanG = 0.0, meanB = 0.0;
        uint32_t srcPixels = srcW * srcH;
        // Sample sparsely for speed
        uint32_t sampleStep = (std::max)(srcPixels / 4096, 1u);
        uint32_t sampleCount = 0;
        for (uint32_t i = 0; i < srcPixels; i += sampleStep) {
            uint32_t off = i * 4;
            meanR += rgba[off + 0];
            meanG += rgba[off + 1];
            meanB += rgba[off + 2];
            ++sampleCount;
        }
        if (sampleCount > 0) {
            meanR /= sampleCount;
            meanG /= sampleCount;
            meanB /= sampleCount;
        }

        float diagHalf = 0.3f * std::sqrt(static_cast<float>(qW * qW + qH * qH));
        float invDiag2 = (diagHalf > 0.0f) ? 1.0f / (2.0f * diagHalf * diagHalf) : 0.0f;
        float cx = qW * 0.5f;
        float cy = qH * 0.5f;

        // Fill per-pixel maps at quarter resolution
        for (uint32_t qy = 0; qy < qH; ++qy) {
            for (uint32_t qx = 0; qx < qW; ++qx) {
                // Map back to source coordinates
                uint32_t sx = (std::min)(qx * srcW / qW, srcW - 1);
                uint32_t sy = (std::min)(qy * srcH / qH, srcH - 1);
                uint32_t off = (sy * srcW + sx) * 4;
                float r = rgba[off + 0];
                float g = rgba[off + 1];
                float b = rgba[off + 2];
                uint32_t idx = qy * qW + qx;

                // 1. Luminance
                gray[idx] = 0.299f * r + 0.587f * g + 0.114f * b;

                // 4. Color uniqueness: distance from mean color
                float dr = r - static_cast<float>(meanR);
                float dg = g - static_cast<float>(meanG);
                float db = b - static_cast<float>(meanB);
                colorUniq[idx] = std::sqrt(dr * dr + dg * dg + db * db) / 441.7f; // max = sqrt(3*255^2)

                // 5. Center bias: Gaussian
                float dx = static_cast<float>(qx) - cx;
                float dy = static_cast<float>(qy) - cy;
                centerBias[idx] = std::exp(-(dx * dx + dy * dy) * invDiag2);

                // 6. Skin-tone detection (YCbCr range)
                float Y = 0.299f * r + 0.587f * g + 0.114f * b;
                float Cb = 128.0f - 0.168736f * r - 0.331264f * g + 0.5f * b;
                float Cr = 128.0f + 0.5f * r - 0.418688f * g - 0.081312f * b;
                (void)Y;
                bool isSkin = (Cb >= 77.0f && Cb <= 127.0f && Cr >= 133.0f && Cr <= 173.0f);
                skinMap[idx] = isSkin ? 1.0f : 0.0f;
            }
        }

        // 2. Contrast map: local standard deviation in 5x5 neighborhood
        for (uint32_t qy = 2; qy + 2 < qH; ++qy) {
            for (uint32_t qx = 2; qx + 2 < qW; ++qx) {
                float sum = 0.0f, sumSq = 0.0f;
                for (int ky = -2; ky <= 2; ++ky) {
                    for (int kx = -2; kx <= 2; ++kx) {
                        float v = gray[(qy + ky) * qW + (qx + kx)];
                        sum += v;
                        sumSq += v * v;
                    }
                }
                float mean = sum / 25.0f;
                float variance = sumSq / 25.0f - mean * mean;
                contrastMap[qy * qW + qx] = std::sqrt((std::max)(variance, 0.0f)) / 128.0f;
            }
        }

        // 3. Edge map: Sobel X + Sobel Y magnitude
        for (uint32_t qy = 1; qy + 1 < qH; ++qy) {
            for (uint32_t qx = 1; qx + 1 < qW; ++qx) {
                float gx = -gray[(qy - 1) * qW + (qx - 1)] - 2.0f * gray[qy * qW + (qx - 1)] - gray[(qy + 1) * qW + (qx - 1)]
                    + gray[(qy - 1) * qW + (qx + 1)] + 2.0f * gray[qy * qW + (qx + 1)] + gray[(qy + 1) * qW + (qx + 1)];
                float gy = -gray[(qy - 1) * qW + (qx - 1)] - 2.0f * gray[(qy - 1) * qW + qx] - gray[(qy - 1) * qW + (qx + 1)]
                    + gray[(qy + 1) * qW + (qx - 1)] + 2.0f * gray[(qy + 1) * qW + qx] + gray[(qy + 1) * qW + (qx + 1)];
                edgeMap[qy * qW + qx] = std::sqrt(gx * gx + gy * gy) / 1442.0f; // max ~4*255*sqrt(2)
            }
        }

        // Combine: saliency = 0.30*contrast + 0.25*edges + 0.20*colorUniq
        //                    + centerBiasWeight*center + 0.10*skinTone
        // Remaining weight filled by adjusting contrast weight
        float wContrast = 0.30f;
        float wEdge = 0.25f;
        float wColorUniq = 0.20f;
        float wCenter = m_centerBiasWeight;
        float wSkin = 0.10f;

        std::vector<float> saliency(qCount, 0.0f);
        for (uint32_t i = 0; i < qCount; ++i) {
            saliency[i] = wContrast * contrastMap[i]
                + wEdge * edgeMap[i]
                + wColorUniq * colorUniq[i]
                + wCenter * centerBias[i]
                + wSkin * skinMap[i];
        }
        return saliency;
    }

    /// Sum saliency in a rectangular region.
    static inline float SumRegion(const std::vector<float>& map, uint32_t mapW,
        uint32_t rx, uint32_t ry,
        uint32_t rw, uint32_t rh) {
        float s = 0.0f;
        for (uint32_t y = ry; y < ry + rh; ++y) {
            for (uint32_t x = rx; x < rx + rw; ++x) {
                s += map[y * mapW + x];
            }
        }
        // Normalize by area
        float area = static_cast<float>(rw * rh);
        return (area > 0.0f) ? s / area : 0.0f;
    }

    float       m_centerBiasWeight = 0.15f;
    mutable SRWLOCK m_statsLock{};
    CropStats   m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
