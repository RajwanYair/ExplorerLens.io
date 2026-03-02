//==============================================================================
// ExplorerLens Engine — Scene Classifier Engine (Sprint 574)
//
// Purpose:
//   Classifies thumbnail content into scene categories (landscape, portrait,
//   document, diagram, screenshot, etc.) using purely CPU-based image feature
//   analysis. No external ML frameworks are required.
//
// Classes:
//   SceneClassifierEngine — Stateless classifier combining six feature
//   extractors (color histogram, edge density, aspect ratio, color count,
//   text region detection, saturation analysis) with weighted scoring to
//   produce a scene category label and confidence estimate.
//
// Inputs:
//   - RGBA pixel buffer (uint8_t*), width, height
//
// Outputs:
//   - ClassifiedScene enum label
//   - SceneFeatures struct with raw numeric features
//   - Confidence in [0, 1]
//   - ClassifierStats with per-category counts and timing
//
// Thread Safety:
//   Classification is stateless and thread-safe. Aggregate statistics are
//   protected by an SRWLOCK.
//
// Build:
//   Header-only, C++20, MSVC /W4 clean, no external dependencies.
//==============================================================================
#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

/// Scene category labels produced by the classifier.
enum class ClassifiedScene : uint32_t {
    Unknown      = 0,
    Landscape    = 1,
    Portrait     = 2,
    Document     = 3,
    Diagram      = 4,
    Screenshot   = 5,
    Icon         = 6,
    Photo        = 7,
    Art          = 8,
    TextHeavy    = 9,
    CodeSnippet  = 10,
    Infographic  = 11,
    Map          = 12,
    Chart        = 13,
    COUNT        = 14
};

/// Raw numeric features extracted from the image.
struct SceneFeatures {
    std::array<float, 16> histR{};          // 16-bin red histogram (normalized)
    std::array<float, 16> histG{};          // 16-bin green histogram (normalized)
    std::array<float, 16> histB{};          // 16-bin blue histogram (normalized)
    float edgeDensity      = 0.0f;          // fraction of high-gradient pixels
    float aspectRatio      = 1.0f;          // width / height
    uint32_t uniqueColors  = 0;             // unique 6-bit-quantized colors
    float textRegionRatio  = 0.0f;          // fraction of rows classified as text
    float meanSaturation   = 0.0f;          // average HSV saturation [0, 1]
    float histogramSparsity = 0.0f;         // fraction of empty histogram bins
    float colorDiversity   = 0.0f;          // normalized unique-color ratio
};

/// Cumulative classifier statistics.
struct ClassifierStats {
    uint64_t totalClassifications = 0;
    std::array<uint64_t, static_cast<size_t>(ClassifiedScene::COUNT)> perCategoryCounts{};
    double totalClassifyTimeMs  = 0.0;
    double totalFeatureTimeMs   = 0.0;
    double AvgClassifyTimeMs() const {
        return totalClassifications ? totalClassifyTimeMs / static_cast<double>(totalClassifications) : 0.0;
    }
    double AvgFeatureTimeMs() const {
        return totalClassifications ? totalFeatureTimeMs / static_cast<double>(totalClassifications) : 0.0;
    }
};

/// CPU-based scene classification engine.
class SceneClassifierEngine {
public:
    SceneClassifierEngine() {
        InitializeSRWLock(&m_statsLock);
    }

    // ---- Feature extraction ------------------------------------------------

