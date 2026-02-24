#pragma once

#include <functional>
#include <future>
#include <string>
#include <cstdint>

namespace ExplorerLens::Engine::Scheduler {

    enum class TaskPriority {
        Background, // Prefetching
        Normal,     // Standard Explorer view
        High,       // Visible item / On-demand
        Critical    // UI blocking
    };

    struct TaskContext {
        uint64_t taskId;
        uint64_t correlationId;
        TaskPriority priority;
        bool isCancelled;
    };

    using TaskFunction = std::function<void(const TaskContext&)>;

    class ITaskScheduler {
    public:
        virtual ~ITaskScheduler() = default;

        // Schedule a unit of work (e.g. generate one thumbnail)
        // Returns a handle to cancel or await the task.
        virtual uint64_t Schedule(TaskFunction work, TaskPriority priority, uint64_t correlationId) = 0;

        // Attempt to cancel a pending or running task
        virtual void Cancel(uint64_t taskId) = 0;

        // Tune concurrency (e.g. set max threads = logical cores - 1)
        virtual void SetConcurrencyLimit(uint32_t limit) = 0;

        // Metrics
        virtual uint32_t GetQueueDepth() const = 0;
        virtual uint32_t GetActiveThreadCount() const = 0;
    };

}

