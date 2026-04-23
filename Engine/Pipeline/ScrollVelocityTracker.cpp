// ScrollVelocityTracker.cpp — Explorer Scroll Velocity Tracker
// Copyright (c) 2026 ExplorerLens Project
//
#include "Pipeline/ScrollVelocityTracker.h"
#include <cmath>
#include <algorithm>
#include <chrono>

namespace ExplorerLens { namespace Engine {

void ScrollVelocityTracker::AddSample(const ScrollSample& sample) noexcept
{
    m_samples[m_head % k_maxSamples] = sample;
    ++m_head;
    if (m_sampleCount < k_maxSamples) ++m_sampleCount;

    // Compute instantaneous velocity from the two most recent samples.
    if (m_sampleCount < 2) return;
    const ScrollSample& prev = m_samples[(m_head - 2) % k_maxSamples];
    const ScrollSample& curr = m_samples[(m_head - 1) % k_maxSamples];
    const int64_t dtUs = curr.timestampUs - prev.timestampUs;
    if (dtUs <= 0) return;
    const float instVelocity = std::abs(static_cast<float>(curr.deltaItems))
                               / (static_cast<float>(dtUs) / 1'000'000.0f);

    // EMA smoothing (alpha = 0.3).
    constexpr float alpha = 0.3f;
    m_emaVelocity = alpha * instVelocity + (1.0f - alpha) * m_emaVelocity;
}

ScrollVelocityStats ScrollVelocityTracker::GetStats() const noexcept
{
    ScrollVelocityStats s{};
    s.itemsPerSecond = m_emaVelocity;
    s.predictedItems = m_emaVelocity * 0.1f;  // 100 ms look-ahead
    s.isFastScroll   = (m_emaVelocity > m_fastScrollThreshold);

    if (m_sampleCount > 0) {
        const ScrollSample& last = m_samples[(m_head - 1) % k_maxSamples];
        s.direction = (last.deltaItems > 0) ? +1 : (last.deltaItems < 0) ? -1 : 0;
    }
    return s;
}

void ScrollVelocityTracker::SetTriggerCallback(TriggerCallback cb) noexcept
{
    m_callback = std::move(cb);
}

void ScrollVelocityTracker::Evaluate(int32_t currentTopIndex) noexcept
{
    if (!m_callback || m_emaVelocity < m_fastScrollThreshold) return;

    const auto stats = GetStats();
    const int32_t lookaheadCount = static_cast<int32_t>(stats.predictedItems) + 4;
    if (lookaheadCount <= 0) return;

    if (stats.direction >= 0) {
        m_callback(currentTopIndex + 1, static_cast<uint32_t>(lookaheadCount), +1);
    } else {
        const int32_t startIdx = std::max(0, currentTopIndex - lookaheadCount);
        m_callback(startIdx, static_cast<uint32_t>(lookaheadCount), -1);
    }
}

void ScrollVelocityTracker::Reset() noexcept
{
    m_sampleCount = 0;
    m_head        = 0;
    m_emaVelocity = 0.0f;
}

void ScrollVelocityTracker::SetFastScrollThreshold(float threshold) noexcept
{
    m_fastScrollThreshold = std::max(1.0f, threshold);
}

}} // namespace ExplorerLens::Engine