    /// Extract all features from an RGBA image buffer.
    inline SceneFeatures ExtractFeatures(const uint8_t* rgbaData,
                                         uint32_t width,
                                         uint32_t height) const {
        SceneFeatures f{};
        if (!rgbaData || width == 0 || height == 0) return f;

        const uint32_t pixelCount = width * height;
        f.aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // --- 1. Color histograms (16 bins per channel, normalized) -----------
        ComputeColorHistograms(rgbaData, pixelCount, f.histR, f.histG, f.histB);

        // Sparsity = fraction of bins that are zero across all three channels
        uint32_t emptyBins = 0;
        for (uint32_t i = 0; i < 16; ++i) {
            if (f.histR[i] < 1e-6f) ++emptyBins;
            if (f.histG[i] < 1e-6f) ++emptyBins;
            if (f.histB[i] < 1e-6f) ++emptyBins;
        }
        f.histogramSparsity = static_cast<float>(emptyBins) / 48.0f;

        // --- 2. Edge density (Sobel gradient magnitude) ----------------------
        f.edgeDensity = ComputeEdgeDensity(rgbaData, width, height);

        // --- 3. Unique color count (quantized to 6-bit: 2 bits per channel) --
        f.uniqueColors = CountUniqueColors6Bit(rgbaData, pixelCount);
        const uint32_t maxPossible = 64; // 2^6
        f.colorDiversity = static_cast<float>((std::min)(f.uniqueColors, maxPossible)) /
                           static_cast<float>(maxPossible);

        // --- 4. Text region detection ----------------------------------------
        f.textRegionRatio = DetectTextRows(rgbaData, width, height);

        // --- 5. Mean saturation (HSV) ----------------------------------------
        f.meanSaturation = ComputeMeanSaturation(rgbaData, pixelCount);

        return f;
    }

    /// Classify an RGBA image into a scene category.
    inline ClassifiedScene Classify(const uint8_t* rgbaData,
                                     uint32_t width,
                                     uint32_t height) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        auto featureStart = Clock::now();
        SceneFeatures f = ExtractFeatures(rgbaData, width, height);
        auto featureEnd   = Clock::now();
        double featureMs  = std::chrono::duration<double, std::milli>(featureEnd - featureStart).count();

        // ---- Weighted scoring per category ----------------------------------
        std::array<float, static_cast<size_t>(ClassifiedScene::COUNT)> scores{};

        // Landscape: wide, colorful, low edge, high saturation
        scores[static_cast<size_t>(ClassifiedScene::Landscape)] =
            (f.aspectRatio > 1.3f ? 0.25f : 0.0f) +
            (f.meanSaturation > 0.35f ? 0.25f : 0.0f) +
            (f.edgeDensity < 0.15f ? 0.20f : 0.0f) +
            (f.colorDiversity > 0.5f ? 0.15f : 0.0f) +
            (f.textRegionRatio < 0.05f ? 0.15f : 0.0f);

        // Portrait: tall, moderate saturation, low text
        scores[static_cast<size_t>(ClassifiedScene::Portrait)] =
            (f.aspectRatio < 0.85f ? 0.30f : 0.0f) +
            (f.meanSaturation > 0.2f ? 0.15f : 0.0f) +
            (f.edgeDensity < 0.25f ? 0.15f : 0.0f) +
            (f.textRegionRatio < 0.1f ? 0.20f : 0.0f) +
            (f.colorDiversity > 0.3f ? 0.20f : 0.0f);

        // Document: high text, low saturation, moderate edge
        scores[static_cast<size_t>(ClassifiedScene::Document)] =
            (f.textRegionRatio > 0.4f ? 0.35f : 0.0f) +
            (f.meanSaturation < 0.15f ? 0.25f : 0.0f) +
            (f.edgeDensity > 0.10f && f.edgeDensity < 0.35f ? 0.15f : 0.0f) +
            (f.colorDiversity < 0.15f ? 0.15f : 0.0f) +
            (f.histogramSparsity > 0.3f ? 0.10f : 0.0f);

        // Diagram: sparse colors, moderate edge, low saturation
        scores[static_cast<size_t>(ClassifiedScene::Diagram)] =
            (f.uniqueColors < 16 ? 0.30f : 0.0f) +
            (f.edgeDensity > 0.15f ? 0.20f : 0.0f) +
            (f.meanSaturation < 0.25f ? 0.15f : 0.0f) +
            (f.histogramSparsity > 0.4f ? 0.20f : 0.0f) +
            (f.textRegionRatio < 0.3f ? 0.15f : 0.0f);

        // Screenshot: moderate edge, many colors, some text
        scores[static_cast<size_t>(ClassifiedScene::Screenshot)] =
            (f.edgeDensity > 0.10f && f.edgeDensity < 0.30f ? 0.20f : 0.0f) +
            (f.textRegionRatio > 0.1f && f.textRegionRatio < 0.5f ? 0.25f : 0.0f) +
            (f.meanSaturation > 0.1f && f.meanSaturation < 0.4f ? 0.20f : 0.0f) +
            (f.colorDiversity > 0.2f ? 0.15f : 0.0f) +
            (std::abs(f.aspectRatio - 1.78f) < 0.3f ? 0.20f : 0.0f); // ~16:9

