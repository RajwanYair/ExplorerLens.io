#pragma once
/******************************************************************************
 * AdaptiveCacheBudgetManager.h
 * Copyright (c) 2026 ExplorerLens Project
 *
 * PURPOSE:
 *   Dynamically adjusts cache memory budget based on real system memory
 *   pressure queried via GlobalMemoryStatusEx. Implements a 4-tier pressure
 *   model (Normal/Moderate/High/Critical) with per-tier budget allocation
 *   across D3D11 texture, CPU pixel, archive metadata, and thumbnail caches.
 *
 * CLASSES:
 *   AdaptiveCacheBudgetManager — Main budget controller with background
 *     auto-update thread, hysteresis (3 consecutive checks before tier
 *     change), budget change notifications, and SRWLOCK thread safety.
 *
 * INPUTS:
 *   Base budget (bytes), optional auto-update interval (ms).
 *
 * OUTPUTS:
 *   Current effective budget, pressure tier, per-tier allocations,
 *   adaptation statistics via BudgetStats.
 *
 * THREAD SAFETY:
 *   All public methods are thread-safe via SRWLOCK.
 *****************************************************************************/

#include <windows.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ExplorerLens {
namespace Cache {

// ─── Cache tier ──────────────────────────────────────────────────────────────

enum class CacheTier : uint32_t {
    D3D11Texture = 0,
    CPUPixel = 1,
    ArchiveMetadata = 2,
    Thumbnail = 3,
};

inline std::string ToString(CacheTier t)
{
    switch (t) {
        case CacheTier::D3D11Texture:
            return "D3D11Texture";
        case CacheTier::CPUPixel:
            return "CPUPixel";
        case CacheTier::ArchiveMetadata:
            return "ArchiveMetadata";
        case CacheTier::Thumbnail:
            return "Thumbnail";
        default:
            return "Unknown";
    }
}

// ─── Per-tier budget ─────────────────────────────────────────────────────────

struct TierBudget
{
    CacheTier tier;
    size_t softLimitBytes{0};
    size_t hardLimitBytes{0};
    double weightFactor{1.0};
};

// ─── Memory pressure level (4-tier, backward-compatible) ─────────────────────

enum class MemoryPressureLevel : uint32_t {
    Normal = 0,    // > 50% free physical RAM
    Moderate = 1,  // 25–50% free
    High = 2,      // 10–25% free
    Critical = 3,  // < 10% free — shed non-essential caches
};

inline std::string ToString(MemoryPressureLevel p)
{
    switch (p) {
        case MemoryPressureLevel::Normal:
            return "Normal";
        case MemoryPressureLevel::Moderate:
            return "Moderate";
        case MemoryPressureLevel::High:
            return "High";
        case MemoryPressureLevel::Critical:
            return "Critical";
        default:
            return "Unknown";
    }
}

// ─── 5-tier pressure model for fine-grained budget control ───────────────────

enum class MemoryPressureTier : uint32_t {
    None = 0,      // < 50% usage
    Low = 1,       // 50–70% usage
    Medium = 2,    // 70–85% usage
    High = 3,      // 85–95% usage
    Critical = 4,  // > 95% usage
};

inline std::string ToString(MemoryPressureTier t)
{
    switch (t) {
        case MemoryPressureTier::None:
            return "None";
        case MemoryPressureTier::Low:
            return "Low";
        case MemoryPressureTier::Medium:
            return "Medium";
        case MemoryPressureTier::High:
            return "High";
        case MemoryPressureTier::Critical:
            return "Critical";
        default:
            return "Unknown";
    }
}

// ─── System memory snapshot ──────────────────────────────────────────────────

struct SystemMemorySnapshot
{
    uint64_t totalPhysicalBytes{0};
    uint64_t availableBytes{0};
    uint64_t processWorkingSet{0};
    uint64_t totalVirtualBytes{0};
    uint64_t availableVirtualBytes{0};
    uint32_t memoryLoadPercent{0};

