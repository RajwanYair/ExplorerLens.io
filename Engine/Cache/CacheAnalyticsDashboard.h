// CacheAnalyticsDashboard.h — Cache Analytics Visualization Data
// Copyright (c) 2026 ExplorerLens Project
//
// Collects and exposes cache performance metrics suitable for dashboard
// visualization, including hit-rate time series and eviction statistics.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

struct CacheRegionStats {
    std::string regionName;
    uint64_t hitCount = 0;
    uint64_t missCount = 0;
    uint64_t evictionCount = 0;
    uint64_t totalSizeBytes = 0;
    uint64_t usedSizeBytes = 0;

    double HitRate() const {
        uint64_t total = hitCount + missCount;
        return total > 0 ? static_cast<double>(hitCount) / total : 0.0;
    }

    double Utilization() const {
        return totalSizeBytes > 0 ?
            static_cast<double>(usedSizeBytes) / totalSizeBytes : 0.0;
    }
};

struct CacheTimeSeries {
    struct DataPoint {
        uint64_t timestampMs = 0;
        double   hitRate = 0.0;
        uint64_t entryCount = 0;
        uint64_t memoryUsed = 0;
    };

    std::string seriesName;
    std::vector<DataPoint> points;
    size_t maxPoints = 1000;

    void AddPoint(double hitRate, uint64_t entries, uint64_t memUsed) {
        DataPoint dp;
        dp.timestampMs = GetTickCount64();
        dp.hitRate = hitRate;
        dp.entryCount = entries;
        dp.memoryUsed = memUsed;
        points.push_back(dp);
        if (points.size() > maxPoints) {
            points.erase(points.begin());
        }
    }

    double AverageHitRate() const {
        if (points.empty()) return 0.0;
        double sum = 0.0;
        for (const auto& p : points) sum += p.hitRate;
        return sum / points.size();
    }
};

class CacheAnalyticsDashboard {
public:
    static CacheAnalyticsDashboard& Instance() {
        static CacheAnalyticsDashboard s;
        return s;
    }

    void CollectMetrics(const std::string& region, uint64_t hits, uint64_t misses,
        uint64_t evictions, uint64_t totalSize, uint64_t usedSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindRegion(region);
        if (it == m_regions.end()) {
            CacheRegionStats stats;
            stats.regionName = region;
            stats.hitCount = hits;
            stats.missCount = misses;
            stats.evictionCount = evictions;
            stats.totalSizeBytes = totalSize;
            stats.usedSizeBytes = usedSize;
            m_regions.push_back(stats);

            CacheTimeSeries ts;
            ts.seriesName = region;
            ts.AddPoint(stats.HitRate(), hits + misses, usedSize);
            m_timeSeries.push_back(ts);
        }
        else {
            it->hitCount += hits;
            it->missCount += misses;
            it->evictionCount += evictions;
            it->totalSizeBytes = totalSize;
            it->usedSizeBytes = usedSize;

            auto tsIt = FindTimeSeries(region);
            if (tsIt != m_timeSeries.end()) {
                tsIt->AddPoint(it->HitRate(), it->hitCount + it->missCount, usedSize);
            }
        }
    }

    std::vector<CacheRegionStats> GetAllRegions() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_regions;
    }

    CacheTimeSeries GetHitRateOverTime(const std::string& region) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindTimeSeriesConst(region);
        return it != m_timeSeries.end() ? *it : CacheTimeSeries{};
    }

    std::vector<std::pair<std::string, uint64_t>> GetEvictionStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::pair<std::string, uint64_t>> result;
        for (const auto& r : m_regions) {
            result.emplace_back(r.regionName, r.evictionCount);
        }
        std::sort(result.begin(), result.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        return result;
    }

    double GetOverallHitRate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t totalHits = 0, totalMisses = 0;
        for (const auto& r : m_regions) {
            totalHits += r.hitCount;
            totalMisses += r.missCount;
        }
        uint64_t total = totalHits + totalMisses;
        return total > 0 ? static_cast<double>(totalHits) / total : 0.0;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_regions.clear();
        m_timeSeries.clear();
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& r : m_regions) {
            if (r.regionName.empty()) return false;
            if (r.usedSizeBytes > r.totalSizeBytes && r.totalSizeBytes > 0) return false;
            double hr = r.HitRate();
            if (hr < 0.0 || hr > 1.0) return false;
        }
        return m_regions.size() == m_timeSeries.size();
    }

private:
    CacheAnalyticsDashboard() = default;
    ~CacheAnalyticsDashboard() = default;
    CacheAnalyticsDashboard(const CacheAnalyticsDashboard&) = delete;
    CacheAnalyticsDashboard& operator=(const CacheAnalyticsDashboard&) = delete;

    std::vector<CacheRegionStats>::iterator FindRegion(const std::string& name) {
        return std::find_if(m_regions.begin(), m_regions.end(),
            [&name](const CacheRegionStats& r) { return r.regionName == name; });
    }

    std::vector<CacheTimeSeries>::iterator FindTimeSeries(const std::string& name) {
        return std::find_if(m_timeSeries.begin(), m_timeSeries.end(),
            [&name](const CacheTimeSeries& ts) { return ts.seriesName == name; });
    }

    std::vector<CacheTimeSeries>::const_iterator FindTimeSeriesConst(const std::string& name) const {
        return std::find_if(m_timeSeries.begin(), m_timeSeries.end(),
            [&name](const CacheTimeSeries& ts) { return ts.seriesName == name; });
    }

    mutable std::mutex m_mutex;
    std::vector<CacheRegionStats> m_regions;
    std::vector<CacheTimeSeries> m_timeSeries;
};

}
} // namespace ExplorerLens::Engine