        // Icon: very few colors, small, high edge, roughly square
        scores[static_cast<size_t>(ClassifiedScene::Icon)] =
            (f.uniqueColors < 10 ? 0.30f : 0.0f) +
            (std::abs(f.aspectRatio - 1.0f) < 0.15f ? 0.25f : 0.0f) +
            (f.edgeDensity > 0.20f ? 0.20f : 0.0f) +
            (f.histogramSparsity > 0.5f ? 0.15f : 0.0f) +
            (f.textRegionRatio < 0.05f ? 0.10f : 0.0f);

        // Photo: high color diversity, moderate saturation, low text
        scores[static_cast<size_t>(ClassifiedScene::Photo)] =
            (f.colorDiversity > 0.6f ? 0.25f : 0.0f) +
            (f.meanSaturation > 0.2f && f.meanSaturation < 0.7f ? 0.20f : 0.0f) +
            (f.edgeDensity < 0.20f ? 0.15f : 0.0f) +
            (f.textRegionRatio < 0.05f ? 0.20f : 0.0f) +
            (f.histogramSparsity < 0.2f ? 0.20f : 0.0f);

        // Art: high saturation, high color diversity, low text
        scores[static_cast<size_t>(ClassifiedScene::Art)] =
            (f.meanSaturation > 0.5f ? 0.30f : 0.0f) +
            (f.colorDiversity > 0.5f ? 0.20f : 0.0f) +
            (f.textRegionRatio < 0.05f ? 0.15f : 0.0f) +
            (f.edgeDensity > 0.15f ? 0.15f : 0.0f) +
            (f.histogramSparsity < 0.15f ? 0.20f : 0.0f);

        // TextHeavy: very high text ratio, few colors, low saturation
        scores[static_cast<size_t>(ClassifiedScene::TextHeavy)] =
            (f.textRegionRatio > 0.6f ? 0.35f : 0.0f) +
            (f.meanSaturation < 0.1f ? 0.20f : 0.0f) +
            (f.uniqueColors < 8 ? 0.20f : 0.0f) +
            (f.edgeDensity > 0.08f ? 0.10f : 0.0f) +
            (f.histogramSparsity > 0.4f ? 0.15f : 0.0f);

        // CodeSnippet: very high text, very low saturation, monospaced patterns
        scores[static_cast<size_t>(ClassifiedScene::CodeSnippet)] =
            (f.textRegionRatio > 0.5f ? 0.30f : 0.0f) +
            (f.meanSaturation < 0.08f ? 0.25f : 0.0f) +
            (f.uniqueColors < 12 ? 0.15f : 0.0f) +
            (f.edgeDensity > 0.05f && f.edgeDensity < 0.25f ? 0.15f : 0.0f) +
            (f.histogramSparsity > 0.5f ? 0.15f : 0.0f);

        // Infographic: moderate text + moderate color diversity + moderate edge
        scores[static_cast<size_t>(ClassifiedScene::Infographic)] =
            (f.textRegionRatio > 0.15f && f.textRegionRatio < 0.5f ? 0.25f : 0.0f) +
            (f.colorDiversity > 0.3f ? 0.20f : 0.0f) +
            (f.edgeDensity > 0.10f ? 0.15f : 0.0f) +
            (f.meanSaturation > 0.2f ? 0.20f : 0.0f) +
            (f.uniqueColors > 10 && f.uniqueColors < 40 ? 0.20f : 0.0f);

        // Map: many colors, moderate saturation, very low text
        scores[static_cast<size_t>(ClassifiedScene::Map)] =
            (f.colorDiversity > 0.5f ? 0.25f : 0.0f) +
            (f.meanSaturation > 0.15f && f.meanSaturation < 0.5f ? 0.20f : 0.0f) +
            (f.textRegionRatio < 0.1f ? 0.15f : 0.0f) +
            (f.edgeDensity > 0.10f && f.edgeDensity < 0.30f ? 0.20f : 0.0f) +
            (std::abs(f.aspectRatio - 1.0f) < 0.4f ? 0.20f : 0.0f);

