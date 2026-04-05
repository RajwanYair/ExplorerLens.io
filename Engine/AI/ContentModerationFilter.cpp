// ContentModerationFilter.cpp — AI-Powered Content Moderation Filter
// Copyright (c) 2026 ExplorerLens Project
//
#include "ContentModerationFilter.h"

namespace ExplorerLens::Engine {

ModerationResult ContentModerationFilter::Evaluate(const void*, uint32_t, uint32_t)
{
    return {};
}
void ContentModerationFilter::SetTier(ModerationTier tier) { m_tier = tier; }
void ContentModerationFilter::AddCustomBlocklist(const std::string& category, float threshold)
{
    m_blocklist[category] = threshold;
}
ModerationTier ContentModerationFilter::GetTier() const
{
    return m_tier;
}
std::vector<std::string> ContentModerationFilter::GetBlocklistCategories() const
{
    std::vector<std::string> cats;
    cats.reserve(m_blocklist.size());
    for (const auto& [k, v] : m_blocklist)
        cats.push_back(k);
    return cats;
}

}  // namespace ExplorerLens::Engine