    MemoryPressureLevel PressureLevel() const
    {
        if (totalPhysicalBytes == 0)
            return MemoryPressureLevel::Normal;
        double freeRatio = static_cast<double>(availableBytes) / static_cast<double>(totalPhysicalBytes);
        if (freeRatio > 0.50)
            return MemoryPressureLevel::Normal;
        if (freeRatio > 0.25)
            return MemoryPressureLevel::Moderate;
        if (freeRatio > 0.10)
            return MemoryPressureLevel::High;
        return MemoryPressureLevel::Critical;
    }

    MemoryPressureTier PressureTier() const
    {
        if (totalPhysicalBytes == 0)
            return MemoryPressureTier::None;
        double usagePercent =
            100.0 * (1.0 - static_cast<double>(availableBytes) / static_cast<double>(totalPhysicalBytes));
        if (usagePercent < 50.0)
            return MemoryPressureTier::None;
        if (usagePercent < 70.0)
            return MemoryPressureTier::Low;
        if (usagePercent < 85.0)
            return MemoryPressureTier::Medium;
        if (usagePercent < 95.0)
            return MemoryPressureTier::High;
        return MemoryPressureTier::Critical;
    }

    static SystemMemorySnapshot QuerySystem()
    {
        SystemMemorySnapshot snap;
        MEMORYSTATUSEX msx{};
        msx.dwLength = sizeof(msx);
        if (::GlobalMemoryStatusEx(&msx)) {
            snap.totalPhysicalBytes = msx.ullTotalPhys;
            snap.availableBytes = msx.ullAvailPhys;
            snap.totalVirtualBytes = msx.ullTotalVirtual;
            snap.availableVirtualBytes = msx.ullAvailVirtual;
            snap.memoryLoadPercent = msx.dwMemoryLoad;
        }
        return snap;
    }
};

// ─── Rebalance result ────────────────────────────────────────────────────────

struct RebalanceResult
{
    bool triggered{false};
    MemoryPressureLevel reason{MemoryPressureLevel::Normal};
    std::vector<TierBudget> newBudgets;
    size_t totalBudget{0};
    double rebalanceMs{0.0};
};

// ─── Budget statistics ───────────────────────────────────────────────────────

struct BudgetStats
{
    MemoryPressureTier currentTier{MemoryPressureTier::None};
    size_t currentBudget{0};
    uint64_t physicalTotal{0};
    uint64_t physicalAvailable{0};
    uint64_t virtualTotal{0};
    uint64_t virtualAvailable{0};
    uint32_t adaptationCount{0};
    uint64_t timeInNoneMs{0};
    uint64_t timeInLowMs{0};
    uint64_t timeInMediumMs{0};
    uint64_t timeInHighMs{0};
    uint64_t timeInCriticalMs{0};
};

// ─── Adaptive cache budget manager ───────────────────────────────────────────

class AdaptiveCacheBudgetManager
{
  public:
    static constexpr size_t kDefaultTotalBudget = 512ULL * 1024 * 1024;

    explicit AdaptiveCacheBudgetManager(size_t totalBudget = kDefaultTotalBudget)
        : m_baseBudget(totalBudget)
        , m_currentBudget(totalBudget)
        , m_currentTier(MemoryPressureTier::None)
        , m_autoUpdateRunning(false)
        , m_hysteresisCount(0)
        , m_pendingTier(MemoryPressureTier::None)
        , m_adaptationCount(0)
    {
        ::InitializeSRWLock(&m_srwLock);
        m_budgets = CreateDefaultBudgets(totalBudget);
        m_tierEntryTime = std::chrono::steady_clock::now();
        for (auto& t : m_tierDurations)
            t = std::chrono::milliseconds(0);
    }

    ~AdaptiveCacheBudgetManager()
    {
        DisableAutoUpdate();
    }

