// FormatDetectionReport.h — Format Detection Report
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates multi-source format detection votes (magic bytes, extension,
// neural classifier, content hash) and builds a consensus detection report
// with a confidence-weighted verdict.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DetectionSource : uint8_t {
    MagicBytes = 0,
    Extension = 1,
    NeuralClassifier = 2,
    ContentHash = 3,
};

enum class DetectionVerdict : uint8_t {
    Confirmed = 0,   // confidence >= 0.90
    Probable = 1,    // confidence >= 0.70
    Conflicted = 2,  // confidence  < 0.70 or contradictory votes
};

struct DetectionVote
{
    DetectionSource source;
    std::string formatId;
    std::string mimeType;
    float confidence = 0.0f;
};

struct FormatDetectionReport
{
    std::string consensusFormatId;
    std::string consensusMimeType;
    float consensusConfidence = 0.0f;
    DetectionVerdict verdict = DetectionVerdict::Conflicted;
    uint32_t evaluationMs = 0;
    std::vector<DetectionVote> votes;

    uint32_t VoteCount() const
    {
        return static_cast<uint32_t>(votes.size());
    }

    bool IsResolved() const
    {
        return verdict != DetectionVerdict::Conflicted;
    }

    static DetectionVerdict VerdictFromConfidence(float c)
    {
        if (c >= 0.90f)
            return DetectionVerdict::Confirmed;
        if (c >= 0.70f)
            return DetectionVerdict::Probable;
        return DetectionVerdict::Conflicted;
    }
};

class FormatDetectionReportBuilder
{
  public:
    FormatDetectionReportBuilder() = default;

    void AddVote(DetectionSource source, const std::string& formatId, const std::string& mimeType, float confidence)
    {
        DetectionVote v;
        v.source = source;
        v.formatId = formatId;
        v.mimeType = mimeType;
        v.confidence = confidence;
        m_report.votes.push_back(v);
        Recompute();
    }

    void SetEvalTime(uint32_t ms)
    {
        m_report.evaluationMs = ms;
    }

    void Reset()
    {
        m_report = FormatDetectionReport{};
    }

    const FormatDetectionReport& GetReport() const
    {
        return m_report;
    }

  private:
    FormatDetectionReport m_report;

    void Recompute()
    {
        if (m_report.votes.empty()) {
            m_report.consensusFormatId = "";
            m_report.consensusMimeType = "";
            m_report.consensusConfidence = 0.0f;
            m_report.verdict = DetectionVerdict::Conflicted;
            return;
        }

        // Find the format with the highest individual vote confidence
        const DetectionVote* best = &m_report.votes[0];
        for (const auto& v : m_report.votes) {
            if (v.confidence > best->confidence)
                best = &v;
        }

        // Compute aggregate confidence for the winning format (max across all votes for it)
        float aggConf = 0.0f;
        for (const auto& v : m_report.votes) {
            if (v.formatId == best->formatId && v.confidence > aggConf)
                aggConf = v.confidence;
        }

        m_report.consensusFormatId = best->formatId;
        m_report.consensusMimeType = best->mimeType;
        m_report.consensusConfidence = aggConf;
        m_report.verdict = FormatDetectionReport::VerdictFromConfidence(aggConf);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
