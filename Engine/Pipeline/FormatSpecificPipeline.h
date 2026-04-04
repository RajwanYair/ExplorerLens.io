// FormatSpecificPipeline.h — Format-Optimized Pipeline Configuration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides pre-configured pipeline configurations optimized for specific
// file formats, selecting optimal decode path and post-processing stages.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PipelineFormatCategory : uint8_t {
    RasterImage,
    VectorImage,
    RawPhoto,
    Video,
    Audio,
    Document,
    Archive,
    Model3D,
    Scientific
};

struct PipelineStageConfig
{
    std::string stageName;
    uint32_t order = 0;
    bool useGPU = false;
    bool optional = false;
};

struct FormatPipelineConfig
{
    std::string formatExtension;
    PipelineFormatCategory category = PipelineFormatCategory::RasterImage;
    std::vector<PipelineStageConfig> stages;
    uint32_t preferredThumbnailSize = 256;
    bool supportsAlpha = false;
    bool requiresColorManagement = false;
};

class FormatSpecificPipeline
{
  public:
    FormatSpecificPipeline()
    {
        RegisterDefaults();
    }

    FormatPipelineConfig GetConfig(const std::string& extension) const
    {
        auto it = m_configs.find(extension);
        if (it != m_configs.end())
            return it->second;
        return m_defaultConfig;
    }

    void RegisterConfig(const std::string& extension, const FormatPipelineConfig& config)
    {
        m_configs[extension] = config;
    }

    bool HasConfig(const std::string& extension) const
    {
        return m_configs.count(extension) > 0;
    }

    size_t GetRegisteredFormatCount() const
    {
        return m_configs.size();
    }

    std::vector<std::string> GetRegisteredFormats() const
    {
        std::vector<std::string> formats;
        formats.reserve(m_configs.size());
        for (const auto& [ext, _] : m_configs) {
            formats.push_back(ext);
        }
        return formats;
    }

    void SetDefaultConfig(const FormatPipelineConfig& config)
    {
        m_defaultConfig = config;
    }

  private:
    void RegisterDefaults()
    {
        m_configs[".jpg"] = {".jpg",
                             PipelineFormatCategory::RasterImage,
                             {{"Decode", 0, false, false}, {"Resize", 1, true, false}},
                             256,
                             false,
                             true};
        m_configs[".png"] = {".png",
                             PipelineFormatCategory::RasterImage,
                             {{"Decode", 0, false, false}, {"Resize", 1, true, false}},
                             256,
                             true,
                             true};
        m_configs[".webp"] = {".webp",
                              PipelineFormatCategory::RasterImage,
                              {{"Decode", 0, false, false}, {"Resize", 1, true, false}},
                              256,
                              true,
                              false};
        m_configs[".pdf"] = {".pdf",
                             PipelineFormatCategory::Document,
                             {{"Render", 0, false, false}, {"Resize", 1, false, false}},
                             256,
                             false,
                             false};
    }

    std::unordered_map<std::string, FormatPipelineConfig> m_configs;
    FormatPipelineConfig m_defaultConfig;
};

}  // namespace Engine
}  // namespace ExplorerLens
