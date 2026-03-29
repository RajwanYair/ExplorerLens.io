// PredictivePreGenEngine.h — Predictive Pre-Generation Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-generates thumbnails for likely-to-be-viewed files using ML access pattern prediction.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

namespace ExplorerLens { namespace Engine {

struct PPGEConfig {
    uint32_t lookAheadCount   = 10;
    float    minConfidence    = 0.6f;
    bool     gpuAccelerate    = true;
};

struct PPGEPrediction {
    std::wstring path;
    float        confidence   = 0.0f;
    uint32_t     priority     = 0;
};

class PredictivePreGenEngine {
public:
    explicit PredictivePreGenEngine(const PPGEConfig& config) : m_config(config) {}

    void RecordAccess(const std::wstring& path) {
        m_accessFreq[path]++;
        m_recentAccesses.push_back(path);
        if (m_recentAccesses.size() > 100) m_recentAccesses.pop_front();
    }
    std::vector<PPGEPrediction> Predict() {
        std::vector<PPGEPrediction> preds;
        for (const auto& [path, freq] : m_accessFreq) {
            float conf = std::min(1.0f, static_cast<float>(freq) / 10.0f);
            if (conf >= m_config.minConfidence) {
                PPGEPrediction p;
                p.path       = path;
                p.confidence = conf;
                p.priority   = freq;
                preds.push_back(p);
            }
            if (preds.size() >= m_config.lookAheadCount) break;
        }
        return preds;
    }
    uint32_t TrackedPathCount() const { return static_cast<uint32_t>(m_accessFreq.size()); }

private:
    PPGEConfig                              m_config;
    std::unordered_map<std::wstring, uint32_t>  m_accessFreq;
    std::deque<std::wstring>                m_recentAccesses;
};

}} // namespace ExplorerLens::Engine
