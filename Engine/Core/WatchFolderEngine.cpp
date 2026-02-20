#include "WatchFolderEngine.h"
#include <algorithm>
#include <chrono>

namespace DarkThumbs { namespace Engine {

WatchFolderEngine::WatchFolderEngine() = default;

const wchar_t* WatchFolderEngine::GetChangeTypeName(FileChangeType type) {
    switch (type) {
        case FileChangeType::Created:   return L"Created";
        case FileChangeType::Modified:  return L"Modified";
        case FileChangeType::Deleted:   return L"Deleted";
        case FileChangeType::Renamed:   return L"Renamed";
        case FileChangeType::Attribute: return L"Attribute";
        default:                        return L"Unknown";
    }
}

const wchar_t* WatchFolderEngine::GetWatchModeName(WatchMode mode) {
    switch (mode) {
        case WatchMode::Polling: return L"Polling";
        case WatchMode::Native:  return L"Native";
        case WatchMode::Hybrid:  return L"Hybrid";
        default:                 return L"Unknown";
    }
}

bool WatchFolderEngine::AddFolder(const std::wstring& path, bool recursive, WatchMode mode) {
    // Check for duplicates
    for (const auto& wf : m_watchFolders) {
        if (wf.path == path) return false;
    }
    WatchFolder wf;
    wf.path = path;
    wf.recursive = recursive;
    wf.mode = mode;
    wf.enabled = true;
    wf.changeCount = 0;
    m_watchFolders.push_back(std::move(wf));
    return true;
}

bool WatchFolderEngine::RemoveFolder(const std::wstring& path) {
    auto it = std::remove_if(m_watchFolders.begin(), m_watchFolders.end(),
        [&](const WatchFolder& wf) { return wf.path == path; });
    if (it == m_watchFolders.end()) return false;
    m_watchFolders.erase(it, m_watchFolders.end());
    return true;
}

void WatchFolderEngine::SimulateChange(const std::wstring& path, FileChangeType type) {
    // Update change count for matching watch folder
    for (auto& wf : m_watchFolders) {
        if (path.find(wf.path) == 0) {
            wf.changeCount++;
        }
    }
    if (m_callback) {
        FileChangeEvent evt;
        evt.filePath = path;
        evt.changeType = type;
        auto now = std::chrono::system_clock::now();
        evt.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        m_callback(evt);
    }
}

}} // namespace DarkThumbs::Engine
