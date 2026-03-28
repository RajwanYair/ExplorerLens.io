// FormatDetectionReport.h — Confidence-Scored Format Detection Report
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates detection results from multiple classifier sources into a single
// ranked report with per-source confidence scores and a final consensus answer.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class DetectionSource  { MagicBytes, Extension, ContentHash, NeuralClassifier, LLMInference };
enum class DetectionVerdict { Confirmed, Probable, Uncertain, Conflicted };

struct DetectionVote {
    DetectionSource source;
    std::string     formatId;
    std::string     mimeType;
    float           confidence  = 0.0f;
};

struct FormatDetectionReport {
    std::vector<DetectionVote> votes;
    std::string                consensusFormatId;
    std::string                consensusMime;
    float                      consensusConfidence = 0.0f;
    DetectionVerdict           verdict             = DetectionVerdict::Uncertain;
    uint64_t                   evaluationMs        = 0;

    void AddVote(const DetectionVote& v) { votes.push_back(v); Recalculate(); }

    void Recalculate() {
        if (votes.empty()) return;
        const auto& best = *std::max_element(votes.begin(), votes.end(),
            [](const DetectionVote& a, const DetectionVote& b){ return a.confidence < b.confidence; });
        consensusFormatId   = best.formatId;
        consensusMime       = best.mimeType;
        consensusConfidence = best.confidence;
        if      (best.confidence >= 0.90f) verdict = DetectionVerdict::Confirmed;
        else if (best.confidence >= 0.70f) verdict = DetectionVerdict::Probable;
        else if (best.confidence >= 0.50f) verdict = DetectionVerdict::Uncertain;
        else                               verdict = DetectionVerdict::Conflicted;
    }

    bool  IsResolved()  const { return verdict == DetectionVerdict::Confirmed ||
                                       verdict == DetectionVerdict::Probable; }
    size_t VoteCount()  const { return votes.size(); }
    void   Clear()            { votes.clear(); consensusFormatId.clear(); consensusConfidence = 0.0f; }
};

class FormatDetectionReportBuilder {
public:
    FormatDetectionReportBuilder() { m_report.evaluationMs = 0; }

    void  AddVote(DetectionSource src, const std::string& fmt,
                  const std::string& mime, float conf) {
        m_report.AddVote({ src, fmt, mime, conf });
    }
    void  SetEvalTime(uint64_t ms) { m_report.evaluationMs = ms; }
    const FormatDetectionReport& GetReport() const { return m_report; }
    void  Reset()                              { m_report = FormatDetectionReport{}; }

private:
    FormatDetectionReport m_report;
};

} // namespace Engine
} // namespace ExplorerLens
