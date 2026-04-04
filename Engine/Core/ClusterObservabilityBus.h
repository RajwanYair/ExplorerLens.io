// ClusterObservabilityBus.h — Cluster Observability Bus
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates distributed trace spans and metrics across render cluster nodes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class COBSpanStatus {
    Ok,
    Error,
    Timeout
};

struct COBSpan
{
    std::string spanId;
    std::string operationName;
    uint64_t startMs = 0;
    uint64_t durationMs = 0;
    COBSpanStatus status = COBSpanStatus::Ok;
    std::string nodeId;
};

struct COBMetricSnapshot
{
    uint32_t totalSpans = 0;
    uint32_t errorSpans = 0;
    float avgDurationMs = 0.0f;
};

class ClusterObservabilityBus
{
  public:
    void RecordSpan(const COBSpan& span)
    {
        m_spans.push_back(span);
    }

    std::vector<COBSpan> QuerySpans(const std::string& nodeId) const
    {
        std::vector<COBSpan> out;
        for (const auto& s : m_spans)
            if (nodeId.empty() || s.nodeId == nodeId)
                out.push_back(s);
        return out;
    }

    COBMetricSnapshot Snapshot() const
    {
        COBMetricSnapshot s;
        s.totalSpans = static_cast<uint32_t>(m_spans.size());
        float totalDur = 0.0f;
        for (const auto& sp : m_spans) {
            totalDur += static_cast<float>(sp.durationMs);
            if (sp.status != COBSpanStatus::Ok)
                ++s.errorSpans;
        }
        s.avgDurationMs = (s.totalSpans > 0) ? totalDur / static_cast<float>(s.totalSpans) : 0.0f;
        return s;
    }

    void Clear()
    {
        m_spans.clear();
    }

  private:
    std::vector<COBSpan> m_spans;
};

}  // namespace Engine
}  // namespace ExplorerLens
