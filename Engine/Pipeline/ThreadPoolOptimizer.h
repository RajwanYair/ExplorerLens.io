#pragma once
// ============================================================================
// ThreadPoolOptimizer.h — Work-Stealing Thread Pool (Sprint 550)
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE:
//   Work-stealing thread pool with per-thread local deques for decode
//   workload balancing. Each worker owns a double-ended work queue and
//   executes tasks from its front. When a worker's queue is empty, it steals
//   from the back of another randomly chosen worker's deque, minimizing
//   contention while maximizing throughput.
//
// CLASSES:
//   - WorkStealingStats: Aggregate statistics (tasks completed, steals
//     performed, average queue depth, maximum task latency).
//   - ThreadPoolOptimizer: Configurable work-stealing pool with Submit(),
//     WaitForIdle(), and Shutdown() lifecycle methods.
//
// INPUTS:
//   - std::function<void()> tasks submitted via Submit()
//   - Optional thread count (defaults to hardware_concurrency)
//
// OUTPUTS:
//   - Tasks execute on pool threads with automatic load balancing
//   - WorkStealingStats via GetStats()
//
// THREAD SAFETY:
//   Each worker's local deque is protected by its own mutex (minimal
//   contention). Stealing uses try_lock to avoid blocking. Global stats
//   use relaxed atomics. Submit() is safe to call from any thread.
// ============================================================================

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <functional>
#include <atomic>
#include <random>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include <memory>

namespace ExplorerLens {
namespace Engine {

/// Thread pool sizing policy
enum class PoolSizingPolicy : uint8_t {
    Fixed = 0,
    CoreCount,
    HyperThread,
    LoadAdaptive,
    PowerAware,
    COUNT
};

/// Power profile for thread count decisions
enum class PowerProfile : uint8_t {
    HighPerformance = 0,
    Balanced,
    PowerSaver,
    BatteryLow,
    ThermalThrottled,
    COUNT
};

struct ThreadPoolConfig {
    PoolSizingPolicy policy = PoolSizingPolicy::CoreCount;
    uint32_t minThreads = 1;
    uint32_t maxThreads = 16;
    uint32_t queueDepthLimit = 1024;
    bool     useIOCP = true;
    bool     setAffinity = false;
};

struct TPOptStats {
    uint32_t activeThreads = 0;
    uint32_t idleThreads = 0;
    uint32_t totalThreads = 0;
    uint64_t tasksCompleted = 0;
    uint64_t tasksQueued = 0;
    double   avgTaskTimeMs = 0.0;
    float    cpuUtilization = 0.0f;
};

/// Work-stealing pool aggregate statistics.
struct WorkStealingStats {
    uint64_t tasksCompleted = 0;
    uint64_t stealsPerformed = 0;
    double   avgQueueDepth = 0.0;
    double   maxLatencyMs = 0.0;
};

class ThreadPoolOptimizer {
public:
    // ====================================================================
    // Backward-compatible static API (v14)
    // ====================================================================

    static constexpr size_t PolicyCount() {
        return static_cast<size_t>(PoolSizingPolicy::COUNT);
    }

    static constexpr size_t PowerProfileCount() {
        return static_cast<size_t>(PowerProfile::COUNT);
    }

    static inline const wchar_t* PolicyName(PoolSizingPolicy p) {
        switch (p) {
        case PoolSizingPolicy::Fixed:        return L"Fixed";
        case PoolSizingPolicy::CoreCount:    return L"Per-Core";
        case PoolSizingPolicy::HyperThread:  return L"Hyper-Thread";
        case PoolSizingPolicy::LoadAdaptive: return L"Load Adaptive";
        case PoolSizingPolicy::PowerAware:   return L"Power Aware";
        default:                             return L"Unknown";
        }
    }

    static inline const wchar_t* PowerProfileName(PowerProfile p) {
        switch (p) {
        case PowerProfile::HighPerformance:  return L"High Performance";
        case PowerProfile::Balanced:         return L"Balanced";
        case PowerProfile::PowerSaver:       return L"Power Saver";
        case PowerProfile::BatteryLow:       return L"Battery Low";
        case PowerProfile::ThermalThrottled: return L"Thermal Throttled";
        default:                             return L"Unknown";
        }
    }

