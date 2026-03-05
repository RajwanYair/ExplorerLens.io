// SemanticTagGenerator.h — Auto-Generate Semantic Tags for Images
// Copyright (c) 2026 ExplorerLens Project
//
// Generates semantic tags for images using lightweight feature extraction
// and a vocabulary-based classification approach with confidence thresholds.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <unordered_map>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class TagConfidenceLevel : uint32_t {
    VeryHigh = 0,  // > 0.9
    High = 1,  // > 0.7
    Medium = 2,  // > 0.5
    Low = 3,  // > 0.3
    VeryLow = 4   // <= 0.3
};

struct SemanticTag {
    std::string       tag;
    double            confidence = 0.0;
    TagConfidenceLevel level = TagConfidenceLevel::VeryLow;
    std::string       category;      // e.g., "scene", "object", "color"

    bool operator<(const SemanticTag& other) const {
        return confidence > other.confidence;
    }

    static TagConfidenceLevel ClassifyConfidence(double conf) {
        if (conf > 0.9) return TagConfidenceLevel::VeryHigh;
        if (conf > 0.7) return TagConfidenceLevel::High;
        if (conf > 0.5) return TagConfidenceLevel::Medium;
        if (conf > 0.3) return TagConfidenceLevel::Low;
        return TagConfidenceLevel::VeryLow;
    }
};

class SemanticTagGenerator {
public:
    static SemanticTagGenerator& Instance() {
        static SemanticTagGenerator s;
        return s;
    }

    std::vector<SemanticTag> GenerateTags(const uint8_t* pixels, uint32_t width,
        uint32_t height, uint32_t channels) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<SemanticTag> tags;

        if (!pixels || width == 0 || height == 0) return tags;

        // Analyze color distribution
        double avgR = 0, avgG = 0, avgB = 0;
        size_t pixelCount = static_cast<size_t>(width) * height;
        size_t sampleStep = (std::max)(pixelCount / 10000, static_cast<size_t>(1));

        for (size_t i = 0; i < pixelCount; i += sampleStep) {
            size_t idx = i * channels;
            avgR += pixels[idx];
            if (channels > 1) avgG += pixels[idx + 1];
            if (channels > 2) avgB += pixels[idx + 2];
        }

        size_t samples = (pixelCount + sampleStep - 1) / sampleStep;
        avgR /= samples; avgG /= samples; avgB /= samples;

        // Color-based tags
        if (avgB > 150 && avgR < 100) AddTag(tags, "sky", 0.7, "scene");
        if (avgG > 150 && avgR < 120) AddTag(tags, "nature", 0.65, "scene");
        if (avgR > 200 && avgG < 100) AddTag(tags, "warm", 0.6, "color");
        if (avgR < 50 && avgG < 50 && avgB < 50) AddTag(tags, "dark", 0.8, "lighting");
        if (avgR > 200 && avgG > 200 && avgB > 200) AddTag(tags, "bright", 0.8, "lighting");

        // Size-based tags
        if (width > 3840) AddTag(tags, "high-resolution", 0.95, "quality");
        else if (width > 1920) AddTag(tags, "full-hd", 0.9, "quality");
        else if (width < 320) AddTag(tags, "thumbnail", 0.85, "quality");

        // Aspect ratio tags
        double aspect = static_cast<double>(width) / height;
        if (aspect > 1.7) AddTag(tags, "panoramic", 0.75, "composition");
        else if (aspect < 0.7) AddTag(tags, "portrait", 0.8, "composition");
        else AddTag(tags, "landscape", 0.6, "composition");

        // Variance analysis for detail
        double variance = ComputeVariance(pixels, pixelCount, channels, sampleStep, avgR);
        if (variance > 5000) AddTag(tags, "detailed", 0.7, "content");
        else if (variance < 500) AddTag(tags, "minimal", 0.65, "content");

        // Filter by threshold
        tags.erase(std::remove_if(tags.begin(), tags.end(),
            [this](const SemanticTag& t) { return t.confidence < m_threshold; }),
            tags.end());
        std::sort(tags.begin(), tags.end());

        m_totalGenerated += tags.size();
        return tags;
    }

    std::vector<std::string> GetVocabulary() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return { "sky", "nature", "warm", "dark", "bright",
                 "high-resolution", "full-hd", "thumbnail",
                 "panoramic", "portrait", "landscape",
                 "detailed", "minimal" };
    }

    void SetThreshold(double threshold) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_threshold = (std::max)(0.0, (std::min)(1.0, threshold));
    }

    double GetThreshold() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_threshold;
    }

    uint64_t GetTotalGenerated() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalGenerated;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_threshold = 0.5;
        m_totalGenerated = 0;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_threshold < 0.0 || m_threshold > 1.0) return false;
        return true;
    }

private:
    SemanticTagGenerator() = default;
    ~SemanticTagGenerator() = default;
    SemanticTagGenerator(const SemanticTagGenerator&) = delete;
    SemanticTagGenerator& operator=(const SemanticTagGenerator&) = delete;

    void AddTag(std::vector<SemanticTag>& tags, const std::string& name,
        double conf, const std::string& cat) {
        SemanticTag t;
        t.tag = name;
        t.confidence = conf;
        t.level = SemanticTag::ClassifyConfidence(conf);
        t.category = cat;
        tags.push_back(t);
    }

    double ComputeVariance(const uint8_t* pixels, size_t count, uint32_t channels,
        size_t step, double mean) const {
        double sumSq = 0.0;
        size_t n = 0;
        for (size_t i = 0; i < count; i += step) {
            double val = static_cast<double>(pixels[i * channels]);
            sumSq += (val - mean) * (val - mean);
            n++;
        }
        return n > 0 ? sumSq / n : 0.0;
    }

    mutable std::mutex m_mutex;
    double   m_threshold = 0.5;
    uint64_t m_totalGenerated = 0;
};

}
} // namespace ExplorerLens::Engine
