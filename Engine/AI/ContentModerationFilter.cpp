// ContentModerationFilter.cpp — AI-Powered Content Moderation Filter
// Copyright (c) 2026 ExplorerLens Project
//
#include "ContentModerationFilter.h"

namespace ExplorerLens::Engine {

ModerationResult ContentModerationFilter::Evaluate(const void*, uint32_t, uint32_t) { return {}; }
void ContentModerationFilter::SetTier(ModerationTier) {}
void ContentModerationFilter::AddCustomBlocklist(const std::string&, float) {}
ModerationTier ContentModerationFilter::GetTier() const { return {}; }
std::vector<std::string> ContentModerationFilter::GetBlocklistCategories() const { return {}; }

} // namespace ExplorerLens::Engine
