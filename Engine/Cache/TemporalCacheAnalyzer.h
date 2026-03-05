// TemporalCacheAnalyzer.h — Time-Series Cache Access Analysis
// Copyright (c) 2026 ExplorerLens Project
//
// Time-series cache access analysis. Detects hourly/daily access patterns,
// optimizes temporal eviction policy to keep entries warm before predictable demand.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <mutex>
#include <cmath>
#include <numeric>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct HourlyBucket {
    uint64_t accessCount = 0;
    uint64_t hitCount = 0;
    uint64_t missCount = 0;
    double avgResponseTimeMs = 0.0;
    double totalResponseTimeMs = 0.0;
};

struct TemporalPattern {
    std::array<double, 24> hourlyDistribution = {};
    std::array<double, 7> dailyDistribution = {};
    double peakHourFraction = 0.0;
    uint32_t peakHour = 0;
    uint32_t peakDay = 0;
    bool hasStrongDailyPattern = false;
    bool hasStrongWeeklyPattern = false;
};

struct EvictionAdvice {
    std::string key;
    double survivalProbability = 0.0;
    double hoursUntilPeakDemand = 24.0;
    bool shouldEvict = true;
    std::string reason;
};

class TemporalCacheAnalyzer {
public:
    static TemporalCacheAnalyzer& Instance() {
        static TemporalCacheAnalyzer instance;
        return instance;
    }

    inline void RecordAccess(const std::string& key, uint32_t hour, uint32_t dayOfWeek, bool wasHit,
        double responseTimeMs = 0.0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        hour = hour % 24;
        dayOfWeek = dayOfWeek % 7;

        m_hourlyBuckets[hour].accessCount++;
        if (wasHit) m_hourlyBuckets[hour].hitCount++;
        else m_hourlyBuckets[hour].missCount++;
        m_hourlyBuckets[hour].totalResponseTimeMs += responseTimeMs;
        if (m_hourlyBuckets[hour].accessCount > 0) {
            m_hourlyBuckets[hour].avgResponseTimeMs =
                m_hourlyBuckets[hour].totalResponseTimeMs / m_hourlyBuckets[hour].accessCount;
        }

        m_keyHourly[key][hour]++;
        m_keyDaily[key][dayOfWeek]++;
        m_totalAccesses++;
    }

    inline TemporalPattern AnalyzePatterns() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        TemporalPattern pattern;

        if (m_totalAccesses == 0) return pattern;

        uint64_t maxHourly = 0;
        for (uint32_t h = 0; h < 24; ++h) {
            pattern.hourlyDistribution[h] = static_cast<double>(m_hourlyBuckets[h].accessCount) / m_totalAccesses;
            if (m_hourlyBuckets[h].accessCount > maxHourly) {
                maxHourly = m_hourlyBuckets[h].accessCount;
                pattern.peakHour = h;
            }
        }
        pattern.peakHourFraction = static_cast<double>(maxHourly) / m_totalAccesses;

        double hourlyEntropy = ComputeEntropy(pattern.hourlyDistribution.data(), 24);
        pattern.hasStrongDailyPattern = hourlyEntropy < 3.5;

        for (uint32_t d = 0; d < 7; ++d) {
            uint64_t dayTotal = 0;
            for (const auto& [key, hourMap] : m_keyHourly) {
                auto dailyIt = m_keyDaily.find(key);
                if (dailyIt != m_keyDaily.end()) {
                    auto dIt = dailyIt->second.find(d);
                    if (dIt != dailyIt->second.end()) dayTotal += dIt->second;
                }
            }
            pattern.dailyDistribution[d] = static_cast<double>(dayTotal) / (std::max)(m_totalAccesses, static_cast<uint64_t>(1));
        }

        uint64_t maxDaily = 0;
        for (uint32_t d = 0; d < 7; ++d) {
            uint64_t count = static_cast<uint64_t>(pattern.dailyDistribution[d] * m_totalAccesses);
            if (count > maxDaily) {
                maxDaily = count;
                pattern.peakDay = d;
            }
        }
        pattern.hasStrongWeeklyPattern = ComputeEntropy(pattern.dailyDistribution.data(), 7) < 2.2;

        return pattern;
    }

    inline EvictionAdvice GetEvictionAdvice(const std::string& key, uint32_t currentHour) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        EvictionAdvice advice;
        advice.key = key;

        auto keyIt = m_keyHourly.find(key);
        if (keyIt == m_keyHourly.end()) {
            advice.shouldEvict = true;
            advice.survivalProbability = 0.0;
            advice.reason = "No access history";
            return advice;
        }

        uint64_t totalForKey = 0;
        uint64_t peakCount = 0;
        uint32_t peakHour = 0;
        for (const auto& [h, count] : keyIt->second) {
            totalForKey += count;
            if (count > peakCount) {
                peakCount = count;
                peakHour = h;
            }
        }

        int hoursUntilPeak = static_cast<int>(peakHour) - static_cast<int>(currentHour % 24);
        if (hoursUntilPeak < 0) hoursUntilPeak += 24;
        advice.hoursUntilPeakDemand = static_cast<double>(hoursUntilPeak);

        advice.survivalProbability = totalForKey > 5 ?
            (std::min)(1.0, static_cast<double>(peakCount) / totalForKey * (1.0 - hoursUntilPeak / 24.0)) : 0.1;

        advice.shouldEvict = advice.survivalProbability < 0.3;
        advice.reason = advice.shouldEvict ? "Low predicted demand" : "High predicted demand in " +
            std::to_string(hoursUntilPeak) + "h";
        return advice;
    }

    inline HourlyBucket GetHourlyStats(uint32_t hour) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hourlyBuckets[hour % 24];
    }

private:
    TemporalCacheAnalyzer() = default;

    inline double ComputeEntropy(const double* dist, size_t count) const {
        double entropy = 0.0;
        for (size_t i = 0; i < count; ++i) {
            if (dist[i] > 0.0) {
                entropy -= dist[i] * std::log2(dist[i]);
            }
        }
        return entropy;
    }

    mutable std::mutex m_mutex;
    std::array<HourlyBucket, 24> m_hourlyBuckets = {};
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint64_t>> m_keyHourly;
    std::unordered_map<std::string, std::unordered_map<uint32_t, uint64_t>> m_keyDaily;
    uint64_t m_totalAccesses = 0;
};

}
} // namespace ExplorerLens::Engine
