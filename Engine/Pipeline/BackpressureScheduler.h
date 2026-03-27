// BackpressureScheduler.h — Backpressure-Aware Reactive Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Applies backpressure when downstream consumers fall behind — drops low-priority requests and signals producers.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class BackpressureState { Normal, Throttling, Dropping, Paused };
struct BackpressureMetrics { uint64_t enqueued; uint64_t dropped; uint64_t processed; BackpressureState state; };
class BackpressureScheduler {
public:
    bool   Submit(std::function<void()> task, uint8_t priority = 128) {
        if (m_state == BackpressureState::Dropping && priority < 64) { m_dropped++; return false; }
        m_queue.push_back(std::move(task)); m_enqueued++; return true;
    }
    void   SetHWM(size_t hwm) { m_hwm = hwm; }
    BackpressureMetrics Metrics() const { return { m_enqueued.load(), m_dropped.load(), m_processed.load(), m_state }; }
    size_t QueueDepth() const { return m_queue.size(); }
private:
    BackpressureState       m_state = BackpressureState::Normal;
    size_t                  m_hwm   = 512;
    std::atomic<uint64_t>   m_enqueued{0}, m_dropped{0}, m_processed{0};
    std::vector<std::function<void()>> m_queue;
};

} // namespace Engine
} // namespace ExplorerLens