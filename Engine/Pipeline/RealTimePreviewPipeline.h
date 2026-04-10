// RealTimePreviewPipeline.h — Real-Time Preview Update Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Debounced, back-pressure-aware subscriber pipeline that coalesces rapid
// file-change notifications and delivers fresh thumbnails to the IThumbnailProvider
// at a controlled rate.  Prevents Explorer from being flooded when a user is
// actively editing and saving a file every few seconds.
//
#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <deque>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

/// A pending thumbnail-update event from the pipeline.
struct PreviewUpdateEvent {
    std::wstring filePath;
    uint64_t     enqueuedAtMs = 0;
    uint32_t     coalesceCount= 1;  ///< How many raw events this represents
};

/// Subscriber callback invoked when the pipeline fires a coalesced event.
using PreviewSubscriber = std::function<void(const PreviewUpdateEvent&)>;

/// Real-time preview pipeline with configurable debounce and back-pressure.
class RealTimePreviewPipeline {
public:
    struct Config {
        uint32_t debounceMs      = 250;  ///< Hold-off window before firing subscriber
        uint32_t maxQueueDepth   = 128;  ///< Drop oldest event if queue exceeds this
        uint32_t maxCoalesceMs   = 2000; ///< Force fire even if events keep arriving
    };

    explicit RealTimePreviewPipeline(const Config& cfg = {});

    /// Register a subscriber.  Only one subscriber per pipeline.
    void Subscribe(PreviewSubscriber sub);

    /// Enqueue a file-change notification.  May coalesce with an existing pending event.
    void Notify(const std::wstring& filePath);

    /// Drain all events whose debounce window has expired.
    /// Returns number of events fired.  Call from a timer or background thread.
    uint32_t Drain(uint64_t currentMs);

    uint32_t PendingEventCount() const;
    uint64_t TotalEventsEnqueued()  const;
    uint64_t TotalEventsFired()     const;
    uint64_t TotalEventsDropped()   const;

    const Config& GetConfig() const;

private:
    Config           m_config;
    PreviewSubscriber m_subscriber;

    struct PendingEvent {
        std::wstring filePath;
        uint64_t     firstSeenMs    = 0;
        uint64_t     lastSeenMs     = 0;
        uint32_t     coalesceCount  = 1;
    };

    std::deque<PendingEvent>              m_queue;
    std::unordered_map<std::wstring, uint32_t> m_queueIndex;  ///< filePath → queue position

    uint64_t m_totalEnqueued = 0;
    uint64_t m_totalFired    = 0;
    uint64_t m_totalDropped  = 0;

    static uint64_t NowMs();
};

}} // namespace ExplorerLens::Engine