        // Chart: moderate colors, some edge, low text
        scores[static_cast<size_t>(ClassifiedScene::Chart)] =
            (f.uniqueColors > 5 && f.uniqueColors < 25 ? 0.25f : 0.0f) +
            (f.edgeDensity > 0.08f && f.edgeDensity < 0.25f ? 0.20f : 0.0f) +
            (f.textRegionRatio > 0.05f && f.textRegionRatio < 0.3f ? 0.20f : 0.0f) +
            (f.meanSaturation > 0.15f ? 0.15f : 0.0f) +
            (f.histogramSparsity > 0.2f ? 0.20f : 0.0f);

        // Find best category
        float bestScore = 0.0f;
        ClassifiedScene best = ClassifiedScene::Unknown;
        for (uint32_t i = 1; i < static_cast<uint32_t>(ClassifiedScene::COUNT); ++i) {
            if (scores[i] > bestScore) {
                bestScore = scores[i];
                best = static_cast<ClassifiedScene>(i);
            }
        }

        // Confidence: best score normalized (max possible ~1.0)
        m_lastConfidence = (std::min)(bestScore, 1.0f);

        auto t1 = Clock::now();
        double totalMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

        // Update stats
        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.totalClassifications++;
        m_stats.perCategoryCounts[static_cast<size_t>(best)]++;
        m_stats.totalClassifyTimeMs += totalMs;
        m_stats.totalFeatureTimeMs  += featureMs;
        ReleaseSRWLockExclusive(&m_statsLock);

        return best;
    }

    /// Confidence of the last classification (0.0 – 1.0).
    inline float GetConfidence() const { return m_lastConfidence; }

    /// Retrieve cumulative classifier statistics (thread-safe).
    inline ClassifierStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        ClassifierStats copy = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        return copy;
    }

    /// Human-readable name for a ClassifiedScene value.
    static inline const wchar_t* CategoryName(ClassifiedScene cat) {
        switch (cat) {
        case ClassifiedScene::Unknown:     return L"Unknown";
        case ClassifiedScene::Landscape:   return L"Landscape";
        case ClassifiedScene::Portrait:    return L"Portrait";
        case ClassifiedScene::Document:    return L"Document";
        case ClassifiedScene::Diagram:     return L"Diagram";
        case ClassifiedScene::Screenshot:  return L"Screenshot";
        case ClassifiedScene::Icon:        return L"Icon";
        case ClassifiedScene::Photo:       return L"Photo";
        case ClassifiedScene::Art:         return L"Art";
        case ClassifiedScene::TextHeavy:   return L"TextHeavy";
        case ClassifiedScene::CodeSnippet: return L"CodeSnippet";
        case ClassifiedScene::Infographic: return L"Infographic";
        case ClassifiedScene::Map:         return L"Map";
        case ClassifiedScene::Chart:       return L"Chart";
        default:                           return L"Unknown";
        }
    }

