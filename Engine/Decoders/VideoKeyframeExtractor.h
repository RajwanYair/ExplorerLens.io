// VideoKeyframeExtractor.h — Smart Video Keyframe Selection for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Intelligently selects the most representative keyframe from video files
// for thumbnail generation. Uses scene analysis heuristics (brightness,
// entropy, face detection hints, motion scoring) to avoid black frames,
// credits, and scene transitions.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Keyframe selection strategy
// ============================================================================

enum class KeyframeStrategy : uint8_t {
    FirstKeyframe = 0,   // First I-frame (fastest, often black/logo)
    ThirdKeyframe = 1,   // Third I-frame (usually after credits)
    TenPercent = 2,      // Frame at 10% of duration
    Midpoint = 3,        // Frame at 50% of duration
    SmartSelect = 4,     // ML-heuristic selection (default)
    HighestEntropy = 5,  // Most visually complex frame
    BrightestFrame = 6   // Brightest frame in candidate set
};

inline const char* KeyframeStrategyToString(KeyframeStrategy strategy)
{
    static const char* names[] = {"FirstKeyframe", "ThirdKeyframe",  "TenPercent",    "Midpoint",
                                  "SmartSelect",   "HighestEntropy", "BrightestFrame"};
    return names[static_cast<uint8_t>(strategy)];
}

// ============================================================================
// Candidate keyframe with quality metrics
// ============================================================================

struct CandidateKeyframe
{
    uint64_t frameIndex = 0;   // Frame number in stream
    double timestampMs = 0.0;  // Presentation timestamp
    uint32_t width = 0;
    uint32_t height = 0;
    bool isIFrame = false;  // Is this an intra-coded frame?

    // Quality metrics (normalized 0.0 - 1.0)
    float brightness = 0.5f;    // Average luminance
    float contrast = 0.5f;      // Standard deviation of luminance
    float entropy = 0.5f;       // Shannon entropy (visual complexity)
    float colorfulness = 0.5f;  // Chroma variance
    float sharpness = 0.5f;     // High-frequency energy
    float faceScore = 0.0f;     // Face detection confidence
    float motionScore = 0.0f;   // Inter-frame motion (0 = still)
    bool isBlack = false;       // All-black frame
    bool isCredits = false;     // Likely credits/title card

    /// Composite quality score for smart selection
    float ComputeQualityScore() const
    {
        if (isBlack || isCredits)
            return 0.0f;

        // Weighted combination
        float score = 0.0f;
        score += brightness * 0.15f;    // Prefer reasonably bright
        score += contrast * 0.20f;      // Prefer good contrast
        score += entropy * 0.25f;       // Prefer visually interesting
        score += colorfulness * 0.15f;  // Prefer colorful
        score += sharpness * 0.15f;     // Prefer sharp
        score += faceScore * 0.10f;     // Bonus for faces

        // Penalize very dark or very bright frames
        if (brightness < 0.1f)
            score *= 0.3f;
        if (brightness > 0.95f)
            score *= 0.5f;

        // Penalize very low motion (likely still/title)
        if (motionScore < 0.05f && timestampMs > 5000.0)
            score *= 0.7f;

        return score;
    }

    bool operator<(const CandidateKeyframe& other) const
    {
        return ComputeQualityScore() > other.ComputeQualityScore();
    }
};

// ============================================================================
// Video analysis summary
// ============================================================================

struct VideoAnalysisSummary
{
    uint64_t totalFrames = 0;
    double durationMs = 0.0;
    double fps = 0.0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t keyframeCount = 0;
    uint32_t candidatesAnalyzed = 0;
    uint64_t selectedFrameIndex = 0;
    double selectedTimestampMs = 0.0;
    float selectedQualityScore = 0.0f;
    double analysisTimeMs = 0.0;
    KeyframeStrategy usedStrategy;
    std::wstring codec;
};

// ============================================================================
// VideoKeyframeExtractor
// ============================================================================

class VideoKeyframeExtractor
{
  public:
    /// Number of candidate frames to analyze for SmartSelect
    static constexpr uint32_t MAX_CANDIDATES = 20;

    /// Skip first N seconds to avoid logos/black frames
    static constexpr double SKIP_INTRO_MS = 3000.0;

    /// Skip last 10% to avoid credits
    static constexpr float SKIP_OUTRO_PERCENT = 0.10f;

    VideoKeyframeExtractor() = default;
    ~VideoKeyframeExtractor() = default;

    // Non-copyable
    VideoKeyframeExtractor(const VideoKeyframeExtractor&) = delete;
    VideoKeyframeExtractor& operator=(const VideoKeyframeExtractor&) = delete;

    // ========================================================================
    // Strategy selection
    // ========================================================================

    void SetStrategy(KeyframeStrategy strategy)
    {
        m_strategy = strategy;
    }

    KeyframeStrategy GetStrategy() const
    {
        return m_strategy;
    }

    // ========================================================================
    // Candidate management
    // ========================================================================