    /// Recommend thread count based on policy, logical core count, and power
    /// profile. Returns at least 1.
    static inline uint32_t RecommendThreads(
        PoolSizingPolicy policy, uint32_t logicalCores,
        PowerProfile power = PowerProfile::HighPerformance) {
        uint32_t base = 4;
        switch (policy) {
        case PoolSizingPolicy::Fixed:        base = 4; break;
        case PoolSizingPolicy::CoreCount:    base = logicalCores / 2; break;
        case PoolSizingPolicy::HyperThread:  base = logicalCores; break;
        case PoolSizingPolicy::LoadAdaptive: base = logicalCores / 2; break;
        case PoolSizingPolicy::PowerAware:   base = logicalCores / 2; break;
        default:                             base = 4; break;
        }
        switch (power) {
        case PowerProfile::PowerSaver:       base = (std::max)(1u, base / 2); break;
        case PowerProfile::BatteryLow:       base = (std::max)(1u, base / 4); break;
        case PowerProfile::ThermalThrottled: base = (std::max)(1u, base / 3); break;
        default: break;
        }
        return (std::max)(1u, base);
    }

    // ====================================================================
    // Sprint 550: Work-stealing thread pool
    // ====================================================================

    /// Construct a work-stealing pool. If threadCount is 0, defaults to
    /// hardware_concurrency (at least 1).
    explicit ThreadPoolOptimizer(uint32_t threadCount = 0)
        : m_stop(false)
        , m_tasksInFlight(0)
        , m_totalTasksCompleted(0)
        , m_totalSteals(0)
        , m_maxLatencyNs(0)
        , m_submitIndex(0) {
        if (threadCount == 0) {
            threadCount = (std::max)(1u, std::thread::hardware_concurrency());
        }
        m_workerCount = threadCount;
        m_queues.resize(threadCount);
        m_queueMutexes = std::make_unique<std::mutex[]>(threadCount);

        m_workers.reserve(threadCount);
        for (uint32_t i = 0; i < threadCount; ++i) {
            m_workers.emplace_back([this, i]() { WorkerLoop(i); });
        }
    }

    ~ThreadPoolOptimizer() {
        Shutdown();
    }

    ThreadPoolOptimizer(const ThreadPoolOptimizer&) = delete;
    ThreadPoolOptimizer& operator=(const ThreadPoolOptimizer&) = delete;

    /// Submit a task. Round-robins across worker local queues.
    inline void Submit(std::function<void()> task) {
        uint32_t idx = m_submitIndex.fetch_add(1, std::memory_order_relaxed)
            % m_workerCount;
        {
            std::lock_guard<std::mutex> lock(m_queueMutexes[idx]);
            m_queues[idx].push_back(std::move(task));
        }
        m_wakeCV.notify_one();
    }

    /// Block the calling thread until all queues are empty and no tasks
    /// are in-flight. Uses a condition variable to avoid busy-spinning.
    inline void WaitForIdle() {
        std::unique_lock<std::mutex> lock(m_idleMutex);
        m_idleCV.wait(lock, [this]() {
            if (m_tasksInFlight.load(std::memory_order_acquire) != 0)
                return false;
            for (uint32_t i = 0; i < m_workerCount; ++i) {
                std::lock_guard<std::mutex> qLock(m_queueMutexes[i]);
                if (!m_queues[i].empty()) return false;
            }
            return m_tasksInFlight.load(std::memory_order_acquire) == 0;
            });
    }

    /// Stop all workers and drain remaining queued tasks.
    inline void Shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_wakeMutex);
            m_stop.store(true, std::memory_order_release);
        }
        m_wakeCV.notify_all();
        for (auto& w : m_workers) {
            if (w.joinable()) w.join();
        }
        m_workers.clear();
    }

    /// Aggregate work-stealing statistics.
    inline WorkStealingStats GetStats() const {
        WorkStealingStats s;
        s.tasksCompleted = m_totalTasksCompleted.load(std::memory_order_acquire);
        s.stealsPerformed = m_totalSteals.load(std::memory_order_acquire);
        s.maxLatencyMs = static_cast<double>(
            m_maxLatencyNs.load(std::memory_order_acquire)) / 1.0e6;

        uint64_t totalDepth = 0;
        for (uint32_t i = 0; i < m_workerCount; ++i) {
            std::lock_guard<std::mutex> lock(m_queueMutexes[i]);
            totalDepth += m_queues[i].size();
        }
        s.avgQueueDepth = (m_workerCount > 0)
            ? static_cast<double>(totalDepth) / m_workerCount : 0.0;
        return s;
    }

    /// Number of worker threads.
    inline uint32_t WorkerCount() const { return m_workerCount; }

    /// Whether the pool is still accepting and processing work.
    inline bool IsRunning() const {
        return !m_stop.load(std::memory_order_acquire);
    }

