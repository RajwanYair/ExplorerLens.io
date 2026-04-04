// MSIXStreamingPrewarmer.h — MSIX Streaming Install Prewarmer
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors MSIX streaming install progress and pre-warms the thumbnail decoder
// pipeline so that thumbnails are available immediately once each content group lands.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MSIXInstallState {
    NotStarted,
    Streaming,
    ContentGroupReady,
    Complete,
    Failed
};

struct MSIXContentGroup
{
    std::string name;
    int ordinal = 0;
    bool required = false;
    double progressPct = 0.0;
    MSIXInstallState state = MSIXInstallState::NotStarted;
};

struct MSIXPrewarmResult
{
    bool success = false;
    int decodersPrewarmed = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

struct MSIXStreamingStats
{
    int groupsReady = 0;
    int groupsTotal = 0;
    int prewarmsIssued = 0;
    double ProgressPct() const noexcept
    {
        return groupsTotal > 0 ? (100.0 * groupsReady / groupsTotal) : 0.0;
    }
};

using MSIXGroupReadyCallback = std::function<void(const MSIXContentGroup&)>;

class MSIXStreamingPrewarmer
{
  public:
    explicit MSIXStreamingPrewarmer() = default;
    void SetGroupReadyCallback(MSIXGroupReadyCallback cb)
    {
        m_callback = std::move(cb);
    }

    void RegisterGroup(MSIXContentGroup group)
    {
        m_stats.groupsTotal++;
        m_groups.push_back(std::move(group));
    }

    MSIXPrewarmResult NotifyGroupReady(const std::string& groupName)
    {
        for (auto& g : m_groups) {
            if (g.name == groupName) {
                g.state = MSIXInstallState::ContentGroupReady;
                g.progressPct = 100.0;
                m_stats.groupsReady++;
                m_stats.prewarmsIssued++;
                if (m_callback)
                    m_callback(g);
                return {true, 1, {}};
            }
        }
        return {false, 0, "Group not registered: " + groupName};
    }

    MSIXStreamingStats GetStats() const noexcept
    {
        return m_stats;
    }
    int GroupCount() const noexcept
    {
        return static_cast<int>(m_groups.size());
    }

  private:
    std::vector<MSIXContentGroup> m_groups;
    MSIXStreamingStats m_stats;
    MSIXGroupReadyCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
