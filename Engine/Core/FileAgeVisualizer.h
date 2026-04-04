// FileAgeVisualizer.h — File Age/Freshness Visualization
// Copyright (c) 2026 ExplorerLens Project
//
// Maps file age to color gradients and freshness categories for thumbnail
// overlays. Uses FILETIME/SYSTEMTIME from the Windows SDK.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

// Age categories ordered from newest to oldest
enum class AgeCategory : uint8_t {
    Fresh = 0,   // < 1 day
    Recent = 1,  // 1 day – 7 days
    Old = 2,     // 7 days – 90 days
    Stale = 3,   // 90 days – 365 days
    Archive = 4  // > 365 days
};

// RGBA color (0–255 per channel)
struct AgeColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

class FileAgeVisualizer
{
  public:
    // 100-nanosecond intervals per millisecond
    static constexpr int64_t kFileTimeTicksPerMs = 10'000LL;
    static constexpr int64_t kMsPerDay = 86'400'000LL;

    // Classify a file time into an age category relative to now (UTC).
    static AgeCategory GetAgeCategory(FILETIME fileTimeUtc) noexcept
    {
        FILETIME nowFt{};
        ::GetSystemTimeAsFileTime(&nowFt);

        const int64_t fileMs = FileTimeToMs(fileTimeUtc);
        const int64_t nowMs = FileTimeToMs(nowFt);

        if (fileMs <= 0 || nowMs <= 0)
            return AgeCategory::Archive;

        const int64_t ageDays = (nowMs - fileMs) / kMsPerDay;

        if (ageDays < 1)
            return AgeCategory::Fresh;
        if (ageDays < 7)
            return AgeCategory::Recent;
        if (ageDays < 90)
            return AgeCategory::Old;
        if (ageDays < 365)
            return AgeCategory::Stale;
        return AgeCategory::Archive;
    }

    // Map an age (in milliseconds since creation) to a gradient color.
    // Green (fresh) → Yellow → Orange → Red → Grey (archive).
    static AgeColor GetAgeColorGradient(int64_t ageMs) noexcept
    {
        if (ageMs < 0)
            ageMs = 0;

        const double ageDays = static_cast<double>(ageMs) / static_cast<double>(kMsPerDay);

        // Normalise to [0, 1] over 365 days, saturate beyond
        double t = (std::min)(ageDays / 365.0, 1.0);

        AgeColor c{};
        c.a = 255;

        if (t < 0.25) {
            // Green → Yellow  (0.0 – 0.25)
            const double s = t / 0.25;
            c.r = static_cast<uint8_t>(s * 255.0);
            c.g = 220;
            c.b = static_cast<uint8_t>((1.0 - s) * 60.0);
        } else if (t < 0.50) {
            // Yellow → Orange  (0.25 – 0.50)
            const double s = (t - 0.25) / 0.25;
            c.r = 255;
            c.g = static_cast<uint8_t>(220.0 - s * 90.0);
            c.b = 0;
        } else if (t < 0.75) {
            // Orange → Red  (0.50 – 0.75)
            const double s = (t - 0.50) / 0.25;
            c.r = 255;
            c.g = static_cast<uint8_t>(130.0 * (1.0 - s));
            c.b = 0;
        } else {
            // Red → Grey  (0.75 – 1.0)
            const double s = (t - 0.75) / 0.25;
            const auto grey = static_cast<uint8_t>(128.0 + s * 40.0);
            c.r = static_cast<uint8_t>(255.0 * (1.0 - s) + grey * s);
            c.g = static_cast<uint8_t>(grey * s);
            c.b = static_cast<uint8_t>(grey * s);
        }
        return c;
    }

    // Human-readable label for each age category.
    static const wchar_t* GetAgeLabelString(AgeCategory category) noexcept
    {
        switch (category) {
            case AgeCategory::Fresh:
                return L"Fresh";
            case AgeCategory::Recent:
                return L"Recent";
            case AgeCategory::Old:
                return L"Old";
            case AgeCategory::Stale:
                return L"Stale";
            case AgeCategory::Archive:
                return L"Archive";
        }
        return L"Unknown";
    }

    // Freshness score in [0.0, 1.0] where 1.0 = very fresh.
    // Considers both creation and last-modified times; the more recent
    // modification relative to creation signals active / living files.
    static double ComputeFreshnessScore(FILETIME createdTime, FILETIME modifiedTime) noexcept
    {
        FILETIME nowFt{};
        ::GetSystemTimeAsFileTime(&nowFt);

        const int64_t nowMs = FileTimeToMs(nowFt);
        const int64_t cMs = FileTimeToMs(createdTime);
        const int64_t mMs = FileTimeToMs(modifiedTime);

        if (nowMs <= 0 || cMs <= 0)
            return 0.0;

        // Age since creation (days)
        const double creationAgeDays =
            static_cast<double>((std::max)(nowMs - cMs, 0LL)) / static_cast<double>(kMsPerDay);

        // Recency of last edit (days ago)
        const double modAgeDays = static_cast<double>((std::max)(nowMs - mMs, 0LL)) / static_cast<double>(kMsPerDay);

        // Exponential decay: half-life of 30 days for creation, 14 for mod
        const double creationFresh = std::exp(-creationAgeDays / 30.0);
        const double modFresh = std::exp(-modAgeDays / 14.0);

        // Weighted blend: modification recency matters more
        const double score = 0.4 * creationFresh + 0.6 * modFresh;
        return (std::max)(0.0, (std::min)(1.0, score));
    }

  private:
    static int64_t FileTimeToMs(FILETIME ft) noexcept
    {
        ULARGE_INTEGER uli{};
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return static_cast<int64_t>(uli.QuadPart / static_cast<uint64_t>(kFileTimeTicksPerMs));
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