    AdaptiveCacheBudgetManager(const AdaptiveCacheBudgetManager&) = delete;
    AdaptiveCacheBudgetManager& operator=(const AdaptiveCacheBudgetManager&) = delete;

    // ── Static helpers ──────────────────────────────────────────────────────

    static std::vector<TierBudget> CreateDefaultBudgets(size_t total)
    {
        return {
            {CacheTier::D3D11Texture, total * 4 / 10, total * 5 / 10, 1.5},
            {CacheTier::CPUPixel, total * 3 / 10, total * 4 / 10, 1.0},
            {CacheTier::ArchiveMetadata, total * 1 / 10, total * 15 / 100, 0.5},
            {CacheTier::Thumbnail, total * 2 / 10, total * 3 / 10, 0.8},
        };
    }

    // ── Backward-compatible rebalance (4-tier) ──────────────────────────────

    RebalanceResult Rebalance(const SystemMemorySnapshot& snapshot)
    {
        ::AcquireSRWLockExclusive(&m_srwLock);

        auto start = std::chrono::steady_clock::now();
        RebalanceResult result;
        result.reason = snapshot.PressureLevel();

        switch (result.reason) {
            case MemoryPressureLevel::Critical:
                result.triggered = true;
                result.newBudgets = CreateDefaultBudgets(m_baseBudget / 4);
                result.totalBudget = m_baseBudget / 4;
                break;
            case MemoryPressureLevel::High:
                result.triggered = true;
                result.newBudgets = CreateDefaultBudgets(m_baseBudget / 2);
                result.totalBudget = m_baseBudget / 2;
                break;
            case MemoryPressureLevel::Moderate:
                result.triggered = true;
                result.newBudgets = CreateDefaultBudgets(m_baseBudget * 3 / 4);
                result.totalBudget = m_baseBudget * 3 / 4;
                break;
            default:
                result.triggered = false;
                result.newBudgets = m_budgets;
                result.totalBudget = m_baseBudget;
                break;
        }

        if (result.triggered) {
            m_budgets = result.newBudgets;
            m_currentBudget = result.totalBudget;
            auto elapsed = std::chrono::steady_clock::now() - start;
            result.rebalanceMs = std::chrono::duration<double, std::milli>(elapsed).count();
        }

        ::ReleaseSRWLockExclusive(&m_srwLock);
        return result;
    }

    // ── 5-tier budget control ───────────────────────────────────────────────

    size_t GetCurrentBudget() const
    {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        size_t b = m_currentBudget;
        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return b;
    }

