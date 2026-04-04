// ThumbnailPersonalisationEngine.cpp — AI Thumbnail Personalisation Engine
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailPersonalisationEngine.h"

namespace ExplorerLens::Engine {

bool ThumbnailPersonalisationEngine::ApplyPersonalisation(const UserPersonalisationProfile&, void*, uint32_t, uint32_t)
{
    return false;
}
void ThumbnailPersonalisationEngine::UpdateSignal(const std::string&, PersonalisationSignal, float) {}
void ThumbnailPersonalisationEngine::ResetProfile(const std::string&) {}
std::optional<UserPersonalisationProfile> ThumbnailPersonalisationEngine::GetProfile(const std::string&) const
{
    return {};
}

}  // namespace ExplorerLens::Engine
