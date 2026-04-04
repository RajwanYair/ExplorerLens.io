// SmartThumbnailPrioritizer.h — Intelligent Thumbnail Generation Ordering
// Copyright (c) 2026 ExplorerLens Project
//
// Prioritizes thumbnail generation based on viewport visibility, file
// recency, and predicted user interest using lightweight heuristics.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PriorityClass : uint8_t {
    Immediate = 0,  // Visible in viewport
    High = 1,       // Adjacent to viewport
    Normal = 2,     // Same folder, off-screen
    Low = 3,        // Prefetch candidate
    Background = 4  // Idle-time generation
};

struct PrioritizedRequest
{
    std::wstring filePath;
    uint32_t requestedSize = 256;
    PriorityClass priority = PriorityClass::Normal;
    float interestScore = 0.5f;
    uint64_t fileModTime = 0;
};

struct PrioritizerStats
{
    uint64_t totalRequests = 0;
    uint64_t immediateCount = 0;
    uint64_t reorderedCount = 0;
};

class SmartThumbnailPrioritizer
{
  public:
    void Enqueue(PrioritizedRequest req)
    {
        m_stats.totalRequests++;
        if (req.priority == PriorityClass::Immediate)
            m_stats.immediateCount++;
        m_queue.push_back(std::move(req));
    }

    void Prioritize()
    {
        std::stable_sort(m_queue.begin(), m_queue.end(), [](const PrioritizedRequest& a, const PrioritizedRequest& b) {
            if (a.priority != b.priority)
                return static_cast<uint8_t>(a.priority) < static_cast<uint8_t>(b.priority);
            return a.interestScore > b.interestScore;
        });
        m_stats.reorderedCount++;
    }

    PrioritizedRequest Dequeue()
    {
        if (m_queue.empty())
            return {};
        auto req = std::move(m_queue.front());
        m_queue.erase(m_queue.begin());
        return req;
    }

    bool Empty() const
    {
        return m_queue.empty();
    }
    size_t Size() const
    {
        return m_queue.size();
    }
    PrioritizerStats Stats() const
    {
        return m_stats;
    }

  private:
    std::vector<PrioritizedRequest> m_queue;
    PrioritizerStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