private:
    // ---- Implementation helpers (all static / const) -----------------------

    /// Compute normalized 16-bin histograms for R, G, B channels.
    static inline void ComputeColorHistograms(const uint8_t* rgba,
                                              uint32_t pixelCount,
                                              std::array<float, 16>& histR,
                                              std::array<float, 16>& histG,
                                              std::array<float, 16>& histB) {
        histR.fill(0.0f);
        histG.fill(0.0f);
        histB.fill(0.0f);
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            histR[rgba[off + 0] >> 4]++;
            histG[rgba[off + 1] >> 4]++;
            histB[rgba[off + 2] >> 4]++;
        }
        float inv = 1.0f / static_cast<float>((std::max)(pixelCount, 1u));
        for (uint32_t b = 0; b < 16; ++b) {
            histR[b] *= inv;
            histG[b] *= inv;
            histB[b] *= inv;
        }
    }

    /// Compute edge density via Sobel on grayscale approximation.
    static inline float ComputeEdgeDensity(const uint8_t* rgba,
                                           uint32_t width,
                                           uint32_t height) {
        if (width < 3 || height < 3) return 0.0f;
        uint32_t edgePixels = 0;
        const float threshold = 60.0f;
        for (uint32_t y = 1; y < height - 1; ++y) {
            for (uint32_t x = 1; x < width - 1; ++x) {
                auto gray = [&](uint32_t px, uint32_t py) -> float {
                    uint32_t off = (py * width + px) * 4;
                    return 0.299f * rgba[off] + 0.587f * rgba[off + 1] + 0.114f * rgba[off + 2];
                };
                float gx = -gray(x - 1, y - 1) - 2.0f * gray(x - 1, y) - gray(x - 1, y + 1)
                           + gray(x + 1, y - 1) + 2.0f * gray(x + 1, y) + gray(x + 1, y + 1);
                float gy = -gray(x - 1, y - 1) - 2.0f * gray(x, y - 1) - gray(x + 1, y - 1)
                           + gray(x - 1, y + 1) + 2.0f * gray(x, y + 1) + gray(x + 1, y + 1);
                float mag = std::sqrt(gx * gx + gy * gy);
                if (mag > threshold) ++edgePixels;
            }
        }
        uint32_t total = (width - 2) * (height - 2);
        return static_cast<float>(edgePixels) / static_cast<float>((std::max)(total, 1u));
    }

    /// Count unique colors after quantization to 6-bit (2 bits per channel).
    static inline uint32_t CountUniqueColors6Bit(const uint8_t* rgba,
                                                  uint32_t pixelCount) {
        // 64 possible 6-bit colors; use a 64-bit bitset
        uint64_t seen = 0;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            uint32_t q = ((rgba[off + 0] >> 6) << 4) |
                         ((rgba[off + 1] >> 6) << 2) |
                          (rgba[off + 2] >> 6);
            seen |= (1ULL << q);
        }
        // popcount
        uint32_t count = 0;
        uint64_t v = seen;
        while (v) { count += static_cast<uint32_t>(v & 1ULL); v >>= 1; }
        return count;
    }

    /// Detect rows likely containing text (high-contrast horizontal runs).
    static inline float DetectTextRows(const uint8_t* rgba,
                                       uint32_t width,
                                       uint32_t height) {
        if (width < 8 || height < 4) return 0.0f;
        uint32_t textRows = 0;
        for (uint32_t y = 0; y < height; ++y) {
            uint32_t transitions = 0;
            uint8_t prevGray = 0;
            for (uint32_t x = 0; x < width; ++x) {
                uint32_t off = (y * width + x) * 4;
                uint8_t g = static_cast<uint8_t>(
                    0.299f * rgba[off] + 0.587f * rgba[off + 1] + 0.114f * rgba[off + 2]);
                if (x > 0) {
                    int diff = static_cast<int>(g) - static_cast<int>(prevGray);
                    if (diff > 40 || diff < -40) ++transitions;
                }
                prevGray = g;
            }
            // A text row typically has many high-contrast transitions
            float transRate = static_cast<float>(transitions) / static_cast<float>(width);
            if (transRate > 0.05f && transRate < 0.5f) ++textRows;
        }
        return static_cast<float>(textRows) / static_cast<float>(height);
    }

    /// Compute mean saturation in HSV color space.
    static inline float ComputeMeanSaturation(const uint8_t* rgba,
                                              uint32_t pixelCount) {
        if (pixelCount == 0) return 0.0f;
        double satSum = 0.0;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            float r = rgba[off + 0] / 255.0f;
            float g = rgba[off + 1] / 255.0f;
            float b = rgba[off + 2] / 255.0f;
            float cmax = (std::max)({r, g, b});
            float cmin = (std::min)({r, g, b});
            float sat  = (cmax > 1e-6f) ? ((cmax - cmin) / cmax) : 0.0f;
            satSum += sat;
        }
        return static_cast<float>(satSum / static_cast<double>(pixelCount));
    }

    mutable SRWLOCK      m_statsLock{};
    ClassifierStats      m_stats{};
    mutable float        m_lastConfidence = 0.0f;
};

} // namespace Engine
} // namespace ExplorerLens
