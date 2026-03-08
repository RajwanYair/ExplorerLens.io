// ColorProfileValidator.h — Validates ICC color profiles in image files
// Copyright (c) 2026 ExplorerLens Project
//
// Checks ICC profile integrity, version compatibility, and rendering intent
// before applying color management to decoded thumbnails.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ColorProfileValidatorConfig {
    bool enabled = true;
    uint32_t maxProfileSizeKB = 512;
    std::string label = "ColorProfileValidator";
};

class ColorProfileValidator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ColorProfileValidatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class ProfileVersion : uint8_t { V2, V4, Unknown };
    enum class RenderIntent : uint8_t { Perceptual, RelativeColorimetric, Saturation, AbsoluteColorimetric };

    bool ValidateProfileSize(uint32_t sizeBytes) const {
        return sizeBytes > 128 && sizeBytes <= m_config.maxProfileSizeKB * 1024;
    }

    bool ValidateSignature(uint32_t signature) const {
        return signature == 0x61637370; // 'acsp'
    }

    ProfileVersion DetectVersion(uint8_t major) const {
        if (major == 2) return ProfileVersion::V2;
        if (major == 4) return ProfileVersion::V4;
        return ProfileVersion::Unknown;
    }

private:
    bool m_initialized = false;
    ColorProfileValidatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
