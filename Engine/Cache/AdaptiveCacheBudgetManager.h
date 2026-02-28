#pragma once
// Adaptive Cache Budget Manager
// Runtime memory sensing, per-tier budget allocation, rebalancing triggers.
// Supports D3D11 texture cache, CPU pixel cache, and archive metadata cache.

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens::Cache {

// ─── Cache tier ──────────────────────────────────────────────────────────────

enum class CacheTier : uint32_t {
 D3D11Texture = 0, // GPU-resident texture cache
 CPUPixel = 1, // decoded BGRA pixel buffers in RAM
 ArchiveMetadata = 2, // directory listings, file headers
 Thumbnail = 3, // finalized WIC/DPI-scaled thumbnails
};

inline std::string ToString(CacheTier t) {
 switch (t) {
 case CacheTier::D3D11Texture: return "D3D11Texture";
 case CacheTier::CPUPixel: return "CPUPixel";
 case CacheTier::ArchiveMetadata: return "ArchiveMetadata";
 case CacheTier::Thumbnail: return "Thumbnail";
 default: return "Unknown";
 }
}

// ─── Per-tier budget ─────────────────────────────────────────────────────────

struct TierBudget {
 CacheTier tier;
 size_t softLimitBytes { 0 }; // start evicting
 size_t hardLimitBytes { 0 }; // block new entries
 double weightFactor { 1.0 }; // relative priority in rebalancing
};

// ─── System memory state ─────────────────────────────────────────────────────

enum class MemoryPressureLevel : uint32_t {
 Normal = 0, // > 50% free physical RAM
 Moderate = 1, // 25–50% free
 High = 2, // 10–25% free
 Critical = 3, // < 10% free — shed non-essential caches
};

struct SystemMemorySnapshot {
 uint64_t totalPhysicalBytes { 0 };
 uint64_t availableBytes { 0 };
 uint64_t processWorkingSet { 0 };

 MemoryPressureLevel PressureLevel() const {
 if (totalPhysicalBytes == 0) return MemoryPressureLevel::Normal;
 double freeRatio = (double)availableBytes / totalPhysicalBytes;
 if (freeRatio > 0.50) return MemoryPressureLevel::Normal;
 if (freeRatio > 0.25) return MemoryPressureLevel::Moderate;
 if (freeRatio > 0.10) return MemoryPressureLevel::High;
 return MemoryPressureLevel::Critical;
 }
};

// ─── Rebalance result ────────────────────────────────────────────────────────

struct RebalanceResult {
 bool triggered { false };
 MemoryPressureLevel reason { MemoryPressureLevel::Normal };
 std::vector<TierBudget> newBudgets;
 size_t totalBudget { 0 };
 double rebalanceMs { 0.0 };
};

// ─── Adaptive cache budget manager ───────────────────────────────────────────

class AdaptiveCacheBudgetManager {
public:
 static constexpr size_t kDefaultTotalBudget = 512ULL * 1024 * 1024; // 512 MB

 explicit AdaptiveCacheBudgetManager(size_t totalBudget = kDefaultTotalBudget)
 : m_totalBudget(totalBudget)
 {
 m_budgets = CreateDefaultBudgets(totalBudget);
 }

 static std::vector<TierBudget> CreateDefaultBudgets(size_t total) {
 return {
 { CacheTier::D3D11Texture, total * 4 / 10, total * 5 / 10, 1.5 },
 { CacheTier::CPUPixel, total * 3 / 10, total * 4 / 10, 1.0 },
 { CacheTier::ArchiveMetadata, total * 1 / 10, total * 15 / 100, 0.5 },
 { CacheTier::Thumbnail, total * 2 / 10, total * 3 / 10, 0.8 },
 };
 }

 RebalanceResult Rebalance(const SystemMemorySnapshot& snapshot) {
 RebalanceResult result;
 result.reason = snapshot.PressureLevel();

 switch (result.reason) {
 case MemoryPressureLevel::Critical:
 result.triggered = true;
 result.newBudgets = CreateDefaultBudgets(m_totalBudget / 4);
 result.totalBudget = m_totalBudget / 4;
 break;
 case MemoryPressureLevel::High:
 result.triggered = true;
 result.newBudgets = CreateDefaultBudgets(m_totalBudget / 2);
 result.totalBudget = m_totalBudget / 2;
 break;
 case MemoryPressureLevel::Moderate:
 result.triggered = true;
 result.newBudgets = CreateDefaultBudgets(m_totalBudget * 3 / 4);
 result.totalBudget = m_totalBudget * 3 / 4;
 break;
 default:
 result.triggered = false;
 result.newBudgets = m_budgets;
 result.totalBudget = m_totalBudget;
 break;
 }

 if (result.triggered) {
 m_budgets = result.newBudgets;
 result.rebalanceMs = 0.5;
 }
 return result;
 }

 const std::vector<TierBudget>& CurrentBudgets() const { return m_budgets; }
 size_t TotalBudget() const { return m_totalBudget; }

private:
 size_t m_totalBudget;
 std::vector<TierBudget> m_budgets;
};

} // namespace ExplorerLens::Cache

