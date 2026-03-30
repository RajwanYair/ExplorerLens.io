// ThumbnailPersonalisationEngine.h — User-Adaptive Thumbnail Rendering
// Copyright (c) 2026 ExplorerLens Project
//
// Adapts thumbnail rendering style based on user preferences and usage history,
// applying personalisation signals such as view duration, stars, and edit history.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class PersonalisationSignal : uint8_t
{
    RecentlyViewed,
    Starred,
    SharedWith,
    EditHistory,
    ViewDuration
};

enum class AdaptationStrategy : uint8_t
{
    Conservative,
    Balanced,
    Aggressive,
    Fixed
};

struct UserPersonalisationProfile
{
    std::string                      userId;
    std::vector<PersonalisationSignal> signals;
    AdaptationStrategy               strategy            = AdaptationStrategy::Balanced;
    float                            confidenceThreshold = 0.7f;
    uint32_t                         maxHistoryDays      = 30;
};

class ThumbnailPersonalisationEngine
{
public:
    ThumbnailPersonalisationEngine() = default;
    ~ThumbnailPersonalisationEngine() = default;

    ThumbnailPersonalisationEngine(ThumbnailPersonalisationEngine const&)            = delete;
    ThumbnailPersonalisationEngine& operator=(ThumbnailPersonalisationEngine const&) = delete;
    ThumbnailPersonalisationEngine(ThumbnailPersonalisationEngine&&)                 = default;
    ThumbnailPersonalisationEngine& operator=(ThumbnailPersonalisationEngine&&)      = default;

    bool ApplyPersonalisation(
        UserPersonalisationProfile const& profile,
        void*                             imageData,
        uint32_t                          w,
        uint32_t                          h);

    void UpdateSignal(
        std::string const&   userId,
        PersonalisationSignal signal,
        float                weight);

    void ResetProfile(std::string const& userId);

    [[nodiscard]] std::optional<UserPersonalisationProfile>
        GetProfile(std::string const& userId) const;

private:
    std::unordered_map<std::string, UserPersonalisationProfile> m_profiles;
    AdaptationStrategy                                          m_strategy = AdaptationStrategy::Balanced;
};

} // namespace Engine
} // namespace ExplorerLens
