// SharedViewStateProtocol.h — Shared View State Protocol (Folder / Filter / Sort Sync)
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the protocol for synchronising Explorer view state (current folder,
// active filters, sort order, zoom level) across collaboration session participants.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ViewSortOrder {
    Name,
    Date,
    Size,
    Type,
    Stars,
    Custom
};
enum class ViewGroupBy {
    None,
    Type,
    Date,
    Stars
};
enum class ViewZoomLevel {
    Tiny = 16,
    Small = 32,
    Medium = 96,
    Large = 256,
    ExtraLarge = 512
};

struct SharedViewState
{
    std::wstring currentFolder;
    ViewSortOrder sortOrder = ViewSortOrder::Name;
    bool sortAscending = true;
    ViewGroupBy groupBy = ViewGroupBy::None;
    ViewZoomLevel zoomLevel = ViewZoomLevel::Medium;
    std::string filterQuery;
    std::string ownerUserId;
    uint64_t version = 0;
};

struct ViewStateSyncResult
{
    bool success = false;
    uint64_t version = 0;
    int peersUpdated = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

using ViewStateCallback = std::function<void(const SharedViewState&)>;

class SharedViewStateProtocol
{
  public:
    explicit SharedViewStateProtocol() = default;
    void SetViewStateCallback(ViewStateCallback cb)
    {
        m_callback = std::move(cb);
    }

    ViewStateSyncResult Publish(const SharedViewState& state)
    {
        m_current = state;
        m_current.version = ++m_version;
        if (m_callback)
            m_callback(m_current);
        return {true, m_version, m_peerCount, {}};
    }

    ViewStateSyncResult Apply(const SharedViewState& state)
    {
        if (state.version > m_current.version) {
            m_current = state;
            m_version = state.version;
        }
        return {true, m_version, 0, {}};
    }

    const SharedViewState& Current() const noexcept
    {
        return m_current;
    }
    uint64_t Version() const noexcept
    {
        return m_version;
    }
    void SetPeerCount(int count) noexcept
    {
        m_peerCount = count;
    }

  private:
    SharedViewState m_current;
    uint64_t m_version = 0;
    int m_peerCount = 0;
    ViewStateCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
