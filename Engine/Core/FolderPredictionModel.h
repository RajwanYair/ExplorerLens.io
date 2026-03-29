// FolderPredictionModel.h — Folder Prediction Model
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which folders the user is likely to open next based on Markov chain analysis.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct FPMAccessRecord {
    std::wstring folderPath;
    uint64_t     timestampMs = 0;
};

struct FPMPrediction {
    std::wstring folderPath;
    float        probability = 0.0f;
};

class FolderPredictionModel {
public:
    void RecordAccess(const FPMAccessRecord& record) {
        if (!m_lastFolder.empty())
            m_transitions[m_lastFolder][record.folderPath]++;
        m_lastFolder = record.folderPath;
        m_totalAccesses++;
    }
    std::vector<FPMPrediction> TopPredictions(uint32_t topK) const {
        std::vector<FPMPrediction> preds;
        auto it = m_transitions.find(m_lastFolder);
        if (it == m_transitions.end()) return preds;
        uint32_t total = 0;
        for (const auto& [k, v] : it->second) total += v;
        for (const auto& [folder, count] : it->second)
            preds.push_back({ folder, total > 0 ? static_cast<float>(count) / static_cast<float>(total) : 0.0f });
        std::sort(preds.begin(), preds.end(),
            [](const FPMPrediction& a, const FPMPrediction& b){ return a.probability > b.probability; });
        if (preds.size() > topK) preds.resize(topK);
        return preds;
    }
    uint32_t TransitionCount() const { return static_cast<uint32_t>(m_transitions.size()); }
    uint32_t TotalAccesses()   const { return m_totalAccesses; }

private:
    std::unordered_map<std::wstring, std::unordered_map<std::wstring, uint32_t>> m_transitions;
    std::wstring m_lastFolder;
    uint32_t     m_totalAccesses = 0;
};

}} // namespace ExplorerLens::Engine
