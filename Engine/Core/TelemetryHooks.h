//==============================================================================
// ExplorerLens Engine - Telemetry Hooks
// Copyright (c) 2026 - ExplorerLens Project
// Task B7: Lightweight telemetry system
//==============================================================================

#pragma once

#include <string>
#include <chrono>
#include <atomic>
#include <functional>

namespace ExplorerLens {
namespace Engine {

    /// <summary>
    /// Telemetry event types
    /// </summary>
    enum class TelemetryEvent {
        DecoderSuccess,
        DecoderFailure,
        CacheHit,
        CacheMiss,
        GPUUsed,
        GPUFallback,
        SlowPath,
        FastPath
    };

    /// <summary>
    /// Telemetry data point
    /// </summary>
    struct TelemetryData {
        TelemetryEvent event;
        std::wstring category;
        std::wstring detail;
        uint64_t value;
        std::chrono::steady_clock::time_point timestamp;
    };

    /// <summary>
    /// Lightweight telemetry collection system
    /// </summary>
    class TelemetryCollector {
    public:
        static TelemetryCollector& GetInstance() {
            static TelemetryCollector instance;
            return instance;
        }

        using TelemetryCallback = std::function<void(const TelemetryData&)>;

        void SetCallback(TelemetryCallback callback) {
            m_callback = callback;
        }

        void Record(TelemetryEvent event, const wchar_t* category = nullptr, 
                   const wchar_t* detail = nullptr, uint64_t value = 0) {
            TelemetryData data;
            data.event = event;
            data.category = category ? category : L"";
            data.detail = detail ? detail : L"";
            data.value = value;
            data.timestamp = std::chrono::steady_clock::now();

            if (m_callback) {
                m_callback(data);
            }

            // Update counters
            m_totalEvents++;
            UpdateEventCounter(event);
        }

        uint64_t GetTotalEvents() const { return m_totalEvents.load(); }
        uint64_t GetEventCount(TelemetryEvent event) const {
            size_t idx = static_cast<size_t>(event);
            return idx < m_eventCounts.size() ? m_eventCounts[idx].load() : 0;
        }

    private:
        TelemetryCollector() : m_eventCounts(8) {}  // 8 event types
        ~TelemetryCollector() = default;
        TelemetryCollector(const TelemetryCollector&) = delete;
        TelemetryCollector& operator=(const TelemetryCollector&) = delete;

        void UpdateEventCounter(TelemetryEvent event) {
            size_t idx = static_cast<size_t>(event);
            if (idx < m_eventCounts.size()) {
                m_eventCounts[idx]++;
            }
        }

        TelemetryCallback m_callback;
        std::atomic<uint64_t> m_totalEvents{0};
        std::vector<std::atomic<uint64_t>> m_eventCounts;
    };

    /// <summary>
    /// RAII telemetry scope for measuring duration
    /// </summary>
    class TelemetryScope {
    public:
        TelemetryScope(TelemetryEvent event, const wchar_t* category)
            : m_event(event), m_category(category),
              m_start(std::chrono::high_resolution_clock::now()) {}

        ~TelemetryScope() {
            auto end = std::chrono::high_resolution_clock::now();
            auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                end - m_start).count();
            
            TelemetryCollector::GetInstance().Record(
                m_event, m_category.c_str(), L"duration", 
                static_cast<uint64_t>(durationMs));
        }

    private:
        TelemetryEvent m_event;
        std::wstring m_category;
        std::chrono::high_resolution_clock::time_point m_start;
    };

    #define DT_TELEMETRY(event, category) \
        ExplorerLens::Engine::TelemetryScope __dtTelem(event, category)

} // namespace Engine
} // namespace ExplorerLens

