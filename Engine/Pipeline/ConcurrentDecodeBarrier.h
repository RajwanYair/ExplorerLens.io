// ConcurrentDecodeBarrier.h — Barrier Synchronization for Batch Decode
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a barrier primitive to synchronize completion of batch
// thumbnail decode operations before proceeding to next pipeline phase.
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class BarrierState : uint8_t {
    Open,
    Waiting,
    Released,
    TimedOut,
    Cancelled
};

struct BarrierMetrics
{
    uint64_t totalBarriersCreated = 0;
    uint64_t totalWaitsCompleted = 0;
    uint64_t totalTimeouts = 0;
    double avgWaitTimeMs = 0.0;
    uint32_t maxParticipants = 0;
};

class ConcurrentDecodeBarrier
{
  public:
    explicit ConcurrentDecodeBarrier(uint32_t participantCount = 1) : m_targetCount(participantCount) {}

    void Arrive()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_arrivedCount++;
        if (m_arrivedCount >= m_targetCount) {
            m_state = BarrierState::Released;
            m_metrics.totalWaitsCompleted++;
            m_cv.notify_all();
        }
    }

    bool Wait(uint32_t timeoutMs = 5000)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_state == BarrierState::Released)
            return true;
        m_state = BarrierState::Waiting;
        bool result = m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                    [this] { return m_state == BarrierState::Released; });
        if (!result) {
            m_state = BarrierState::TimedOut;
            m_metrics.totalTimeouts++;
        }
        return result;
    }

    void Reset(uint32_t newParticipantCount = 0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_arrivedCount = 0;
        if (newParticipantCount > 0)
            m_targetCount = newParticipantCount;
        m_state = BarrierState::Open;
        m_metrics.totalBarriersCreated++;
    }

    void Cancel()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = BarrierState::Cancelled;
        m_cv.notify_all();
    }

    BarrierState GetState() const
    {
        return m_state;
    }
    uint32_t GetArrivedCount() const
    {
        return m_arrivedCount;
    }
    uint32_t GetTargetCount() const
    {
        return m_targetCount;
    }
    BarrierMetrics GetMetrics() const
    {
        return m_metrics;
    }

  private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    uint32_t m_targetCount;
    uint32_t m_arrivedCount = 0;
    BarrierState m_state = BarrierState::Open;
    BarrierMetrics m_metrics;
};

}  // namespace Engine
}  // namespace ExplorerLens
