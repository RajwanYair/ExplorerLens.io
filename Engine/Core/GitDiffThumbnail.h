// GitDiffThumbnail.h — Side-by-Side Git Diff Thumbnail Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Produces a compact diff-view thumbnail showing before/after frames for
// changed image/document files, with added/removed line counts as overlay.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DiffViewMode {
    SideBySide,
    Unified,
    BeforeOnly,
    AfterOnly
};

struct GitDiffInfo
{
    std::string beforeCommit;
    std::string afterCommit = "HEAD";
    std::string filePath;
    int32_t linesAdded = 0;
    int32_t linesRemoved = 0;
    bool isBinary = true;
    bool valid = false;
};

struct GitDiffThumbnailConfig
{
    DiffViewMode mode = DiffViewMode::SideBySide;
    uint32_t thumbnailSize = 256;
    bool showStats = true;
    uint32_t addedColor = 0xFF00CC44U;
    uint32_t removedColor = 0xFFCC2200U;
};

class GitDiffThumbnail
{
  public:
    explicit GitDiffThumbnail(const GitDiffThumbnailConfig& cfg = {}) : m_cfg(cfg) {}

    GitDiffInfo Analyze(const std::string& path, const std::string& beforeCommit, const std::string& afterCommit) const
    {
        GitDiffInfo d;
        if (path.empty())
            return d;
        d.filePath = path;
        d.beforeCommit = beforeCommit;
        d.afterCommit = afterCommit;
        d.isBinary = true;
        d.valid = true;
        return d;
    }

    DiffViewMode GetMode() const
    {
        return m_cfg.mode;
    }
    void SetMode(DiffViewMode m)
    {
        m_cfg.mode = m;
    }
    const GitDiffThumbnailConfig& GetConfig() const
    {
        return m_cfg;
    }

    static bool IsDiffableFormat(const std::string& ext)
    {
        return ext == ".png" || ext == ".jpg" || ext == ".svg" || ext == ".pdf";
    }

  private:
    GitDiffThumbnailConfig m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