    void SetBaseBudget(size_t bytes)
    {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_baseBudget = bytes;
        RecalculateBudgetLocked();
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void Update()
    {
        auto snapshot = SystemMemorySnapshot::QuerySystem();
        MemoryPressureTier newTier = snapshot.PressureTier();

        ::AcquireSRWLockExclusive(&m_srwLock);

        // Accumulate time spent in current tier
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tierEntryTime);
        m_tierDurations[static_cast<size_t>(m_currentTier)] += elapsed;
        m_tierEntryTime = now;

        // Hysteresis: require 3 consecutive checks at new tier before switching
        if (newTier != m_currentTier) {
            if (newTier == m_pendingTier) {
                m_hysteresisCount++;
            } else {
                m_pendingTier = newTier;
                m_hysteresisCount = 1;
            }
            if (m_hysteresisCount >= 3) {
                MemoryPressureTier oldTier = m_currentTier;
                m_currentTier = newTier;
                m_hysteresisCount = 0;
                m_adaptationCount++;
                RecalculateBudgetLocked();
                m_lastSnapshot = snapshot;

                // Notify callback outside lock would be ideal, but we copy first
                auto cb = m_budgetChangedCallback;
                size_t budget = m_currentBudget;
                MemoryPressureTier tier = m_currentTier;
                ::ReleaseSRWLockExclusive(&m_srwLock);
                if (cb)
                    cb(budget, tier);
                return;
            }
        } else {
            m_hysteresisCount = 0;
            m_pendingTier = m_currentTier;
        }
        m_lastSnapshot = snapshot;

        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void EnableAutoUpdate(uint32_t intervalMs = 5000)
    {
        if (m_autoUpdateRunning.exchange(true))
            return;
        m_autoUpdateThread = std::thread([this, intervalMs]() {
            while (m_autoUpdateRunning.load()) {
                Update();
                for (uint32_t waited = 0; waited < intervalMs && m_autoUpdateRunning.load(); waited += 100) {
                    ::Sleep(100);
                }
            }
        });
    }

    void DisableAutoUpdate()
    {
        if (m_autoUpdateRunning.exchange(false)) {
            if (m_autoUpdateThread.joinable()) {
                m_autoUpdateThread.join();
            }
        }
    }

    void OnBudgetChanged(std::function<void(size_t newBudget, MemoryPressureTier tier)> fn)
    {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_budgetChangedCallback = std::move(fn);
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    BudgetStats GetStats() const
    {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        BudgetStats stats;
        stats.currentTier = m_currentTier;
        stats.currentBudget = m_currentBudget;
        stats.physicalTotal = m_lastSnapshot.totalPhysicalBytes;
        stats.physicalAvailable = m_lastSnapshot.availableBytes;
        stats.virtualTotal = m_lastSnapshot.totalVirtualBytes;
        stats.virtualAvailable = m_lastSnapshot.availableVirtualBytes;
        stats.adaptationCount = m_adaptationCount;
        stats.timeInNoneMs = static_cast<uint64_t>(m_tierDurations[0].count());
        stats.timeInLowMs = static_cast<uint64_t>(m_tierDurations[1].count());
        stats.timeInMediumMs = static_cast<uint64_t>(m_tierDurations[2].count());
        stats.timeInHighMs = static_cast<uint64_t>(m_tierDurations[3].count());
        stats.timeInCriticalMs = static_cast<uint64_t>(m_tierDurations[4].count());
        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return stats;
    }

    // ── Legacy accessors (backward-compatible) ──────────────────────────────

    const std::vector<TierBudget>& CurrentBudgets() const
    {
        return m_budgets;
    }
    size_t TotalBudget() const
    {
        return m_baseBudget;
    }

  private:
    void RecalculateBudgetLocked()
    {
        double shrinkFactor = 1.0;
        switch (m_currentTier) {
            case MemoryPressureTier::None:
                shrinkFactor = 1.0;
                break;
            case MemoryPressureTier::Low:
                shrinkFactor = 0.75;
                break;  // shrink by 25%
            case MemoryPressureTier::Medium:
                shrinkFactor = 0.50;
                break;  // shrink by 50%
            case MemoryPressureTier::High:
                shrinkFactor = 0.25;
                break;  // shrink by 75%
            case MemoryPressureTier::Critical:
                shrinkFactor = 0.10;
                break;  // shrink by 90%
        }
        m_currentBudget = static_cast<size_t>(static_cast<double>(m_baseBudget) * shrinkFactor);
        m_budgets = CreateDefaultBudgets(m_currentBudget);
    }

    SRWLOCK m_srwLock{};
    size_t m_baseBudget;
    size_t m_currentBudget;
    std::vector<TierBudget> m_budgets;

    MemoryPressureTier m_currentTier;
    MemoryPressureTier m_pendingTier;
    uint32_t m_hysteresisCount;
    uint32_t m_adaptationCount;

    SystemMemorySnapshot m_lastSnapshot{};
    std::chrono::steady_clock::time_point m_tierEntryTime;
    std::chrono::milliseconds m_tierDurations[5]{};

    std::atomic<bool> m_autoUpdateRunning;
    std::thread m_autoUpdateThread;
    std::function<void(size_t, MemoryPressureTier)> m_budgetChangedCallback;
};

}  // namespace Cache
}  // namespace ExplorerLens
