// FormatHealthIndicator.h — Decoder Health Traffic Light Indicators
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors decoder health for each format using a traffic-light model
// (Green/Yellow/Red) based on success rates, latency, and error counts.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HealthIndicatorState : uint32_t {
    Green = 0,
    Yellow = 1,
    Red = 2,
    Unknown = 3
};

struct FormatHealthReport
{
    std::string formatName;
    HealthIndicatorState state = HealthIndicatorState::Unknown;
    uint64_t successCount = 0;
    uint64_t failureCount = 0;
    uint64_t totalCount = 0;
    double avgLatencyMs = 0.0;
    double p95LatencyMs = 0.0;
    double successRate = 0.0;
    uint64_t lastCheckMs = 0;
    std::string lastError;

    void Recalculate()
    {
        totalCount = successCount + failureCount;
        successRate = totalCount > 0 ? static_cast<double>(successCount) / totalCount : 0.0;

        if (totalCount == 0) {
            state = HealthIndicatorState::Unknown;
        } else if (successRate >= 0.95 && avgLatencyMs < 100.0) {
            state = HealthIndicatorState::Green;
        } else if (successRate >= 0.80 && avgLatencyMs < 500.0) {
            state = HealthIndicatorState::Yellow;
        } else {
            state = HealthIndicatorState::Red;
        }
    }
};

class FormatHealthIndicator
{
  public:
    static FormatHealthIndicator& Instance()
    {
        static FormatHealthIndicator s;
        return s;
    }

    void RecordSuccess(const std::string& format, double latencyMs)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& report = m_reports[format];
        report.formatName = format;
        report.successCount++;
        report.lastCheckMs = GetTickCount64();

        // Update running average
        double total = static_cast<double>(report.successCount + report.failureCount);
        report.avgLatencyMs = ((total - 1) * report.avgLatencyMs + latencyMs) / total;

        // Track latencies for P95
        m_latencies[format].push_back(latencyMs);
        UpdateP95(format);
        report.Recalculate();
    }

    void RecordFailure(const std::string& format, const std::string& error)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& report = m_reports[format];
        report.formatName = format;
        report.failureCount++;
        report.lastError = error;
        report.lastCheckMs = GetTickCount64();
        report.Recalculate();
    }

    void Assess(const std::string& format)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_reports.find(format);
        if (it != m_reports.end()) {
            it->second.Recalculate();
        }
    }

    FormatHealthReport GetHealthForFormat(const std::string& format) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_reports.find(format);
        if (it != m_reports.end())
            return it->second;
        FormatHealthReport unknown;
        unknown.formatName = format;
        return unknown;
    }

    std::vector<FormatHealthReport> GetAllHealth() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<FormatHealthReport> all;
        for (const auto& [name, report] : m_reports) {
            all.push_back(report);
        }
        // Sort: Red first, then Yellow, then Green, then Unknown
        std::sort(all.begin(), all.end(), [](const FormatHealthReport& a, const FormatHealthReport& b) {
            return static_cast<uint32_t>(a.state) > static_cast<uint32_t>(b.state);
        });
        return all;
    }

    size_t CountByState(HealthIndicatorState state) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (const auto& [name, report] : m_reports) {
            if (report.state == state)
                count++;
        }
        return count;
    }

    void Reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_reports.clear();
        m_latencies.clear();
    }

    bool Validate() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [name, report] : m_reports) {
            if (report.formatName.empty())
                return false;
            if (report.successRate < 0.0 || report.successRate > 1.0)
                return false;
            if (report.avgLatencyMs < 0.0)
                return false;
            if (report.successCount + report.failureCount != report.totalCount)
                return false;
        }
        return true;
    }

  private:
    FormatHealthIndicator() = default;
    ~FormatHealthIndicator() = default;
    FormatHealthIndicator(const FormatHealthIndicator&) = delete;
    FormatHealthIndicator& operator=(const FormatHealthIndicator&) = delete;

    void UpdateP95(const std::string& format)
    {
        auto& lats = m_latencies[format];
        if (lats.empty())
            return;

        std::vector<double> sorted = lats;
        std::sort(sorted.begin(), sorted.end());
        size_t idx = static_cast<size_t>(sorted.size() * 0.95);
        if (idx >= sorted.size())
            idx = sorted.size() - 1;
        m_reports[format].p95LatencyMs = sorted[idx];

        // Keep only last 1000 entries
        if (lats.size() > 1000) {
            lats.erase(lats.begin(), lats.begin() + static_cast<ptrdiff_t>(lats.size() - 1000));
        }
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, FormatHealthReport> m_reports;
    std::unordered_map<std::string, std::vector<double>> m_latencies;
};

}  // namespace Engine
}  // namespace ExplorerLens
