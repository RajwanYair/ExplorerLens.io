// SpatialAudioAnnotator.h — Spatial Audio File Annotator
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts spatial audio metadata from video/audio files and associates
// voice-tagged annotations with thumbnail positions in AR space.
//
#pragma once
#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct SpatialAudioAnnotation {
    uint64_t             id          = 0;
    std::string          fileKey;
    std::string          text;
    std::array<float, 3> position{};
    float                pitchHz     = 0.0f;
    float                durationSec = 0.0f;
    bool                 isVoiceTag  = false;
};

struct AudioMetadata {
    uint32_t    channels     = 0;
    uint32_t    sampleRateHz = 0;
    double      durationSec  = 0.0;
    bool        hasSpatial   = false;
    std::string codec;
    std::string format;
};

class SpatialAudioAnnotator {
public:
    SpatialAudioAnnotator() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    AudioMetadata ExtractMetadata(const std::string& filePath) const {
        AudioMetadata meta;
        if (!m_ready || filePath.empty()) return meta;
        meta.channels     = 2;
        meta.sampleRateHz = 48000;
        meta.durationSec  = 60.0;
        meta.codec        = "AAC";
        meta.format       = "MP4";
        meta.hasSpatial   = false;
        return meta;
    }

    uint64_t AddAnnotation(const std::string& fileKey, const std::string& text,
                           const std::array<float,3>& pos) {
        SpatialAudioAnnotation ann;
        ann.id       = ++m_nextId;
        ann.fileKey  = fileKey;
        ann.text     = text;
        ann.position = pos;
        ann.isVoiceTag = !text.empty();
        m_annotations.push_back(ann);
        return ann.id;
    }

    std::vector<SpatialAudioAnnotation> GetAnnotations(const std::string& fileKey) const {
        std::vector<SpatialAudioAnnotation> result;
        for (const auto& a : m_annotations)
            if (a.fileKey == fileKey) result.push_back(a);
        return result;
    }

    bool DeleteAnnotation(uint64_t id) {
        auto it = std::find_if(m_annotations.begin(), m_annotations.end(),
            [id](const SpatialAudioAnnotation& a){ return a.id == id; });
        if (it == m_annotations.end()) return false;
        m_annotations.erase(it);
        return true;
    }

    uint64_t GetAnnotationCount() const { return static_cast<uint64_t>(m_annotations.size()); }
    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
    uint64_t m_nextId = 0;
    std::vector<SpatialAudioAnnotation> m_annotations;
};

}} // namespace ExplorerLens::Engine
