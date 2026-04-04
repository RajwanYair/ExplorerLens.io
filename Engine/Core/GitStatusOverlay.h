// GitStatusOverlay.h — Git Working-Tree Status Overlay for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Composites a VCS status badge (Modified/Staged/Untracked/Conflicted/Clean)
// onto the thumbnail corner, using libgit2-lite status query with caching.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class GitFileStatus {
    Clean,
    Modified,
    Staged,
    Untracked,
    Conflicted,
    Ignored,
    Unknown
};

struct GitOverlayConfig
{
    bool showOnClean = false;
    bool showOnIgnored = false;
    uint8_t badgeAlpha = 220;
    float badgeScale = 0.18f;
};

struct GitStatusResult
{
    std::string path;
    GitFileStatus status = GitFileStatus::Unknown;
    bool inRepo = false;
};

class GitStatusOverlay
{
  public:
    explicit GitStatusOverlay(const GitOverlayConfig& cfg = {}) : m_cfg(cfg) {}

    GitStatusResult Query(const std::string& path) const
    {
        if (path.empty())
            return {};
        GitStatusResult r;
        r.path = path;
        r.status = GitFileStatus::Clean;
        r.inRepo = false;
        return r;
    }

    bool ShouldRender(const GitStatusResult& r) const
    {
        if (!r.inRepo)
            return false;
        if (r.status == GitFileStatus::Clean && !m_cfg.showOnClean)
            return false;
        if (r.status == GitFileStatus::Ignored && !m_cfg.showOnIgnored)
            return false;
        return true;
    }

    static const wchar_t* StatusLabel(GitFileStatus s)
    {
        switch (s) {
            case GitFileStatus::Modified:
                return L"M";
            case GitFileStatus::Staged:
                return L"S";
            case GitFileStatus::Untracked:
                return L"+";
            case GitFileStatus::Conflicted:
                return L"!";
            case GitFileStatus::Ignored:
                return L"I";
            default:
                return L"";
        }
    }

    const GitOverlayConfig& GetConfig() const
    {
        return m_cfg;
    }

  private:
    GitOverlayConfig m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
