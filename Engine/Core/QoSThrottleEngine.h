// QoSThrottleEngine.h — Quality-of-Service Throttle Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces QoS throttling policies on the thumbnail pipeline, ensuring foreground
// interactive requests are never starved by background batch decode operations.
//
#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <mutex>
#include <queue>

namespace ExplorerLens {
namespace Engine {

enum class QoSClass    { Interactive, BatchDecode, Background, Idle };
enum class ThrottleMode { None, SoftThrottle, HardThrottle, Suspend };

struct QoSPolicy {
    int  interactiveSlots = 8;
    int  batchDecodeSlots = 4;
    int  backgroundSlots  = 2;
    bool suspendBackground = false;
};

struct QoSRequest {
    uint64_t     id       = 0;
    QoSClass     qosClass = QoSClass::BatchDecode;
    int          estimatedMs = 17;
};

struct QoSThrottleDecision {
    ThrottleMode mode         = ThrottleMode::None;
    QoSClass     qosClass     = QoSClass::BatchDecode;
    int          maxConcurrent = 4;
    int          delayMs       = 0;
    bool IsAllowed() const noexcept { return mode != ThrottleMode::Suspend; }
};

class QoSThrottleEngine {
public:
    explicit QoSThrottleEngine(QoSPolicy policy = {})
        : m_policy(std::move(policy)) {}

    QoSThrottleDecision Evaluate(const QoSRequest& req) {
        int active = m_activeInteractive.load();
        QoSThrottleDecision d;
        d.qosClass = req.qosClass;
        switch (req.qosClass) {
        case QoSClass::Interactive:
            d.mode = ThrottleMode::None;
            d.maxConcurrent = m_policy.interactiveSlots;
            m_activeInteractive++;
            break;
        case QoSClass::BatchDecode:
            d.mode = active > m_policy.interactiveSlots / 2
                ? ThrottleMode::SoftThrottle : ThrottleMode::None;
            d.maxConcurrent = m_policy.batchDecodeSlots;
            d.delayMs = (d.mode == ThrottleMode::SoftThrottle) ? 5 : 0;
            break;
        case QoSClass::Background:
            d.mode = m_policy.suspendBackground ? ThrottleMode::Suspend : ThrottleMode::HardThrottle;
            d.maxConcurrent = m_policy.backgroundSlots;
            d.delayMs = 50;
            break;
        case QoSClass::Idle:
            d.mode = ThrottleMode::HardThrottle;
            d.maxConcurrent = 1;
            d.delayMs = 200;
            break;
        }
        return d;
    }

    void NotifyComplete(QoSClass qosClass) noexcept {
        if (qosClass == QoSClass::Interactive && m_activeInteractive > 0)
            m_activeInteractive--;
    }

    const QoSPolicy& Policy() const noexcept { return m_policy; }

    static std::string ClassName(QoSClass c) noexcept {
        switch (c) {
        case QoSClass::Interactive: return "Interactive";
        case QoSClass::BatchDecode: return "BatchDecode";
        case QoSClass::Background:  return "Background";
        case QoSClass::Idle:        return "Idle";
        }
        return "Unknown";
    }

private:
    QoSPolicy          m_policy;
    std::atomic<int>   m_activeInteractive{0};
};

} // namespace Engine
} // namespace ExplorerLens
