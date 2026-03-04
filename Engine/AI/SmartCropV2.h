// SmartCropV2.h — Canonical Smart Crop (V1 + V2 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Consolidated smart crop: V1 saliency engine + V2 strategy-based cropper.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

// ── SmartCropEngine V1 types (consolidated from SmartCropEngine.h) ─────────

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

        uint32_t qW = (std::max)(srcW / 4, 1u);
        uint32_t qH = (std::max)(srcH / 4, 1u);
        std::vector<float> saliency = BuildSaliencyMap(rgbaData, srcW, srcH, qW, qH);

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

        double meanR = 0.0, meanG = 0.0, meanB = 0.0;
        uint32_t srcPixels = srcW * srcH;
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

        for (uint32_t qy = 0; qy < qH; ++qy) {
            for (uint32_t qx = 0; qx < qW; ++qx) {
                uint32_t sx = (std::min)(qx * srcW / qW, srcW - 1);
                uint32_t sy = (std::min)(qy * srcH / qH, srcH - 1);
                uint32_t off = (sy * srcW + sx) * 4;
                float r = rgba[off + 0];
                float g = rgba[off + 1];
                float b = rgba[off + 2];
                uint32_t idx = qy * qW + qx;

                gray[idx] = 0.299f * r + 0.587f * g + 0.114f * b;

                float dr = r - static_cast<float>(meanR);
                float dg = g - static_cast<float>(meanG);
                float db = b - static_cast<float>(meanB);
                colorUniq[idx] = std::sqrt(dr * dr + dg * dg + db * db) / 441.7f;

                float dx = static_cast<float>(qx) - cx;
                float dy = static_cast<float>(qy) - cy;
                centerBias[idx] = std::exp(-(dx * dx + dy * dy) * invDiag2);

                float Y = 0.299f * r + 0.587f * g + 0.114f * b;
                float Cb = 128.0f - 0.168736f * r - 0.331264f * g + 0.5f * b;
                float Cr = 128.0f + 0.5f * r - 0.418688f * g - 0.081312f * b;
                (void)Y;
                bool isSkin = (Cb >= 77.0f && Cb <= 127.0f && Cr >= 133.0f && Cr <= 173.0f);
                skinMap[idx] = isSkin ? 1.0f : 0.0f;
            }
        }

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

        for (uint32_t qy = 1; qy + 1 < qH; ++qy) {
            for (uint32_t qx = 1; qx + 1 < qW; ++qx) {
                float gx = -gray[(qy - 1) * qW + (qx - 1)] - 2.0f * gray[qy * qW + (qx - 1)] - gray[(qy + 1) * qW + (qx - 1)]
                    + gray[(qy - 1) * qW + (qx + 1)] + 2.0f * gray[qy * qW + (qx + 1)] + gray[(qy + 1) * qW + (qx + 1)];
                float gy = -gray[(qy - 1) * qW + (qx - 1)] - 2.0f * gray[(qy - 1) * qW + qx] - gray[(qy - 1) * qW + (qx + 1)]
                    + gray[(qy + 1) * qW + (qx - 1)] + 2.0f * gray[(qy + 1) * qW + qx] + gray[(qy + 1) * qW + (qx + 1)];
                edgeMap[qy * qW + qx] = std::sqrt(gx * gx + gy * gy) / 1442.0f;
            }
        }

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

    static inline float SumRegion(const std::vector<float>& map, uint32_t mapW,
        uint32_t rx, uint32_t ry,
        uint32_t rw, uint32_t rh) {
        float s = 0.0f;
        for (uint32_t y = ry; y < ry + rh; ++y) {
            for (uint32_t x = rx; x < rx + rw; ++x) {
                s += map[y * mapW + x];
            }
        }
        float area = static_cast<float>(rw * rh);
        return (area > 0.0f) ? s / area : 0.0f;
    }

    float       m_centerBiasWeight = 0.15f;
    mutable SRWLOCK m_statsLock{};
    CropStats   m_stats{};
};

// ── SmartCropV2 (strategy-based cropper) ───────────────────────────────────

enum class CropStrategy : uint8_t {
    CenterCrop = 0, SaliencyMap, FaceCentered, RuleOfThirds, GoldenRatio, SubjectAware, COUNT
};
enum class CropAspectRatio : uint8_t { Square = 0, Landscape4_3, Portrait3_4, Wide16_9, Auto, COUNT };
enum class CropPaddingMode : uint8_t { None = 0, Extend, Blur, SolidColor, COUNT };

struct SmartCropRequest {
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    CropStrategy strategy = CropStrategy::SaliencyMap;
    CropAspectRatio aspectRatio = CropAspectRatio::Square;
    CropPaddingMode padding = CropPaddingMode::Blur;
};

struct SmartCropResult {
    uint32_t cropX = 0, cropY = 0, cropW = 0, cropH = 0;
    CropStrategy usedStrategy = CropStrategy::CenterCrop;
    float saliencyScore = 0.0f;
    bool faceDetected = false;
};

