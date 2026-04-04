// ExplorerStatusBarProvider.h — Explorer Status Bar Information Provider
// Copyright (c) 2026 ExplorerLens Project
//
// Provides formatted status information for Explorer's status bar area.
// Reports decode counts, cache hit rates, and format-specific stats
// in a compact human-readable format.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <sstream>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct StatusBarInfo
{
    std::wstring primaryText;    // Main status text
    std::wstring secondaryText;  // Detailed status
    std::wstring tooltipText;    // Full explanation
    uint32_t iconId = 0;         // 0=none, 1=ok, 2=warning, 3=error
    bool isAnimating = false;    // Show activity indicator
};

struct StatusBarConfig
{
    bool showDecodeCount = true;
    bool showCacheHitRate = true;
    bool showFormatCount = true;
    bool showMemoryUsage = true;
    uint32_t updateIntervalMs = 500;
};

struct StatusBarStats
{
    uint32_t updatesGenerated = 0;
    uint32_t warningsShown = 0;
    uint32_t errorsShown = 0;
};

class ExplorerStatusBarProvider
{
  public:
    ExplorerStatusBarProvider()
    {
        InitializeSRWLock(&m_lock);
    }
    ~ExplorerStatusBarProvider() = default;

    static const wchar_t* GetName()
    {
        return L"ExplorerStatusBarProvider";
    }

    void Configure(const StatusBarConfig& config)
    {
        m_config = config;
    }

    /// Update the decode counters.
    void SetDecodeCount(uint32_t total, uint32_t cached, uint32_t errors)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_totalDecodes = total;
        m_cachedDecodes = cached;
        m_errorDecodes = errors;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Update memory usage info.
    void SetMemoryUsage(uint64_t usedBytes, uint64_t budgetBytes)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_memUsedBytes = usedBytes;
        m_memBudgetBytes = budgetBytes;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Set the active format count.
    void SetFormatCount(uint32_t count)
    {
        m_formatCount = count;
    }

    /// Generate current status bar content.
    StatusBarInfo Generate() const
    {
        StatusBarInfo info;

        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        // Primary text: decode count + cache hit rate
        std::wostringstream primary;
        if (m_config.showDecodeCount) {
            primary << m_totalDecodes << L" decoded";
        }
        if (m_config.showCacheHitRate && m_totalDecodes > 0) {
            double hitRate = 100.0 * m_cachedDecodes / m_totalDecodes;
            primary << L" | " << static_cast<int>(hitRate) << L"% cached";
        }
        info.primaryText = primary.str();

        // Secondary text: memory usage
        std::wostringstream secondary;
        if (m_config.showMemoryUsage && m_memBudgetBytes > 0) {
            double usedMB = m_memUsedBytes / (1024.0 * 1024.0);
            double budgetMB = m_memBudgetBytes / (1024.0 * 1024.0);
            secondary << static_cast<int>(usedMB) << L"/" << static_cast<int>(budgetMB) << L" MB";
        }
        if (m_config.showFormatCount) {
            secondary << L" | " << m_formatCount << L" formats";
        }
        info.secondaryText = secondary.str();

        // Icon state
        if (m_errorDecodes > 0) {
            info.iconId = 3;  // Error
        } else if (m_memBudgetBytes > 0 && m_memUsedBytes > m_memBudgetBytes * 0.9) {
            info.iconId = 2;  // Warning
        } else {
            info.iconId = 1;  // OK
        }

        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        m_stats.updatesGenerated++;
        return info;
    }

    /// Format a byte count to human-readable string.
    static std::wstring FormatBytes(uint64_t bytes)
    {
        if (bytes < 1024)
            return std::to_wstring(bytes) + L" B";
        if (bytes < 1048576)
            return std::to_wstring(bytes / 1024) + L" KB";
        if (bytes < 1073741824)
            return std::to_wstring(bytes / 1048576) + L" MB";
        return std::to_wstring(bytes / 1073741824) + L" GB";
    }

    StatusBarStats GetStats() const
    {
        return m_stats;
    }

  private:
    SRWLOCK m_lock{};
    StatusBarConfig m_config;
    uint32_t m_totalDecodes = 0;
    uint32_t m_cachedDecodes = 0;
    uint32_t m_errorDecodes = 0;
    uint64_t m_memUsedBytes = 0;
    uint64_t m_memBudgetBytes = 0;
    uint32_t m_formatCount = 0;
    mutable StatusBarStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
