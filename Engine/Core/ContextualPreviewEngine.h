// ContextualPreviewEngine.h — Context-aware preview quality selection
// Copyright (c) 2026 ExplorerLens Project
//
// Adjusts thumbnail quality and size based on Explorer view mode (icons,
// tiles, details, content) to optimize decode time vs visual quality.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct ContextualPreviewEngineConfig
{
    bool enabled = true;
    uint32_t minThumbnailSize = 32;
    uint32_t maxThumbnailSize = 1024;
    std::string label = "ContextualPreviewEngine";
};

class ContextualPreviewEngine
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    ContextualPreviewEngineConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class ViewMode : uint8_t {
        SmallIcons,
        MediumIcons,
        LargeIcons,
        ExtraLargeIcons,
        Tiles,
        Details,
        Content,
        List
    };

    struct QualityParams
    {
        uint32_t targetSize = 256;
        bool highQualityResize = true;
        bool applySharpening = false;
        uint8_t jpegQuality = 85;
    };

    QualityParams GetParamsForView(ViewMode mode) const
    {
        QualityParams p;
        switch (mode) {
            case ViewMode::SmallIcons:
                p = {48, false, false, 70};
                break;
            case ViewMode::MediumIcons:
                p = {96, false, false, 75};
                break;
            case ViewMode::LargeIcons:
                p = {256, true, false, 85};
                break;
            case ViewMode::ExtraLargeIcons:
                p = {512, true, true, 90};
                break;
            case ViewMode::Tiles:
                p = {128, true, false, 80};
                break;
            case ViewMode::Details:
                p = {32, false, false, 65};
                break;
            case ViewMode::Content:
                p = {128, true, false, 80};
                break;
            case ViewMode::List:
                p = {16, false, false, 60};
                break;
        }
        return p;
    }

  private:
    bool m_initialized = false;
    ContextualPreviewEngineConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
