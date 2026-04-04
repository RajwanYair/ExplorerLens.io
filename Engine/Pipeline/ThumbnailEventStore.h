// ThumbnailEventStore.h — Append-Only Thumbnail Event Store
// Copyright (c) 2026 ExplorerLens Project
//
// Event-sourced append-only log for thumbnail generation events — enables replay, audit, and time-travel debugging.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ThumbEventType {
    Requested,
    Started,
    Completed,
    Failed,
    Evicted,
    Invalidated
};
struct ThumbnailEvent
{
    uint64_t id;
    std::wstring filePath;
    ThumbEventType type;
    uint64_t timestamp = 0;
    std::string metadata;
};
class ThumbnailEventStore
{
  public:
    uint64_t Append(ThumbnailEvent ev)
    {
        ev.id = ++m_seq;
        m_log.push_back(std::move(ev));
        return m_seq;
    }
    size_t EventCount() const
    {
        return m_log.size();
    }
    std::vector<ThumbnailEvent> Replay(uint64_t fromSeq = 0) const
    {
        std::vector<ThumbnailEvent> out;
        for (auto& e : m_log)
            if (e.id >= fromSeq)
                out.push_back(e);
        return out;
    }

  private:
    uint64_t m_seq = 0;
    std::vector<ThumbnailEvent> m_log;
};

}  // namespace Engine
}  // namespace ExplorerLens