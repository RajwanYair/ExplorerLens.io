// StreamingDecodeEngine.h — Progressive Streaming Decode Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Enables progressive decoding: starts rendering a thumbnail from partial
// data while the rest of the file is still being read. Critical for large
// files on network shares and slow storage.
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class StreamState : uint8_t {
    NotStarted,
    HeaderParsed,
    PartialDecode,
    FullDecode,
    Error,
    Cancelled,
    COUNT
};

struct StreamProgress
{
    StreamState state = StreamState::NotStarted;
    float percent = 0.0f;
    uint64_t bytesRead = 0;
    uint64_t totalBytes = 0;
    uint32_t rowsDecoded = 0;
    uint32_t totalRows = 0;
    bool previewReady = false;
};

using StreamProgressCallback = std::function<void(const StreamProgress&)>;

class StreamDecodeEngine
{
  public:
    void SetCallback(StreamProgressCallback cb)
    {
        m_callback = std::move(cb);
    }

    void Begin(uint64_t totalBytes)
    {
        m_progress = {};
        m_progress.totalBytes = totalBytes;
        m_progress.state = StreamState::HeaderParsed;
        Notify();
    }

    void FeedData(const uint8_t* data, uint32_t size)
    {
        if (!data || size == 0)
            return;
        m_progress.bytesRead += size;
        if (m_progress.totalBytes > 0) {
            m_progress.percent =
                static_cast<float>(m_progress.bytesRead) / static_cast<float>(m_progress.totalBytes) * 100.0f;
        }
        m_progress.state = StreamState::PartialDecode;
        m_progress.rowsDecoded += size / 1024;  // Approximate
        if (m_progress.percent >= 10.0f)
            m_progress.previewReady = true;
        Notify();
    }

    void Complete()
    {
        m_progress.state = StreamState::FullDecode;
        m_progress.percent = 100.0f;
        m_progress.previewReady = true;
        Notify();
    }

    void Cancel()
    {
        m_progress.state = StreamState::Cancelled;
        Notify();
    }

    const StreamProgress& Progress() const
    {
        return m_progress;
    }
    bool IsPreviewReady() const
    {
        return m_progress.previewReady;
    }
    bool IsComplete() const
    {
        return m_progress.state == StreamState::FullDecode;
    }

    static const wchar_t* StateName(StreamState s)
    {
        switch (s) {
            case StreamState::NotStarted:
                return L"NotStarted";
            case StreamState::HeaderParsed:
                return L"HeaderParsed";
            case StreamState::PartialDecode:
                return L"PartialDecode";
            case StreamState::FullDecode:
                return L"FullDecode";
            case StreamState::Error:
                return L"Error";
            case StreamState::Cancelled:
                return L"Cancelled";
            default:
                return L"Unknown";
        }
    }
    static size_t StateCount()
    {
        return static_cast<size_t>(StreamState::COUNT);
    }

  private:
    void Notify()
    {
        if (m_callback)
            m_callback(m_progress);
    }
    StreamProgress m_progress;
    StreamProgressCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
