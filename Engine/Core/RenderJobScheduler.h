// RenderJobScheduler.h — Render Job Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Priority-based scheduling of distributed thumbnail render jobs with dependency tracking.
//
#pragma once
#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RJSPriority {
    Critical = 0,
    High,
    Normal,
    Low,
    Background
};
enum class RJSJobState {
    Pending,
    Running,
    Complete,
    Failed
};

struct RJSJob
{
    uint32_t id = 0;
    RJSPriority priority = RJSPriority::Normal;
    RJSJobState state = RJSJobState::Pending;
    std::string description;
    uint64_t enqueuedMs = 0;
};

class RenderJobScheduler
{
  public:
    uint32_t Enqueue(const std::string& description, RJSPriority priority = RJSPriority::Normal)
    {
        RJSJob j;
        j.id = ++m_counter;
        j.priority = priority;
        j.description = description;
        j.state = RJSJobState::Pending;
        m_pending.push({static_cast<int>(priority), j});
        return j.id;
    }

    RJSJob Dequeue()
    {
        if (m_pending.empty())
            return {};
        auto [pri, job] = m_pending.top();
        m_pending.pop();
        job.state = RJSJobState::Running;
        m_running[job.id] = job;
        return job;
    }

    bool Complete(uint32_t jobId)
    {
        auto it = m_running.find(jobId);
        if (it == m_running.end())
            return false;
        it->second.state = RJSJobState::Complete;
        m_completed.push_back(it->second);
        m_running.erase(it);
        return true;
    }

    bool Fail(uint32_t jobId)
    {
        auto it = m_running.find(jobId);
        if (it == m_running.end())
            return false;
        it->second.state = RJSJobState::Failed;
        m_completed.push_back(it->second);
        m_running.erase(it);
        return true;
    }

    uint32_t PendingCount() const
    {
        return static_cast<uint32_t>(m_pending.size());
    }
    uint32_t RunningCount() const
    {
        return static_cast<uint32_t>(m_running.size());
    }
    uint32_t CompletedCount() const
    {
        return static_cast<uint32_t>(m_completed.size());
    }

  private:
    using QPair = std::pair<int, RJSJob>;
    struct Cmp
    {
        bool operator()(const QPair& a, const QPair& b) const
        {
            return a.first > b.first;
        }
    };
    std::priority_queue<QPair, std::vector<QPair>, Cmp> m_pending;
    std::unordered_map<uint32_t, RJSJob> m_running;
    std::vector<RJSJob> m_completed;
    uint32_t m_counter = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