private:
    /// Worker loop: try own queue (front) → steal from random other (back)
    /// → sleep briefly if nothing found.
    inline void WorkerLoop(uint32_t myIndex) {
        std::mt19937 rng(myIndex * 7919u + 42u);

        while (!m_stop.load(std::memory_order_acquire)) {
            std::function<void()> task;
            bool stolen = false;

            // 1. Try pop from front of own local queue
            {
                std::lock_guard<std::mutex> lock(m_queueMutexes[myIndex]);
                if (!m_queues[myIndex].empty()) {
                    task = std::move(m_queues[myIndex].front());
                    m_queues[myIndex].pop_front();
                    m_tasksInFlight.fetch_add(1, std::memory_order_relaxed);
                }
            }

            // 2. If local queue empty and multiple workers, steal from back
            //    of a randomly chosen other worker's deque (try_lock to avoid
            //    blocking the victim).
            if (!task && m_workerCount > 1) {
                std::uniform_int_distribution<uint32_t> dist(0, m_workerCount - 2);
                uint32_t victim = dist(rng);
                if (victim >= myIndex) ++victim; // skip self

                if (m_queueMutexes[victim].try_lock()) {
                    if (!m_queues[victim].empty()) {
                        task = std::move(m_queues[victim].back());
                        m_queues[victim].pop_back();
                        m_tasksInFlight.fetch_add(1, std::memory_order_relaxed);
                        stolen = true;
                    }
                    m_queueMutexes[victim].unlock();
                }
            }

            if (task) {
                if (stolen) {
                    m_totalSteals.fetch_add(1, std::memory_order_relaxed);
                }

                auto start = std::chrono::steady_clock::now();
                task();
                auto elapsed = std::chrono::steady_clock::now() - start;
                auto ns = static_cast<uint64_t>(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        elapsed).count());

                // Update max latency (relaxed CAS loop — benign races are OK)
                uint64_t prev = m_maxLatencyNs.load(std::memory_order_relaxed);
                while (ns > prev) {
                    if (m_maxLatencyNs.compare_exchange_weak(
                        prev, ns, std::memory_order_relaxed))
                        break;
                }

                m_totalTasksCompleted.fetch_add(1, std::memory_order_relaxed);
                m_tasksInFlight.fetch_sub(1, std::memory_order_relaxed);
                m_idleCV.notify_all();
            }
            else {
                // 3. No work found — sleep briefly to avoid spinning
                std::unique_lock<std::mutex> lock(m_wakeMutex);
                m_wakeCV.wait_for(lock, std::chrono::microseconds(100));
            }
        }
    }

    uint32_t                m_workerCount = 0;
    std::vector<std::thread> m_workers;

    // Per-worker local deques (double-ended: push to back, pop from front,
    // stolen from back by other workers).
    std::vector<std::deque<std::function<void()>>> m_queues;
    std::unique_ptr<std::mutex[]>                  m_queueMutexes;

    // Shutdown and coordination
    std::atomic<bool>     m_stop;
    std::atomic<uint32_t> m_tasksInFlight;
    std::mutex            m_wakeMutex;
    std::condition_variable m_wakeCV;
    std::mutex            m_idleMutex;
    std::condition_variable m_idleCV;

    // Stats (relaxed atomics — no ordering needed for counters)
    std::atomic<uint64_t> m_totalTasksCompleted;
    std::atomic<uint64_t> m_totalSteals;
    std::atomic<uint64_t> m_maxLatencyNs;
    std::atomic<uint32_t> m_submitIndex;
};

} // namespace Engine
} // namespace ExplorerLens
