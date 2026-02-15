// Performance Profiling System for DarkThumbs
// Tracks timing metrics and performance bottlenecks

#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>

namespace DarkThumbs {

struct PerformanceMetric {
    std::string name;
    uint64_t callCount;
    double totalTimeMs;
    double minTimeMs;
    double maxTimeMs;
    double avgTimeMs;

    PerformanceMetric() : callCount(0), totalTimeMs(0), minTimeMs(DBL_MAX), maxTimeMs(0), avgTimeMs(0) {}

    void AddSample(double timeMs) {
        callCount++;
        totalTimeMs += timeMs;
        minTimeMs = min(minTimeMs, timeMs);
        maxTimeMs = max(maxTimeMs, timeMs);
        avgTimeMs = totalTimeMs / callCount;
    }

    std::string ToString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3)
            << name << ": calls=" << callCount
            << ", avg=" << avgTimeMs << "ms"
            << ", min=" << minTimeMs << "ms"
            << ", max=" << maxTimeMs << "ms"
            << ", total=" << totalTimeMs << "ms";
        return oss.str();
    }
};

class PerformanceProfiler {
private:
    std::unordered_map<std::string, PerformanceMetric> m_metrics;
    std::mutex m_mutex;
    bool m_enabled;

    PerformanceProfiler() : m_enabled(false) {
        // Check if profiling is enabled via registry
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\DarkThumbs\\Settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD profiling = 0;
            DWORD size = sizeof(DWORD);
            if (RegQueryValueExW(hKey, L"EnableProfiling", nullptr, nullptr, (LPBYTE)&profiling, &size) == ERROR_SUCCESS) {
                m_enabled = (profiling != 0);
            }
            RegCloseKey(hKey);
        }
    }

public:
    static PerformanceProfiler& Instance() {
        static PerformanceProfiler instance;
        return instance;
    }

    void RecordSample(const std::string& metricName, double timeMs) {
        if (!m_enabled) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_metrics[metricName].name = metricName;
        m_metrics[metricName].AddSample(timeMs);
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_metrics.clear();
    }

    std::string GetReport() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        
        std::ostringstream report;
        report << "Performance Profiling Report\n";
        report << "============================\n\n";

        for (const auto& pair : m_metrics) {
            report << pair.second.ToString() << "\n";
        }

        return report.str();
    }

    const PerformanceMetric* GetMetric(const std::string& name) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        auto it = m_metrics.find(name);
        return (it != m_metrics.end()) ? &it->second : nullptr;
    }

    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
};

// RAII timer for automatic profiling
class CBXScopedTimer {
private:
    std::string m_metricName;
    std::chrono::high_resolution_clock::time_point m_start;
    bool m_enabled;

public:
    explicit CBXScopedTimer(const std::string& metricName)
        : m_metricName(metricName)
        , m_enabled(PerformanceProfiler::Instance().IsEnabled())
    {
        if (m_enabled) {
            m_start = std::chrono::high_resolution_clock::now();
        }
    }

    ~CBXScopedTimer() {
        if (m_enabled) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
            double ms = duration.count() / 1000.0;
            PerformanceProfiler::Instance().RecordSample(m_metricName, ms);
        }
    }

    // Non-copyable
    CBXScopedTimer(const CBXScopedTimer&) = delete;
    CBXScopedTimer& operator=(const CBXScopedTimer&) = delete;
};

// Convenience macro
#define PROFILE_SCOPE(name) DarkThumbs::CBXScopedTimer __profiler_timer__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

} // namespace DarkThumbs
