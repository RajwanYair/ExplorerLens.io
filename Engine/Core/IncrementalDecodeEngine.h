// IncrementalDecodeEngine.h — Progressive Thumbnail Refinement
// Copyright (c) 2026 ExplorerLens Project
//
// Produces increasingly refined thumbnail versions: first a fast low-quality
// preview, then progressively improves quality as decode resources allow.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RefinementLevel : uint8_t {
    Placeholder = 0,
    FastPreview,
    MediumQuality,
    HighQuality,
    Final
};

struct RefinementStep
{
    RefinementLevel level = RefinementLevel::Placeholder;
    uint32_t width = 0;
    uint32_t height = 0;
    float qualityFactor = 0.0f;
    double decodeTimeMs = 0.0;
};

struct IncrementalConfig
{
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    uint32_t maxRefinements = 4;
    double timeBudgetMs = 50.0;
    bool skipPlaceholder = false;
};

class IncrementalDecodeEngine
{
  public:
    IncrementalDecodeEngine() = default;

    bool StartDecode(const std::wstring& filePath, const IncrementalConfig& config)
    {
        if (filePath.empty())
            return false;
        m_config = config;
        m_currentLevel = RefinementLevel::Placeholder;
        m_active = true;
        return true;
    }

    RefinementStep RefineNext()
    {
        RefinementStep step;
        if (!m_active)
            return step;
        step.level = m_currentLevel;
        step.width = m_config.targetWidth;
        step.height = m_config.targetHeight;
        uint8_t lvl = static_cast<uint8_t>(m_currentLevel);
        step.qualityFactor = (lvl + 1) * 0.25f;
        if (lvl < static_cast<uint8_t>(RefinementLevel::Final))
            m_currentLevel = static_cast<RefinementLevel>(lvl + 1);
        return step;
    }

    bool IsComplete() const
    {
        return m_currentLevel == RefinementLevel::Final;
    }

    RefinementLevel GetCurrentLevel() const
    {
        return m_currentLevel;
    }
    void Cancel()
    {
        m_active = false;
    }

  private:
    IncrementalConfig m_config;
    RefinementLevel m_currentLevel = RefinementLevel::Placeholder;
    bool m_active = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
