// StyleTransferRenderer.h — Neural Style Transfer for Artistic Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Applies neural style transfer to thumbnails for artistic rendering modes,
// supporting photorealistic, impressionist, watercolor, sketch, HDR, and cinematic styles.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace ExplorerLens {
namespace Engine {

enum class ArtisticStyle : uint8_t
{
    Photorealistic,
    Impressionist,
    Watercolor,
    Sketch,
    HDR,
    Cinematic
};

enum class StyleStrength : uint8_t
{
    Subtle,
    Moderate,
    Strong,
    Extreme
};

struct StyleParams
{
    ArtisticStyle style          = ArtisticStyle::Photorealistic;
    StyleStrength strength       = StyleStrength::Moderate;
    bool          preserveColors = true;
    float         blendFactor    = 0.5f;
};

class StyleTransferRenderer
{
public:
    StyleTransferRenderer() = default;
    ~StyleTransferRenderer() = default;

    StyleTransferRenderer(StyleTransferRenderer const&)            = delete;
    StyleTransferRenderer& operator=(StyleTransferRenderer const&) = delete;
    StyleTransferRenderer(StyleTransferRenderer&&)                 = default;
    StyleTransferRenderer& operator=(StyleTransferRenderer&&)      = default;

    bool ApplyStyle(
        void*              imageData,
        uint32_t           width,
        uint32_t           height,
        StyleParams const& params);

    bool LoadStyleModel(std::string const& path);
    void UnloadStyleModel();

    [[nodiscard]] std::vector<ArtisticStyle>   GetAvailableStyles() const;
    [[nodiscard]] std::optional<ArtisticStyle> GetLastAppliedStyle() const;

private:
    std::optional<ArtisticStyle> m_loadedStyle;
    std::string                  m_modelPath;
    bool                         m_modelLoaded = false;
};

} // namespace Engine
} // namespace ExplorerLens
