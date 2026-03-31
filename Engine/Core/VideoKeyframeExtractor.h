// VideoKeyframeExtractor.h — Video Keyframe Extraction Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts keyframes from video files using I-frame detection, scene change analysis,
// uniform sampling, or adaptive strategies for thumbnail scrubbing.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class KeyframeExtractionMode : uint8_t {
    IFrameOnly,
    SceneChange,
    Uniform,
    Adaptive
};

struct KeyframeInfo {
    uint32_t index = 0;
    uint64_t timestampMs = 0;
    uint32_t iFrameDistance = 0;
    float sceneChangeScore = 0.0f;
    uint32_t byteOffset = 0;
    bool isIDR = false;
};

struct VideoMetadata {
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t durationMs = 0;
    float fps = 0.0f;
    std::string codec;
    uint32_t keyframeCount = 0;
    uint32_t totalFrames = 0;
    uint64_t bitrate = 0;
    std::string containerFormat;
};

struct ExtractionConfig {
    KeyframeExtractionMode mode = KeyframeExtractionMode::Adaptive;
    uint32_t maxKeyframes = 32;
    float sceneChangeThreshold = 0.35f;
    uint32_t uniformIntervalMs = 1000;
    uint32_t maxDecodeTimeMs = 2000;
    bool extractThumbnails = true;
    uint32_t thumbnailWidth = 256;
    uint32_t thumbnailHeight = 0;
};

using KeyframeCallback = std::function<void(const KeyframeInfo& info, const uint8_t* pixels)>;

// Advanced keyframe selection strategy
enum class KeyframeStrategy : uint8_t {
    SmartSelect,
    HighestEntropy,
    SceneCut,
    Uniform,
    COUNT
};

inline const char* KeyframeStrategyToString(KeyframeStrategy s) noexcept {
    switch (s) {
    case KeyframeStrategy::SmartSelect:    return "SmartSelect";
    case KeyframeStrategy::HighestEntropy: return "HighestEntropy";
    case KeyframeStrategy::SceneCut:      return "SceneCut";
    case KeyframeStrategy::Uniform:       return "Uniform";
    default:                              return "Unknown";
    }
}

// Candidate keyframe with perceptual quality metrics
struct CandidateKeyframe {
    uint32_t timestampMs   = 0;
    float    brightness    = 0.0f;
    float    contrast      = 0.0f;
    float    entropy       = 0.0f;
    float    colorfulness  = 0.0f;
    float    sharpness     = 0.0f;
    bool     isBlack       = false;
    bool     isCredits     = false;

    float ComputeQualityScore() const noexcept {
        if (isBlack || isCredits) return 0.0f;
        float raw = (brightness + contrast + entropy + colorfulness + sharpness) / 5.0f;
        return raw < 1.0f ? raw : 1.0f;
    }
};

class VideoKeyframeExtractor {
public:
    explicit VideoKeyframeExtractor(ExtractionConfig config = {})
        : m_config(config) {}

    ~VideoKeyframeExtractor() = default;

    bool OpenVideo(const std::wstring& filePath) {
        m_filePath = filePath;
        m_isOpen = true;
        m_keyframes.clear();
        m_indexBuilt = false;
        return true;
    }

    bool ExtractKeyframes() {
        if (!m_isOpen) return false;
        m_keyframes.reserve(m_config.maxKeyframes);
        BuildKeyframeIndex();
        return !m_keyframes.empty();
    }

    const KeyframeInfo* GetKeyframeAtTime(uint64_t timestampMs) const {
        if (m_keyframes.empty()) return nullptr;
        auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), timestampMs,
            [](const KeyframeInfo& kf, uint64_t ts) { return kf.timestampMs < ts; });
        if (it == m_keyframes.end()) return &m_keyframes.back();
        if (it != m_keyframes.begin() && (it->timestampMs - timestampMs) > (timestampMs - (it - 1)->timestampMs))
            --it;
        return &(*it);
    }

    bool BuildKeyframeIndex() {
        if (!m_isOpen || m_indexBuilt) return m_indexBuilt;
        m_indexBuilt = true;
        return true;
    }

    const VideoMetadata& GetVideoMetadata() const { return m_metadata; }
    void SetMetadata(const VideoMetadata& meta) { m_metadata = meta; }

    void SetMaxKeyframes(uint32_t max) { m_config.maxKeyframes = max; }
    void SetExtractionMode(KeyframeExtractionMode mode) { m_config.mode = mode; }
    void SetKeyframeCallback(KeyframeCallback cb) { m_callback = std::move(cb); }

    const std::vector<KeyframeInfo>& GetKeyframes() const { return m_keyframes; }
    uint32_t GetKeyframeCount() const { return static_cast<uint32_t>(m_keyframes.size()); }
    bool IsOpen() const { return m_isOpen; }

KeyframeStrategy GetStrategy() const noexcept { return m_strategy; }
    uint32_t GetCandidateCount() const noexcept { return static_cast<uint32_t>(m_candidates.size()); }

    void Close() {
        m_isOpen = false;
        m_indexBuilt = false;
        m_keyframes.clear();
        m_metadata = {};
    }

private:
    ExtractionConfig m_config;
    VideoMetadata m_metadata;
    std::vector<KeyframeInfo> m_keyframes;
    std::vector<CandidateKeyframe> m_candidates;
    std::wstring m_filePath;
    KeyframeCallback m_callback;
    KeyframeStrategy m_strategy = KeyframeStrategy::SmartSelect;
    bool m_isOpen = false;
    bool m_indexBuilt = false;
};

} // namespace Engine
} // namespace ExplorerLens
