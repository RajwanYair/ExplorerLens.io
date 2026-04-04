// CooperativeTaskScheduler.h — Cooperative Micro-Task Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Yield-based cooperative scheduler for short-lived decode tasks — avoids preemption overhead on tight inner loops.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CoopTask
{
    std::function<bool()> step;  // returns true when done
    uint32_t priority = 0;
    uint64_t id = 0;
};
class CooperativeTaskScheduler
{
  public:
    void Submit(CoopTask task)
    {
        m_tasks.push_back(std::move(task));
    }
    size_t RunOnce()
    {
        size_t ran = 0;
        for (auto it = m_tasks.begin(); it != m_tasks.end();) {
            if (it->step()) {
                it = m_tasks.erase(it);
            } else {
                ++it;
            }
            ran++;
        }
        return ran;
    }
    size_t Pending() const
    {
        return m_tasks.size();
    }
    void DrainAll()
    {
        while (!m_tasks.empty())
            RunOnce();
    }

  private:
    std::vector<CoopTask> m_tasks;
};

}  // namespace Engine
}  // namespace ExplorerLens