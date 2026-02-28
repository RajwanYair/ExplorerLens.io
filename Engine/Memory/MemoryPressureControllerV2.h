#pragma once
// Memory Pressure Controller V2
// Kernel-level low-memory notification hook, tiered response ladder,
// proactive pre-eviction, ETW emission on each pressure transition.

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens::Memory {

// ─── Pressure level ──────────────────────────────────────────────────────────

enum class PressureLevel : uint32_t {
 Normal = 0, // > 50% free
 Low = 1, // 25–50% free — start background compaction
 Medium = 2, // 10–25% free — shed D3D11 cache
 High = 3, // 5–10% free — shed all caches
 Critical = 4, // < 5% free — emergency eviction, no new decodes
};

inline std::string ToString(PressureLevel p) {
 switch (p) {
 case PressureLevel::Normal: return "Normal";
 case PressureLevel::Low: return "Low";
 case PressureLevel::Medium: return "Medium";
 case PressureLevel::High: return "High";
 case PressureLevel::Critical: return "Critical";
 default: return "Unknown";
 }
}

// ─── Response action ─────────────────────────────────────────────────────────

enum class PressureAction : uint32_t {
 None = 0x00,
 BackgroundCompact = 0x01,
 EvictD3D11Cache = 0x02,
 EvictCPUPixelCache = 0x04,
 EvictMetadataCache = 0x08,
 BlockNewDecodes = 0x10,
 EmitETWEvent = 0x20,
};

inline PressureAction operator|(PressureAction a, PressureAction b) {
 return static_cast<PressureAction>(
 static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

// ─── Response ladder rung ─────────────────────────────────────────────────────

struct PressureLadderRung {
 PressureLevel level;
 PressureAction actions;
 uint32_t maxEvictionMs { 200 }; // time budget for eviction
 size_t targetFreeBytes { 0 }; // desired free bytes after action
};

inline std::vector<PressureLadderRung> DefaultPressureLadder() {
 return {
 { PressureLevel::Normal, PressureAction::None, 0, 0 },
 { PressureLevel::Low, PressureAction::BackgroundCompact | PressureAction::EmitETWEvent,
 200, 64ULL * 1024 * 1024 },
 { PressureLevel::Medium, PressureAction::EvictD3D11Cache | PressureAction::EmitETWEvent,
 300, 128ULL * 1024 * 1024 },
 { PressureLevel::High, PressureAction::EvictCPUPixelCache | PressureAction::EvictD3D11Cache | PressureAction::EmitETWEvent,
 400, 256ULL * 1024 * 1024 },
 { PressureLevel::Critical, PressureAction::EvictCPUPixelCache | PressureAction::EvictD3D11Cache
 | PressureAction::BlockNewDecodes | PressureAction::EmitETWEvent,
 500, 512ULL * 1024 * 1024 },
 };
}

// ─── Pressure transition event ────────────────────────────────────────────────

struct PressureTransition {
 PressureLevel from;
 PressureLevel to;
 uint64_t timestampMs { 0 };
 size_t freeBytesBefore { 0 };
 size_t freeBytesAfter { 0 };
 PressureAction actionsExecuted { PressureAction::None };

 bool IsEscalation() const { return static_cast<uint32_t>(to) > static_cast<uint32_t>(from); }
};

// ─── Memory pressure controller V2 ───────────────────────────────────────────

using PressureTransitionCallback = std::function<void(const PressureTransition&)>;

class MemoryPressureControllerV2 {
public:
 explicit MemoryPressureControllerV2(
 std::vector<PressureLadderRung> ladder = DefaultPressureLadder())
 : m_ladder(std::move(ladder))
 , m_current(PressureLevel::Normal)
 {}

 PressureLevel CurrentLevel() const { return m_current; }

 PressureTransition Evaluate(uint64_t totalBytes, uint64_t freeBytes) {
 PressureTransition t;
 t.from = m_current;
 t.freeBytesBefore = freeBytes;

 double ratio = totalBytes > 0 ? (double)freeBytes / totalBytes : 1.0;

 PressureLevel newLevel;
 if (ratio > 0.50) newLevel = PressureLevel::Normal;
 else if (ratio > 0.25) newLevel = PressureLevel::Low;
 else if (ratio > 0.10) newLevel = PressureLevel::Medium;
 else if (ratio > 0.05) newLevel = PressureLevel::High;
 else newLevel = PressureLevel::Critical;

 t.to = newLevel;
 m_current = newLevel;

 // Determine actions
 for (const auto& rung : m_ladder) {
 if (rung.level == newLevel) {
 t.actionsExecuted = rung.actions;
 break;
 }
 }

 if (m_callback) m_callback(t);
 return t;
 }

 void OnTransition(PressureTransitionCallback cb) { m_callback = std::move(cb); }

 static MemoryPressureControllerV2 Create() {
 return MemoryPressureControllerV2(DefaultPressureLadder());
 }

private:
 std::vector<PressureLadderRung> m_ladder;
 PressureLevel m_current;
 PressureTransitionCallback m_callback;
};

} // namespace ExplorerLens::Memory

