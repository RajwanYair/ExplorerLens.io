// FileSizeProportionBadge.h — File Size Proportion Indicator Badges
// Copyright (c) 2026 ExplorerLens Project
//
// Generates file size proportion indicators and human-readable formatting
// for visual badge overlays. Pure size classification and formatting with
// no rendering — outputs categories, colors, and proportion values.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SizeCategory : uint8_t {
    Tiny = 0,    // < 1 KB
    Small = 1,   // 1 KB – 100 KB
    Medium = 2,  // 100 KB – 10 MB
    Large = 3,   // 10 MB – 1 GB
    Huge = 4,    // 1 GB – 100 GB
    Massive = 5  // > 100 GB
};

struct BadgeColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

class FileSizeProportionBadge
{
  public:
    FileSizeProportionBadge() = default;

    // ---------------------------------------------------------------
    // Set the file size for this badge
    // ---------------------------------------------------------------
    void SetFileSize(uint64_t bytes) noexcept
    {
        m_fileSize = bytes;
    }

    uint64_t GetFileSize() const noexcept
    {
        return m_fileSize;
    }

    // ---------------------------------------------------------------
    // Classify the file size into a category
    // ---------------------------------------------------------------
    SizeCategory GetSizeCategory() const noexcept
    {
        return ClassifySize(m_fileSize);
    }

    static SizeCategory ClassifySize(uint64_t bytes) noexcept
    {
        constexpr uint64_t KB = 1024ULL;
        constexpr uint64_t MB = 1024ULL * KB;
        constexpr uint64_t GB = 1024ULL * MB;

        if (bytes < KB)
            return SizeCategory::Tiny;
        if (bytes < 100 * KB)
            return SizeCategory::Small;
        if (bytes < 10 * MB)
            return SizeCategory::Medium;
        if (bytes < GB)
            return SizeCategory::Large;
        if (bytes < 100 * GB)
            return SizeCategory::Huge;
        return SizeCategory::Massive;
    }

    // ---------------------------------------------------------------
    // Format bytes into human-readable string (decimal: KB, MB, GB)
    // ---------------------------------------------------------------
    static std::string FormatHumanReadable(uint64_t bytes) noexcept
    {
        constexpr uint64_t KB = 1024ULL;
        constexpr uint64_t MB = 1024ULL * KB;
        constexpr uint64_t GB = 1024ULL * MB;
        constexpr uint64_t TB = 1024ULL * GB;

        char buf[64] = {};

        if (bytes >= TB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(TB);
            std::snprintf(buf, sizeof(buf), "%.2f TB", val);
        } else if (bytes >= GB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(GB);
            std::snprintf(buf, sizeof(buf), "%.2f GB", val);
        } else if (bytes >= MB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(MB);
            std::snprintf(buf, sizeof(buf), "%.2f MB", val);
        } else if (bytes >= KB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(KB);
            std::snprintf(buf, sizeof(buf), "%.2f KB", val);
        } else {
            std::snprintf(buf, sizeof(buf), "%llu B", static_cast<unsigned long long>(bytes));
        }

        return std::string(buf);
    }

    // ---------------------------------------------------------------
    // Format bytes in binary units (KiB, MiB, GiB, TiB)
    // ---------------------------------------------------------------
    static std::string FormatBinarySize(uint64_t bytes) noexcept
    {
        constexpr uint64_t KiB = 1024ULL;
        constexpr uint64_t MiB = 1024ULL * KiB;
        constexpr uint64_t GiB = 1024ULL * MiB;
        constexpr uint64_t TiB = 1024ULL * GiB;

        char buf[64] = {};

        if (bytes >= TiB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(TiB);
            std::snprintf(buf, sizeof(buf), "%.2f TiB", val);
        } else if (bytes >= GiB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(GiB);
            std::snprintf(buf, sizeof(buf), "%.2f GiB", val);
        } else if (bytes >= MiB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(MiB);
            std::snprintf(buf, sizeof(buf), "%.2f MiB", val);
        } else if (bytes >= KiB) {
            const double val = static_cast<double>(bytes) / static_cast<double>(KiB);
            std::snprintf(buf, sizeof(buf), "%.2f KiB", val);
        } else {
            std::snprintf(buf, sizeof(buf), "%llu B", static_cast<unsigned long long>(bytes));
        }

        return std::string(buf);
    }

    // ---------------------------------------------------------------
    // Get badge color based on size category
    //   Tiny=gray, Small=green, Medium=blue, Large=orange, Huge=red, Massive=purple
    // ---------------------------------------------------------------
    static BadgeColor GetBadgeColor(SizeCategory category) noexcept
    {
        switch (category) {
            case SizeCategory::Tiny:
                return {160, 160, 160, 255};  // Gray
            case SizeCategory::Small:
                return {76, 175, 80, 255};  // Green
            case SizeCategory::Medium:
                return {33, 150, 243, 255};  // Blue
            case SizeCategory::Large:
                return {255, 152, 0, 255};  // Orange
            case SizeCategory::Huge:
                return {244, 67, 54, 255};  // Red
            case SizeCategory::Massive:
                return {156, 39, 176, 255};  // Purple
            default:
                return {128, 128, 128, 255};
        }
    }

    // ---------------------------------------------------------------
    // Proportion of bytes relative to maxBytes (0.0 – 1.0)
    // ---------------------------------------------------------------
    static double GetFilledProportion(uint64_t bytes, uint64_t maxBytes) noexcept
    {
        if (maxBytes == 0)
            return 0.0;
        if (bytes >= maxBytes)
            return 1.0;
        return static_cast<double>(bytes) / static_cast<double>(maxBytes);
    }

    // ---------------------------------------------------------------
    // Percentage string (e.g., "45.2%")
    // ---------------------------------------------------------------
    static std::string FormatPercentage(uint64_t bytes, uint64_t maxBytes) noexcept
    {
        const double pct = GetFilledProportion(bytes, maxBytes) * 100.0;
        char buf[32] = {};
        std::snprintf(buf, sizeof(buf), "%.1f%%", pct);
        return std::string(buf);
    }

  private:
    uint64_t m_fileSize = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
