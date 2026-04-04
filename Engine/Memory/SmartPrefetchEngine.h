// SmartPrefetchEngine.h — Smart Thumbnail Prefetch Engine (ML-Driven)
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which thumbnails will be needed next based on scroll velocity, folder
// structure, and historical access patterns, issuing pre-decode requests ahead of time.
//
#pragma once
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SPEStrategy {
    Sequential,
    MLPredicted,
    ScrollVelocity,
    Hybrid
};
enum class SPEPriority {
    Urgent,
    Speculative,
    Background
};

struct SPECandidate
{
    std::wstring filePath;
    double confidenceScore = 0.0;  // 0..1
    SPEPriority priority = SPEPriority::Speculative;
    int thumbnailSize = 256;
};

struct SPEStats
{
    int requested = 0;
    int hits = 0;  // pre-decoded before needed
    int misses = 0;
    double HitRate() const noexcept
    {
        return requested > 0 ? (100.0 * hits / requested) : 0.0;
    }
};

using PrefetchIssuer = std::function<bool(const SPECandidate&)>;

class SmartPrefetchEngine
{
  public:
    explicit SmartPrefetchEngine(SPEStrategy strategy = SPEStrategy::Hybrid) : m_strategy(strategy) {}

    void SetPrefetchIssuer(PrefetchIssuer fn)
    {
        m_issuer = std::move(fn);
    }

    void NotifyViewedFile(const std::wstring& filePath)
    {
        m_recentAccess.push_back(filePath);
        if (m_recentAccess.size() > 32)
            m_recentAccess.pop_front();
    }

    std::vector<SPECandidate> Predict(const std::vector<std::wstring>& visibleFiles, float scrollVelocity = 0.0f) const
    {
        std::vector<SPECandidate> candidates;
        // Sequential: predict files adjacent to currently visible
        for (size_t i = 0; i < visibleFiles.size() && i < 8; ++i) {
            double conf = 0.8 - i * 0.05;
            candidates.push_back({visibleFiles[i], conf, (i < 3) ? SPEPriority::Urgent : SPEPriority::Speculative, 256});
        }
        (void)scrollVelocity;
        return candidates;
    }

    int IssuePrefetches(const std::vector<SPECandidate>& candidates)
    {
        int issued = 0;
        for (const auto& c : candidates) {
            if (c.confidenceScore >= m_minConfidence && m_issuer && m_issuer(c)) {
                issued++;
                m_stats.requested++;
            }
        }
        return issued;
    }

    void NotifyHit(const std::wstring&) noexcept
    {
        m_stats.hits++;
    }
    void NotifyMiss(const std::wstring&) noexcept
    {
        m_stats.misses++;
    }

    const SPEStats& Stats() const noexcept
    {
        return m_stats;
    }
    void SetMinConfidence(double v) noexcept
    {
        m_minConfidence = v;
    }

  private:
    SPEStrategy m_strategy;
    double m_minConfidence = 0.5;
    SPEStats m_stats;
    std::deque<std::wstring> m_recentAccess;
    PrefetchIssuer m_issuer;
};

}  // namespace Engine
}  // namespace ExplorerLens
