// DecodeLatencyProfiler.cpp — Per-Format Decode Latency Profiler
// Copyright (c) 2026 ExplorerLens Project
//
#include "DecodeLatencyProfiler.h"
#include <numeric>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

void DecodeLatencyProfiler::RecordSample(const std::string& formatTag, double latencyMs)
{
    m_rings[formatTag].Push(latencyMs);
}

LatencyPercentiles DecodeLatencyProfiler::GetPercentiles(const std::string& formatTag) const
{
    LatencyPercentiles p{};
    p.formatTag = formatTag;

    auto it = m_rings.find(formatTag);
    if (it == m_rings.end() || it->second.count == 0)
        return p;

    const auto& ring = it->second;
    const auto  sorted = ring.Sorted();

    p.sampleCount = ring.count;
    p.p50Ms  = Ring::Percentile(sorted, 50.0);
    p.p95Ms  = Ring::Percentile(sorted, 95.0);
    p.p99Ms  = Ring::Percentile(sorted, 99.0);
    p.minMs  = sorted.front();
    p.maxMs  = sorted.back();
    p.meanMs = std::accumulate(sorted.begin(), sorted.end(), 0.0) / sorted.size();

    return p;
}

double DecodeLatencyProfiler::GetP50(const std::string& formatTag) const
{
    return GetPercentiles(formatTag).p50Ms;
}

double DecodeLatencyProfiler::GetP95(const std::string& formatTag) const
{
    return GetPercentiles(formatTag).p95Ms;
}

double DecodeLatencyProfiler::GetP99(const std::string& formatTag) const
{
    return GetPercentiles(formatTag).p99Ms;
}

uint32_t DecodeLatencyProfiler::SampleCount(const std::string& formatTag) const noexcept
{
    auto it = m_rings.find(formatTag);
    return it != m_rings.end() ? it->second.count : 0u;
}

std::string DecodeLatencyProfiler::ToJSON() const
{
    std::ostringstream oss;
    oss << "[\n";
    bool first = true;
    for (const auto& kv : m_rings) {
        if (!first) oss << ",\n";
        first = false;
        const auto p = GetPercentiles(kv.first);
        oss << "  {\"format\":\"" << kv.first << "\""
            << ",\"samples\":" << p.sampleCount
            << ",\"p50\":" << p.p50Ms
            << ",\"p95\":" << p.p95Ms
            << ",\"p99\":" << p.p99Ms
            << ",\"min\":" << p.minMs
            << ",\"max\":" << p.maxMs
            << ",\"mean\":" << p.meanMs
            << "}";
    }
    oss << "\n]";
    return oss.str();
}

} // namespace Engine
} // namespace ExplorerLens