class SmartCropV2 {
public:
    static const wchar_t* StrategyName(CropStrategy s) {
        switch (s) {
        case CropStrategy::CenterCrop: return L"Center Crop";
        case CropStrategy::SaliencyMap: return L"Saliency Map";
        case CropStrategy::FaceCentered: return L"Face Centered";
        case CropStrategy::RuleOfThirds: return L"Rule of Thirds";
        case CropStrategy::GoldenRatio: return L"Golden Ratio";
        case CropStrategy::SubjectAware: return L"Subject Aware";
        default: return L"Unknown";
        }
    }
    static const wchar_t* AspectRatioName(CropAspectRatio a) {
        switch (a) {
        case CropAspectRatio::Square: return L"1:1 Square";
        case CropAspectRatio::Landscape4_3: return L"4:3 Landscape";
        case CropAspectRatio::Portrait3_4: return L"3:4 Portrait";
        case CropAspectRatio::Wide16_9: return L"16:9 Wide";
        case CropAspectRatio::Auto: return L"Auto";
        default: return L"Unknown";
        }
    }
    static const wchar_t* PaddingModeName(CropPaddingMode p) {
        switch (p) {
        case CropPaddingMode::None: return L"None";
        case CropPaddingMode::Extend: return L"Extend";
        case CropPaddingMode::Blur: return L"Blur";
        case CropPaddingMode::SolidColor: return L"Solid Color";
        default: return L"Unknown";
        }
    }
    static constexpr size_t StrategyCount() { return static_cast<size_t>(CropStrategy::COUNT); }
    static constexpr size_t AspectRatioCount() { return static_cast<size_t>(CropAspectRatio::COUNT); }
    static constexpr size_t PaddingCount() { return static_cast<size_t>(CropPaddingMode::COUNT); }

    //==========================================================================
    // Smart Crop — Gradient-weighted center of interest
    //==========================================================================

    /// Compute center-of-interest using gradient energy (Sobel magnitude).
    /// Returns (cx, cy) as the weighted center of visual interest.
    /// Input: 8-bit grayscale, width, height, stride.
    static void ComputeCenterOfInterest(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride, float& outCX, float& outCY) {
        outCX = static_cast<float>(width) / 2.0f;
        outCY = static_cast<float>(height) / 2.0f;
        if (!gray || width < 3 || height < 3) return;
        double weightedX = 0, weightedY = 0, totalWeight = 0;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                // Sobel gradient magnitude (simplified)
                int gx = -gray[(y - 1) * stride + (x - 1)] + gray[(y - 1) * stride + (x + 1)]
                    - 2 * gray[y * stride + (x - 1)] + 2 * gray[y * stride + (x + 1)]
                    - gray[(y + 1) * stride + (x - 1)] + gray[(y + 1) * stride + (x + 1)];
                int gy = -gray[(y - 1) * stride + (x - 1)] - 2 * gray[(y - 1) * stride + x]
                    - gray[(y - 1) * stride + (x + 1)]
                    + gray[(y + 1) * stride + (x - 1)] + 2 * gray[(y + 1) * stride + x]
                    + gray[(y + 1) * stride + (x + 1)];
                double mag = gx * gx + gy * gy; // skip sqrt for perf
                weightedX += x * mag;
                weightedY += y * mag;
                totalWeight += mag;
            }
        }
        if (totalWeight > 0) {
            outCX = static_cast<float>(weightedX / totalWeight);
            outCY = static_cast<float>(weightedY / totalWeight);
        }
    }

    /// Compute a crop region centered on the visual interest point.
    /// cropWidth/cropHeight: desired output dimensions.
    /// Returns: x, y, w, h of the crop rectangle (clamped to image bounds).
    static SmartCropResult ComputeCropRegion(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride, uint32_t cropWidth, uint32_t cropHeight) {
        SmartCropResult result;
        result.usedStrategy = CropStrategy::SubjectAware;
        if (!gray || width == 0 || height == 0) return result;
        // Clamp crop to image size
        if (cropWidth > width) cropWidth = width;
        if (cropHeight > height) cropHeight = height;
        // Find center of interest
        float cx, cy;
        ComputeCenterOfInterest(gray, width, height, stride, cx, cy);
        // Center crop on interest point, clamped to image bounds
        int x0 = static_cast<int>(cx) - static_cast<int>(cropWidth / 2);
        int y0 = static_cast<int>(cy) - static_cast<int>(cropHeight / 2);
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x0 + cropWidth > width) x0 = width - cropWidth;
        if (y0 + cropHeight > height) y0 = height - cropHeight;
        result.cropX = static_cast<uint32_t>(x0);
        result.cropY = static_cast<uint32_t>(y0);
        result.cropW = cropWidth;
        result.cropH = cropHeight;
        result.saliencyScore = 0.8f; // Gradient-based heuristic
        return result;
    }
};

}
} // namespace ExplorerLens::Engine
