// ErrorReportingPipeline.h — Structured error aggregation pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Provides ErrorDomain, ErrorAggregation enums and ErrorReportingPipeline for
// collecting, deduplicating, and querying runtime errors by domain and aggregation.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ErrorDomain : uint8_t {
    GPU = 0,
    Decoder = 1,
    COM = 2,
    Cache = 3,
    IO = 4,
    Network = 5,
    Plugin = 6,
    Memory = 7,
    COUNT
};

enum class ErrorAggregation : uint8_t {
    Total = 0,
    PerFile = 1,
    PerSession = 2,
    COUNT
};

inline const char* ErrorDomainName(ErrorDomain d) noexcept
{
    switch (d) {
        case ErrorDomain::GPU:
            return "GPU";
        case ErrorDomain::Decoder:
            return "Decoder";
        case ErrorDomain::COM:
            return "COM";
        case ErrorDomain::Cache:
            return "Cache";
        case ErrorDomain::IO:
            return "IO";
        case ErrorDomain::Network:
            return "Network";
        case ErrorDomain::Plugin:
            return "Plugin";
        case ErrorDomain::Memory:
            return "Memory";
        default:
            return "Unknown";
    }
}

inline const char* ErrorAggregationName(ErrorAggregation a) noexcept
{
    switch (a) {
        case ErrorAggregation::Total:
            return "Total";
        case ErrorAggregation::PerFile:
            return "PerFile";
        case ErrorAggregation::PerSession:
            return "PerSession";
        default:
            return "Unknown";
    }
}

struct ErrorReport
{
    ErrorDomain domain = ErrorDomain::GPU;
    ErrorAggregation aggregation = ErrorAggregation::Total;
    uint32_t count = 0;
    std::string message;
};

class ErrorReportingPipeline
{
  public:
    void Report(ErrorDomain domain, ErrorAggregation agg, const char* msg)
    {
        auto key = std::make_pair(domain, agg);
        auto& rep = m_buckets[key];
        rep.domain = domain;
        rep.aggregation = agg;
        rep.count++;
        if (rep.message.empty() && msg) {
            rep.message = msg;
        }
    }

    std::vector<ErrorReport> GetReports() const
    {
        std::vector<ErrorReport> result;
        result.reserve(m_buckets.size());
        for (auto& [k, v] : m_buckets) {
            result.push_back(v);
        }
        return result;
    }

    size_t GetBucketCount() const noexcept
    {
        return m_buckets.size();
    }

    size_t GetTotalErrorCount() const noexcept
    {
        size_t total = 0;
        for (auto& [k, v] : m_buckets) {
            total += v.count;
        }
        return total;
    }

    std::vector<ErrorReport> GetTopErrors(size_t n) const
    {
        auto result = GetReports();
        std::sort(result.begin(), result.end(),
                  [](const ErrorReport& a, const ErrorReport& b) { return a.count > b.count; });
        if (result.size() > n) {
            result.resize(n);
        }
        return result;
    }

    void Clear() noexcept
    {
        m_buckets.clear();
    }

  private:
    std::map<std::pair<ErrorDomain, ErrorAggregation>, ErrorReport> m_buckets;
};

}  // namespace Engine
}  // namespace ExplorerLens