    /// Set candidate keyframes for analysis
    void SetCandidates(const std::vector<CandidateKeyframe>& candidates)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_candidates = candidates;
    }

    void AddCandidate(const CandidateKeyframe& frame)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_candidates.size() < MAX_CANDIDATES) {
            m_candidates.push_back(frame);
        }
    }

    uint32_t GetCandidateCount() const
    {
        return static_cast<uint32_t>(m_candidates.size());
    }

    void ClearCandidates()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_candidates.clear();
    }

    // ========================================================================
    // Keyframe selection
    // ========================================================================

    /// Select the best keyframe from candidates
    CandidateKeyframe SelectBestFrame(double videoDurationMs = 0.0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_candidates.empty()) {
            return {};
        }

        switch (m_strategy) {
            case KeyframeStrategy::FirstKeyframe:
                return SelectFirst();
            case KeyframeStrategy::ThirdKeyframe:
                return SelectNth(2);  // 0-indexed
            case KeyframeStrategy::TenPercent:
                return SelectAtPercent(0.10, videoDurationMs);
            case KeyframeStrategy::Midpoint:
                return SelectAtPercent(0.50, videoDurationMs);
            case KeyframeStrategy::SmartSelect:
                return SelectSmart(videoDurationMs);
            case KeyframeStrategy::HighestEntropy:
                return SelectByMetric([](const CandidateKeyframe& f) { return f.entropy; });
            case KeyframeStrategy::BrightestFrame:
                return SelectByMetric([](const CandidateKeyframe& f) { return f.brightness; });
            default:
                return SelectSmart(videoDurationMs);
        }
    }

    /// Quick analysis: generates heuristic metrics for a frame
    static void AnalyzeFrameQuick(CandidateKeyframe& frame, const uint8_t* rgbData, uint32_t pixelCount)
    {
        if (!rgbData || pixelCount == 0)
            return;

        // Compute brightness (average of green channel for speed)
        uint64_t totalLum = 0;
        uint64_t totalR = 0, totalG = 0, totalB = 0;
        for (uint32_t i = 0; i < pixelCount; i++) {
            uint8_t r = rgbData[i * 3];
            uint8_t g = rgbData[i * 3 + 1];
            uint8_t b = rgbData[i * 3 + 2];
            totalLum += (r * 299 + g * 587 + b * 114) / 1000;
            totalR += r;
            totalG += g;
            totalB += b;
        }
        frame.brightness = static_cast<float>(totalLum) / (pixelCount * 255.0f);

        // Black frame detection
        frame.isBlack = (frame.brightness < 0.02f);

        // Colorfulness (simplified: variance of channel means)
        float avgR = static_cast<float>(totalR) / pixelCount;
        float avgG = static_cast<float>(totalG) / pixelCount;
        float avgB = static_cast<float>(totalB) / pixelCount;
        float channelVar =
            ((avgR - avgG) * (avgR - avgG) + (avgG - avgB) * (avgG - avgB) + (avgR - avgB) * (avgR - avgB)) / 3.0f;
        frame.colorfulness = (std::min)(channelVar / 10000.0f, 1.0f);

        // Simple sharpness (edge count heuristic)
        frame.sharpness = 0.5f;  // Would use Laplacian variance in production

        // Simple entropy estimate based on brightness distribution
        frame.entropy = (std::min)(frame.brightness * (1.0f - frame.brightness) * 4.0f, 1.0f);
    }

    // ========================================================================
    // Summary
    // ========================================================================

    VideoAnalysisSummary GetLastAnalysis() const
    {
        return m_lastAnalysis;
    }

  private:
    CandidateKeyframe SelectFirst() const
    {
        for (const auto& c : m_candidates) {
            if (c.isIFrame)
                return c;
        }
        return m_candidates.front();
    }

    CandidateKeyframe SelectNth(uint32_t n) const
    {
        uint32_t iFrameIdx = 0;
        for (const auto& c : m_candidates) {
            if (c.isIFrame) {
                if (iFrameIdx == n)
                    return c;
                iFrameIdx++;
            }
        }
        return m_candidates.back();
    }

    CandidateKeyframe SelectAtPercent(double pct, double durationMs) const
    {
        double targetMs = durationMs * pct;
        CandidateKeyframe best = m_candidates.front();
        double bestDiff = std::abs(best.timestampMs - targetMs);

        for (const auto& c : m_candidates) {
            double diff = std::abs(c.timestampMs - targetMs);
            if (diff < bestDiff) {
                best = c;
                bestDiff = diff;
            }
        }
        return best;
    }

    CandidateKeyframe SelectSmart(double durationMs) const
    {
        // Filter: skip intro and outro
        std::vector<CandidateKeyframe> filtered;
        double outroThreshold = durationMs * (1.0 - SKIP_OUTRO_PERCENT);

        for (const auto& c : m_candidates) {
            if (c.timestampMs >= SKIP_INTRO_MS && (durationMs == 0.0 || c.timestampMs <= outroThreshold) && !c.isBlack
                && !c.isCredits) {
                filtered.push_back(c);
            }
        }

        if (filtered.empty())
            filtered = m_candidates;

        // Sort by quality score
        std::sort(filtered.begin(), filtered.end());
        return filtered.front();
    }

    template <typename MetricFn>
    CandidateKeyframe SelectByMetric(MetricFn fn) const
    {
        auto best =
            std::max_element(m_candidates.begin(), m_candidates.end(),
                             [&fn](const CandidateKeyframe& a, const CandidateKeyframe& b) { return fn(a) < fn(b); });
        return (best != m_candidates.end()) ? *best : CandidateKeyframe{};
    }

    // State
    std::vector<CandidateKeyframe> m_candidates;
    KeyframeStrategy m_strategy = KeyframeStrategy::SmartSelect;
    VideoAnalysisSummary m_lastAnalysis{};
    mutable std::mutex m_mutex;
};

}  // namespace Engine
}  // namespace ExplorerLens
