// LocalDataAggregator.h — Local Data Aggregator
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates telemetry locally with Gaussian noise injection before cloud upload.
//
#pragma once
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LDARecord
{
    std::string metricName;
    double value = 0.0;
    uint64_t epochMs = 0;
};

struct LDAAggregateResult
{
    std::string metricName;
    double noisyMean = 0.0;
    uint32_t sampleCount = 0;
    bool readyForUpload = false;
};

class LocalDataAggregator
{
  public:
    LocalDataAggregator() : m_noiseScale(0.1), m_rng(12345) {}

    void AddRecord(const LDARecord& record)
    {
        m_buckets[record.metricName].push_back(record.value);
    }
    std::vector<LDAAggregateResult> Flush()
    {
        std::vector<LDAAggregateResult> results;
        std::normal_distribution<double> noise(0.0, m_noiseScale);
        for (auto& [name, vals] : m_buckets) {
            if (vals.empty())
                continue;
            double sum = 0.0;
            for (double v : vals)
                sum += v;
            LDAAggregateResult r;
            r.metricName = name;
            r.noisyMean = sum / static_cast<double>(vals.size()) + noise(m_rng);
            r.sampleCount = static_cast<uint32_t>(vals.size());
            r.readyForUpload = true;
            results.push_back(r);
        }
        m_buckets.clear();
        return results;
    }
    uint32_t PendingCount() const
    {
        uint32_t total = 0;
        for (auto& [k, v] : m_buckets)
            total += static_cast<uint32_t>(v.size());
        return total;
    }
    void SetNoiseScale(double scale)
    {
        m_noiseScale = scale;
    }

  private:
    double m_noiseScale;
    std::mt19937_64 m_rng;
    std::unordered_map<std::string, std::vector<double>> m_buckets;
};

}  // namespace Engine
}  // namespace ExplorerLens
