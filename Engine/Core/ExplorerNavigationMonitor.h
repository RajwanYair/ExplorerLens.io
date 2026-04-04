// ExplorerNavigationMonitor.h — Explorer Folder Navigation Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors Windows Explorer navigation events to predictively prefetch
// thumbnails for folders the user is likely to browse next.
//
#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FolderNavigationEvent
{
    std::wstring folderPath;
    uint64_t timestamp = 0;
    uint32_t fileCount = 0;
    bool isBackNavigation = false;
};

struct PrefetchSuggestion
{
    std::wstring folderPath;
    float probability = 0.0f;
    uint32_t estimatedFiles = 0;
};

class ExplorerNavigationMonitor
{
  public:
    ExplorerNavigationMonitor() = default;

    void RecordNavigation(const FolderNavigationEvent& event)
    {
        m_history.push_back(event);
        if (m_history.size() > m_maxHistory)
            m_history.pop_front();
        m_totalNavigations++;
    }

    std::vector<PrefetchSuggestion> GetPrefetchSuggestions(uint32_t maxSuggestions = 3) const
    {
        std::vector<PrefetchSuggestion> suggestions;
        if (m_history.empty())
            return suggestions;
        suggestions.reserve(maxSuggestions);
        // Return parent directory as likely next navigation
        if (!m_history.back().folderPath.empty()) {
            suggestions.push_back(PrefetchSuggestion{m_history.back().folderPath, 0.5f, 10});
        }
        return suggestions;
    }

    uint64_t GetTotalNavigations() const
    {
        return m_totalNavigations;
    }
    size_t GetHistorySize() const
    {
        return m_history.size();
    }
    void ClearHistory()
    {
        m_history.clear();
    }
    void SetMaxHistory(size_t max)
    {
        m_maxHistory = max;
    }

  private:
    std::deque<FolderNavigationEvent> m_history;
    size_t m_maxHistory = 100;
    uint64_t m_totalNavigations = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
