// ProgressiveThumbnailStream.h — Server-Sent Events Progressive Thumbnail Feeder
// Copyright (c) 2026 ExplorerLens Project
//
// Streams progressive thumbnail frames (placeholder → low-res → full-res) as
// Server-Sent Events (SSE) to browser clients, enabling instant feedback for
// cloud-hosted files that are still hydrating.
//
#pragma once

#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class StreamEventType : uint32_t
{
    THUMBNAIL_PLACEHOLDER = 0,
    THUMBNAIL_LOW_RES     = 1,
    THUMBNAIL_READY       = 2,
    STREAM_COMPLETE       = 3,
    STREAM_ERROR          = 4
};

struct SSEFrame
{
    StreamEventType      eventType  = StreamEventType::THUMBNAIL_PLACEHOLDER;
    uint32_t             requestId  = 0u;
    uint32_t             frameIndex = 0u;
    std::vector<uint8_t> payload;
};

class ProgressiveThumbnailStream
{
public:
    ProgressiveThumbnailStream()  = default;
    ~ProgressiveThumbnailStream() = default;

    ProgressiveThumbnailStream(const ProgressiveThumbnailStream&)            = delete;
    ProgressiveThumbnailStream& operator=(const ProgressiveThumbnailStream&) = delete;
    ProgressiveThumbnailStream(ProgressiveThumbnailStream&&)                 = default;
    ProgressiveThumbnailStream& operator=(ProgressiveThumbnailStream&&)      = default;

    void Emit(const SSEFrame& frame);

    bool            IsComplete()     const;
    size_t          EmittedCount()   const;
    StreamEventType LastEventType()  const;

private:
    std::vector<SSEFrame> m_frames;
    bool                  m_complete  = false;
    StreamEventType       m_lastEvent = StreamEventType::THUMBNAIL_PLACEHOLDER;
};

}} // namespace ExplorerLens::Engine
