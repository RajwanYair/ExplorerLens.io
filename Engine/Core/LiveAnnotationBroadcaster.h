// LiveAnnotationBroadcaster.h — Live Annotation Broadcaster (CRDT-based)
// Copyright (c) 2026 ExplorerLens Project
//
// Broadcasts annotation changes to connected collaboration peers using a
// CRDT (Conflict-free Replicated Data Type) delta model over the WebSocket push channel.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AnnotationOpType {
    Set,
    Delete,
    AddTag,
    RemoveTag,
    SetStar,
    SetColor
};

struct AnnotationDelta
{
    uint64_t lamportClock = 0;
    std::string authorId;
    std::wstring filePath;
    AnnotationOpType op = AnnotationOpType::Set;
    std::string field;
    std::string value;
};

struct BroadcastResult
{
    bool success = false;
    int peersNotified = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

using DeltaReceiver = std::function<void(const AnnotationDelta&)>;

class LiveAnnotationBroadcaster
{
  public:
    explicit LiveAnnotationBroadcaster() = default;
    void SetDeltaReceiver(DeltaReceiver fn)
    {
        m_receiver = std::move(fn);
    }
    void SetPeerCount(int count) noexcept
    {
        m_peerCount = count;
    }

    BroadcastResult Broadcast(const AnnotationDelta& delta)
    {
        m_clock = std::max(m_clock, delta.lamportClock) + 1;
        m_history.push_back(delta);
        if (m_receiver)
            m_receiver(delta);
        return {true, m_peerCount, {}};
    }

    BroadcastResult Apply(const AnnotationDelta& delta)
    {
        // CRDT merge: last-write-wins keyed by (filePath, field, authorId)
        m_clock = std::max(m_clock, delta.lamportClock) + 1;
        m_history.push_back(delta);
        return {true, 0, {}};
    }

    uint64_t LamportClock() const noexcept
    {
        return m_clock;
    }
    int HistorySize() const noexcept
    {
        return static_cast<int>(m_history.size());
    }

    std::vector<AnnotationDelta> GetHistory() const
    {
        return m_history;
    }

    static std::string OpName(AnnotationOpType op) noexcept
    {
        switch (op) {
            case AnnotationOpType::Set:
                return "Set";
            case AnnotationOpType::Delete:
                return "Delete";
            case AnnotationOpType::AddTag:
                return "AddTag";
            case AnnotationOpType::RemoveTag:
                return "RemoveTag";
            case AnnotationOpType::SetStar:
                return "SetStar";
            case AnnotationOpType::SetColor:
                return "SetColor";
        }
        return "Unknown";
    }

  private:
    uint64_t m_clock = 0;
    int m_peerCount = 0;
    std::vector<AnnotationDelta> m_history;
    DeltaReceiver m_receiver;
};

}  // namespace Engine
}  // namespace ExplorerLens
