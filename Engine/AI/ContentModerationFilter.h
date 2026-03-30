// ContentModerationFilter.h — Generative Thumbnail Content Screening
// Copyright (c) 2026 ExplorerLens Project
//
// Screens generated thumbnails for inappropriate content before displaying or caching,
// supporting tiered policy enforcement from Off through Enterprise compliance levels.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class ModerationTier : uint8_t
{
    Off,
    Standard,
    Strict,
    Enterprise
};

enum class ContentFlag : uint8_t
{
    Safe,
    AdultContent,
    Violence,
    Misinformation,
    CopyrightRisk,
    LowQuality
};

struct ModerationResult
{
    ContentFlag flag             = ContentFlag::Safe;
    float       confidenceScore  = 0.0f;
    bool        blockedByPolicy  = false;
    bool        reviewRequired   = false;
    uint64_t    moderatedAtMs    = 0;
};

class ContentModerationFilter
{
public:
    ContentModerationFilter() = default;
    ~ContentModerationFilter() = default;

    ContentModerationFilter(ContentModerationFilter const&)            = delete;
    ContentModerationFilter& operator=(ContentModerationFilter const&) = delete;
    ContentModerationFilter(ContentModerationFilter&&)                 = default;
    ContentModerationFilter& operator=(ContentModerationFilter&&)      = default;

    ModerationResult Evaluate(
        void const* imageData,
        uint32_t    width,
        uint32_t    height);

    void SetTier(ModerationTier tier);

    void AddCustomBlocklist(
        std::string const& category,
        float              threshold);

    [[nodiscard]] ModerationTier             GetTier() const;
    [[nodiscard]] std::vector<std::string>   GetBlocklistCategories() const;

private:
    ModerationTier                             m_tier        = ModerationTier::Standard;
    std::unordered_map<std::string, float>     m_blocklist;
    bool                                       m_modelLoaded = false;
};

} // namespace Engine
} // namespace ExplorerLens
