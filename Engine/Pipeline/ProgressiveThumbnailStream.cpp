// ProgressiveThumbnailStream.cpp — Server-Sent Events Progressive Thumbnail Feeder
// Copyright (c) 2026 ExplorerLens Project
//
#include "ProgressiveThumbnailStream.h"

namespace ExplorerLens { namespace Engine {

void ProgressiveThumbnailStream::Emit(const SSEFrame& frame)
{
    // Once the stream is complete, subsequent emits are silently discarded
    if (m_complete)
        return;

    m_frames.push_back(frame);
    m_lastEvent = frame.eventType;

    if (frame.eventType == StreamEventType::STREAM_COMPLETE ||
        frame.eventType == StreamEventType::STREAM_ERROR)
    {
        m_complete = true;
    }
}

bool            ProgressiveThumbnailStream::IsComplete()    const { return m_complete; }
size_t          ProgressiveThumbnailStream::EmittedCount()  const { return m_frames.size(); }
StreamEventType ProgressiveThumbnailStream::LastEventType() const { return m_lastEvent; }

}} // namespace ExplorerLens::Engine
