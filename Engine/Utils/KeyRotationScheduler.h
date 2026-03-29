// KeyRotationScheduler.h — Key Rotation Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Time-based scheduler for cryptographic key rotation events. Integrates with
// KeyRotationOrchestrator to fire rotations on expiry with configurable jitter.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct RotationScheduleEntry {
    std::string keyId;
    int64_t     nextRotationMs = 0;
    uint32_t    intervalDays   = 90;
    uint32_t    jitterMs       = 300000; // ±5 min jitter
    bool        active         = true;
};

using RotationDueCallback = std::function<void(const std::string& keyId)>;

class KeyRotationScheduler {
public:
    KeyRotationScheduler() = default;

    bool Initialize(int64_t nowMs = 0) {
        m_baseTimeMs = nowMs;
        m_ready      = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    void Schedule(const std::string& keyId, uint32_t intervalDays,
                  int64_t firstRotationMs, uint32_t jitterMs = 300000) {
        RotationScheduleEntry e;
        e.keyId           = keyId;
        e.intervalDays    = intervalDays;
        e.nextRotationMs  = firstRotationMs;
        e.jitterMs        = jitterMs;
        e.active          = true;
        m_schedule.push_back(e);
    }

    bool Unschedule(const std::string& keyId) {
        for (auto& e : m_schedule) {
            if (e.keyId == keyId) { e.active = false; return true; }
        }
        return false;
    }

    void SetDueCallback(RotationDueCallback cb) { m_callback = std::move(cb); }

    uint32_t Tick(int64_t nowMs) {
        uint32_t fired = 0;
        for (auto& e : m_schedule) {
            if (!e.active) continue;
            if (nowMs >= e.nextRotationMs) {
                if (m_callback) m_callback(e.keyId);
                e.nextRotationMs = nowMs +
                    static_cast<int64_t>(e.intervalDays) * 86400000LL +
                    (e.jitterMs > 0 ? (nowMs % e.jitterMs) : 0);
                ++fired;
            }
        }
        return fired;
    }

    uint32_t GetScheduledCount() const {
        uint32_t n = 0;
        for (const auto& e : m_schedule) if (e.active) ++n;
        return n;
    }

    void Shutdown() { m_ready = false; }

private:
    bool                              m_ready      = false;
    int64_t                           m_baseTimeMs = 0;
    std::vector<RotationScheduleEntry> m_schedule;
    RotationDueCallback               m_callback;
};

}} // namespace ExplorerLens::Engine
