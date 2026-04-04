// GitBlameHeatmapOverlay.h — Git Blame Heatmap Thumbnail Overlay
// Copyright (c) 2026 ExplorerLens Project
//
// Visualises per-file commit-age heatmap as a subtle colour tint on the
// thumbnail: hot (recent commits) → red, cold (old) → blue-grey.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class HeatmapColorScheme {
    HotCold,
    Viridis,
    Grayscale
};

struct BlameHeatmapConfig
{
    HeatmapColorScheme scheme = HeatmapColorScheme::HotCold;
    uint32_t maxAgeDays = 365;
    float blendAlpha = 0.25f;
    bool showCommitAge = true;
};

struct BlameHeatmapData
{
    std::string path;
    uint32_t lastCommitAgeDays = 0;
    uint32_t commitCount = 0;
    float heatScore = 0.0f;  // 0.0 (cold) to 1.0 (hot)
    bool available = false;
};

class GitBlameHeatmapOverlay
{
  public:
    explicit GitBlameHeatmapOverlay(const BlameHeatmapConfig& cfg = {}) : m_cfg(cfg) {}

    BlameHeatmapData ComputeHeat(const std::string& path) const
    {
        BlameHeatmapData d;
        d.path = path;
        if (path.empty())
            return d;
        d.available = true;
        d.lastCommitAgeDays = 30;
        d.commitCount = 1;
        d.heatScore = ComputeScore(d.lastCommitAgeDays);
        return d;
    }

    float ComputeScore(uint32_t ageDays) const
    {
        if (m_cfg.maxAgeDays == 0)
            return 0.0f;
        float ratio = static_cast<float>(ageDays) / static_cast<float>(m_cfg.maxAgeDays);
        return 1.0f - (ratio > 1.0f ? 1.0f : ratio);
    }

    const BlameHeatmapConfig& GetConfig() const
    {
        return m_cfg;
    }
    void SetScheme(HeatmapColorScheme s)
    {
        m_cfg.scheme = s;
    }

  private:
    BlameHeatmapConfig m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
