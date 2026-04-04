// DecoderPriorityScheduler.h — Decoder task priority scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules decoder tasks by priority using FIFO or weighted policies.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecoderTaskPriority : uint8_t {
    Urgent = 0,
    Normal = 1,
    Background = 2,
    Deferred = 3
};

inline const char* DecoderTaskPriorityName(DecoderTaskPriority p) noexcept
{
    switch (p) {
        case DecoderTaskPriority::Urgent:
            return "Urgent";
        case DecoderTaskPriority::Normal:
            return "Normal";
        case DecoderTaskPriority::Background:
            return "Background";
        case DecoderTaskPriority::Deferred:
            return "Deferred";
        default:
            return "Unknown";
    }
}

enum class SchedulerPolicy : uint8_t {
    FIFO = 0,
    Weighted = 1,
    StrictPriority = 2
};

inline const char* SchedulerPolicyName(SchedulerPolicy p) noexcept
{
    switch (p) {
        case SchedulerPolicy::FIFO:
            return "FIFO";
        case SchedulerPolicy::Weighted:
            return "Weighted";
        case SchedulerPolicy::StrictPriority:
            return "StrictPriority";
        default:
            return "Unknown";
    }
}

struct DecoderTask
{
    DecoderTaskPriority priority = DecoderTaskPriority::Normal;
    std::string decoderName;
    uint32_t id = 0;
};

class DecoderPriorityScheduler
{
  public:
    uint32_t Submit(DecoderTask task)
    {
        task.id = ++m_nextId;
        m_queue.push_back(task);
        std::stable_sort(m_queue.begin(), m_queue.end(), [](const DecoderTask& a, const DecoderTask& b) {
            return static_cast<int>(a.priority) < static_cast<int>(b.priority);
        });
        return task.id;
    }

    bool Cancel(uint32_t id)
    {
        auto it = std::find_if(m_queue.begin(), m_queue.end(), [id](const DecoderTask& t) { return t.id == id; });
        if (it == m_queue.end())
            return false;
        m_queue.erase(it);
        return true;
    }

    size_t GetQueueDepth() const noexcept
    {
        return m_queue.size();
    }

  private:
    std::vector<DecoderTask> m_queue;
    uint32_t m_nextId = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
