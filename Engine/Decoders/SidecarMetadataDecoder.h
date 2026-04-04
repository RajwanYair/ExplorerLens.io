// SidecarMetadataDecoder.h — XMP sidecar file reader
// Copyright (c) 2026 ExplorerLens Project
//
// Reads XMP sidecar files (.xmp) alongside RAW images to extract
// editing metadata (crop, rotation, ratings) for thumbnail generation.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct SidecarMetadataDecoderConfig
{
    bool enabled = true;
    uint32_t maxSidecarSizeKB = 256;
    std::string sidecarExtension = ".xmp";
    std::string label = "SidecarMetadataDecoder";
};

class SidecarMetadataDecoder
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
    SidecarMetadataDecoderConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct SidecarData
    {
        float cropLeft = 0, cropTop = 0, cropRight = 1.0f, cropBottom = 1.0f;
        int16_t rotation = 0;
        int8_t rating = 0;
        bool hasCrop = false;
        bool hasRotation = false;
    };

    std::string GetSidecarPath(const std::string& imagePath) const
    {
        auto dot = imagePath.rfind('.');
        if (dot == std::string::npos)
            return imagePath + m_config.sidecarExtension;
        return imagePath.substr(0, dot) + m_config.sidecarExtension;
    }

    bool HasCrop(const SidecarData& data) const
    {
        return data.hasCrop;
    }
    bool HasRotation(const SidecarData& data) const
    {
        return data.hasRotation;
    }

  private:
    bool m_initialized = false;
    SidecarMetadataDecoderConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
