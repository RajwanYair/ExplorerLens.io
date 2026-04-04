// BackgroundIntelligenceService.h — Background Intelligence Service
// Copyright (c) 2026 ExplorerLens Project
//
// Implements a BITS-inspired background decode scheduling service that quietly
// pre-generates thumbnails during system idle periods without impacting user activity.
//
#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class BISState {
    Idle,
    Running,
    Suspended,
    Paused
};
enum class BISJobStatus {
    Queued,
    Active,
    Complete,
    Failed
};

struct BISJob
{
    uint64_t id = 0;
    std::wstring filePath;
    int priority = 50;
    BISJobStatus status = BISJobStatus::Queued;
};

struct BISMetrics
{
    std::atomic<int> queued{0};
    std::atomic<int> completed{0};
    std::atomic<int> failed{0};
};

using BISWorkerFn = std::function<bool(const BISJob&)>;

class BackgroundIntelligenceService
{
  public:
    explicit BackgroundIntelligenceService() = default;
    void SetWorker(BISWorkerFn fn)
    {
        m_worker = std::move(fn);
    }
    void SetIdleThresholdCPU(float pct) noexcept
    {
        m_idleCPUThreshold = pct;
    }

    uint64_t Enqueue(std::wstring filePath, int priority = 50)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        BISJob job{++m_idCounter, std::move(filePath), priority, BISJobStatus::Queued};
        m_queue.push(job);
        m_metrics.queued++;
        return job.id;
    }

    int DrainNext(int maxJobs = 4, float currentCPUPct = 0.0f)
    {
        if (currentCPUPct > m_idleCPUThreshold)
            return 0;
        int done = 0;
        for (int i = 0; i < maxJobs; ++i) {
            BISJob job;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_queue.empty())
                    break;
                job = m_queue.front();
                m_queue.pop();
            }
            bool ok = m_worker ? m_worker(job) : false;
            if (ok)
                m_metrics.completed++;
            else
                m_metrics.failed++;
            done++;
        }
        return done;
    }

    BISState State() const noexcept
    {
        return m_state;
    }
    void SetState(BISState s) noexcept
    {
        m_state = s;
    }
    int QueueDepth()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<int>(m_queue.size());
    }
    const BISMetrics& Metrics() const noexcept
    {
        return m_metrics;
    }

  private:
    BISState m_state = BISState::Idle;
    float m_idleCPUThreshold = 30.0f;
    BISWorkerFn m_worker;
    BISMetrics m_metrics;
    std::queue<BISJob> m_queue;
    std::mutex m_mutex;
    uint64_t m_idCounter = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
