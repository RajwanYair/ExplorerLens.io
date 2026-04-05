// ImageDescriptionSynthesizer.h — Natural-Language Thumbnail Description Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Generates natural-language descriptions of thumbnail content for accessibility
// and search indexing, with configurable depth and multi-language output support.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DescriptionDepth : uint8_t {
    Brief,
    Standard,
    Detailed,
    Exhaustive
};

enum class DescriptionLanguage : uint8_t {
    English,
    French,
    German,
    Spanish,
    Japanese,
    Chinese
};

struct SynthesisResult
{
    std::string description;
    float confidence = 0.0f;
    std::vector<std::string> keyTags;
    uint64_t generatedAtMs = 0;
};

class ImageDescriptionSynthesizer
{
public:
    ImageDescriptionSynthesizer() = default;
    ~ImageDescriptionSynthesizer() = default;

    ImageDescriptionSynthesizer(ImageDescriptionSynthesizer const&) = delete;
    ImageDescriptionSynthesizer& operator=(ImageDescriptionSynthesizer const&) = delete;
    ImageDescriptionSynthesizer(ImageDescriptionSynthesizer&&) = default;
    ImageDescriptionSynthesizer& operator=(ImageDescriptionSynthesizer&&) = default;

    SynthesisResult Synthesize(void const* imageData, uint32_t width, uint32_t height, DescriptionDepth depth);

    void SetLanguage(DescriptionLanguage language);
    bool LoadModel(std::string const& path);

    [[nodiscard]] DescriptionLanguage GetLanguage() const;
    [[nodiscard]] bool IsModelLoaded() const;

private:
    DescriptionLanguage m_language = DescriptionLanguage::English;
    std::string m_modelPath;
    bool m_modelLoaded = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
