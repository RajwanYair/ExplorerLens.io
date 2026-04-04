// CalendarHeatmapRenderer.h — Calendar-Style Heatmap Layout Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Generates calendar-style heatmap layouts for file modification activity
// visualization. Pure calendar math and geometry — computes cell positions,
// heat levels, and day-of-week calculations without any rendering library.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class HeatLevel : uint8_t {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Max = 4
};

struct HeatmapCell
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

class CalendarHeatmapRenderer
{
  public:
    CalendarHeatmapRenderer()
    {
        m_dayCounts.fill(0);
    }

    // ---------------------------------------------------------------
    // Record activity for a specific date
    // ---------------------------------------------------------------
    bool AddActivity(uint32_t year, uint32_t month, uint32_t day, uint32_t count) noexcept
    {
        if (month < 1 || month > 12 || day < 1 || day > 31)
            return false;
        if (day > DaysInMonth(year, month))
            return false;

        const uint32_t dayOfYear = DayOfYear(year, month, day);
        if (dayOfYear == 0 || dayOfYear > 366)
            return false;

        m_dayCounts[dayOfYear - 1] += count;

        // Track year for later queries
        if (m_year == 0)
            m_year = year;
        return true;
    }

    // ---------------------------------------------------------------
    // Get the rectangle for a specific day-of-year cell in a canvas
    // Layout: columns = weeks (up to 53), rows = weekdays (7)
    // ---------------------------------------------------------------
    HeatmapCell GetCellRect(uint32_t dayOfYear, uint32_t canvasW, uint32_t canvasH) const noexcept
    {
        if (dayOfYear == 0 || dayOfYear > 366)
            return {};
        if (canvasW == 0 || canvasH == 0)
            return {};

        const uint32_t totalWeeks = GetWeeksInYear(m_year > 0 ? m_year : 2026);
        if (totalWeeks == 0)
            return {};

        // First day offset: which weekday does Jan 1 fall on (0=Sun..6=Sat)
        const uint32_t jan1Dow = GetDayOfWeek(m_year > 0 ? m_year : 2026, 1, 1);
        const uint32_t adjusted = (dayOfYear - 1) + jan1Dow;

        const uint32_t weekCol = adjusted / 7;
        const uint32_t dayRow = adjusted % 7;

        // Cell dimensions with 1px gap
        const uint32_t cellW = canvasW / (totalWeeks + 1);
        const uint32_t cellH = canvasH / 7;
        const uint32_t gap = (std::max)(1u, (std::min)(cellW, cellH) / 8);

        const uint32_t effectiveW = (cellW > gap) ? cellW - gap : 1;
        const uint32_t effectiveH = (cellH > gap) ? cellH - gap : 1;

        return {static_cast<int32_t>(weekCol * cellW), static_cast<int32_t>(dayRow * cellH), effectiveW, effectiveH};
    }

    // ---------------------------------------------------------------
    // Map a count to a heat level (0-4) relative to the maximum
    // ---------------------------------------------------------------
    static HeatLevel GetHeatLevel(uint32_t count, uint32_t maxCount) noexcept
    {
        if (maxCount == 0 || count == 0)
            return HeatLevel::None;
        if (count >= maxCount)
            return HeatLevel::Max;

        // Quartile-based thresholds
        const uint32_t quarter = maxCount / 4;
        if (quarter == 0)
            return (count > 0) ? HeatLevel::Max : HeatLevel::None;

        if (count <= quarter)
            return HeatLevel::Low;
        if (count <= quarter * 2)
            return HeatLevel::Medium;
        if (count <= quarter * 3)
            return HeatLevel::High;
        return HeatLevel::Max;
    }

    // ---------------------------------------------------------------
    // Number of ISO weeks that contain days of the given year
    // A year has 53 weeks if Jan 1 or Dec 31 is a Thursday
    // ---------------------------------------------------------------
    static uint32_t GetWeeksInYear(uint32_t year) noexcept
    {
        if (year == 0)
            return 0;
        // Compute day-of-week for Jan 1 using Tomohiko Sakamoto's algorithm
        const uint32_t jan1 = GetDayOfWeek(year, 1, 1);
        const uint32_t dec31 = GetDayOfWeek(year, 12, 31);

        // A year has 53 weeks if it starts or ends on Thursday
        if (jan1 == 4 || dec31 == 4)
            return 53;
        return 52;
    }

    // ---------------------------------------------------------------
    // Day of week: 0=Sunday, 1=Monday, ..., 6=Saturday
    // Tomohiko Sakamoto's algorithm
    // ---------------------------------------------------------------
    static uint32_t GetDayOfWeek(uint32_t year, uint32_t month, uint32_t day) noexcept
    {
        if (month < 1 || month > 12 || day < 1 || year == 0)
            return 0;

        static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        uint32_t y = year;
        if (month < 3)
            y -= 1;
        return (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
    }

    // ---------------------------------------------------------------
    // Retrieve recorded activity count for a day
    // ---------------------------------------------------------------
    uint32_t GetActivityCount(uint32_t dayOfYear) const noexcept
    {
        if (dayOfYear == 0 || dayOfYear > 366)
            return 0;
        return m_dayCounts[dayOfYear - 1];
    }

    // ---------------------------------------------------------------
    // Maximum activity across all days (for normalization)
    // ---------------------------------------------------------------
    uint32_t GetMaxActivity() const noexcept
    {
        uint32_t maxVal = 0;
        for (const auto& c : m_dayCounts) {
            maxVal = (std::max)(maxVal, c);
        }
        return maxVal;
    }

  private:
    // ---------------------------------------------------------------
    // Calendar utilities
    // ---------------------------------------------------------------
    static bool IsLeapYear(uint32_t year) noexcept
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static uint32_t DaysInMonth(uint32_t year, uint32_t month) noexcept
    {
        static const uint32_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month < 1 || month > 12)
            return 0;
        if (month == 2 && IsLeapYear(year))
            return 29;
        return days[month - 1];
    }

    static uint32_t DayOfYear(uint32_t year, uint32_t month, uint32_t day) noexcept
    {
        uint32_t doy = 0;
        for (uint32_t m = 1; m < month; ++m) {
            doy += DaysInMonth(year, m);
        }
        return doy + day;
    }

    std::array<uint32_t, 366> m_dayCounts{};
    uint32_t m_year = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
