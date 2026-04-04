// PredictionAccuracyTracker.h — Prediction Accuracy Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Measures hit/miss ratio of predictive pre-generation to drive model improvement feedback.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PATSample
{
    std::wstring predictedPath;
    bool wasViewed = false;
    float confidence = 0.0f;
};

struct PATAccuracyReport
{
    uint32_t totalPredictions = 0;
    uint32_t hits = 0;
    float hitRate = 0.0f;
    float avgConfidence = 0.0f;
};

class PredictionAccuracyTracker
{
  public:
    void Record(const PATSample& sample)
    {
        m_samples.push_back(sample);
    }

    PATAccuracyReport Compute() const
    {
        PATAccuracyReport r;
        r.totalPredictions = static_cast<uint32_t>(m_samples.size());
        if (r.totalPredictions == 0)
            return r;
        float confSum = 0.0f;
        for (const auto& s : m_samples) {
            if (s.wasViewed)
                ++r.hits;
            confSum += s.confidence;
        }
        r.hitRate = static_cast<float>(r.hits) / static_cast<float>(r.totalPredictions);
        r.avgConfidence = confSum / static_cast<float>(r.totalPredictions);
        return r;
    }
    void Clear()
    {
        m_samples.clear();
    }
    uint32_t SampleCount() const
    {
        return static_cast<uint32_t>(m_samples.size());
    }

  private:
    std::vector<PATSample> m_samples;
};

}  // namespace Engine
}  // namespace ExplorerLens
