// CommitBadgeCompositor.h — Git Commit Metadata Badge Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Renders a compact badge showing commit hash, author initials, and relative
// age ("3d", "2w") over the bottom edge of a thumbnail image.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class BadgePosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};
enum class BadgeSize {
    Tiny,
    Small,
    Medium,
    Large
};

struct CommitBadgeInfo
{
    std::string shortHash;
    std::string authorInitials;
    uint32_t ageDays = 0;
    std::string relativeAge;  // "3d", "2w", "1m"
    bool valid = false;
};

struct CommitBadgeConfig
{
    BadgePosition position = BadgePosition::BottomRight;
    BadgeSize size = BadgeSize::Small;
    bool showHash = true;
    bool showAuthor = false;
    bool showAge = true;
    uint8_t alpha = 200;
};

class CommitBadgeCompositor
{
  public:
    explicit CommitBadgeCompositor(const CommitBadgeConfig& cfg = {}) : m_cfg(cfg) {}

    CommitBadgeInfo Build(const std::string& hash, const std::string& author, uint32_t ageDays) const
    {
        CommitBadgeInfo b;
        if (hash.empty())
            return b;
        b.shortHash = hash.substr(0, 7);
        b.authorInitials = author.empty() ? "?" : author.substr(0, 2);
        b.ageDays = ageDays;
        b.relativeAge = FormatAge(ageDays);
        b.valid = true;
        return b;
    }

    static std::string FormatAge(uint32_t days)
    {
        if (days < 1)
            return "now";
        if (days < 7)
            return std::to_string(days) + "d";
        if (days < 30)
            return std::to_string(days / 7) + "w";
        if (days < 365)
            return std::to_string(days / 30) + "m";
        return std::to_string(days / 365) + "y";
    }

    const CommitBadgeConfig& GetConfig() const
    {
        return m_cfg;
    }
    void SetPosition(BadgePosition p)
    {
        m_cfg.position = p;
    }

  private:
    CommitBadgeConfig m_cfg;
};

}  // namespace Engine
}  // namespace ExplorerLens
