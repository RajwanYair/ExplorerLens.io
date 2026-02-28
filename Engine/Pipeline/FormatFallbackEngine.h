#pragma once
// Format Fallback Intelligence Engine
// Ranked fallback chain, trigger conditions, per-format fallback map, telemetry.

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens::Pipeline {

// ─── Fallback trigger ────────────────────────────────────────────────────────

enum class FallbackTrigger : uint32_t {
 None = 0x00,
 DecodeFailed = 0x01,
 Timeout = 0x02,
 MemoryExceeded = 0x04,
 GPUInitFailed = 0x08,
 PluginMissing = 0x10,
 UnsupportedFile = 0x20,
 CorruptData = 0x40,
};

inline FallbackTrigger operator|(FallbackTrigger a, FallbackTrigger b) {
 return static_cast<FallbackTrigger>(
 static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool HasTrigger(FallbackTrigger set, FallbackTrigger flag) {
 return (static_cast<uint32_t>(set) & static_cast<uint32_t>(flag)) != 0;
}

inline std::string ToString(FallbackTrigger t) {
 std::string s;
 if (HasTrigger(t, FallbackTrigger::DecodeFailed)) s += "DecodeFailed|";
 if (HasTrigger(t, FallbackTrigger::Timeout)) s += "Timeout|";
 if (HasTrigger(t, FallbackTrigger::MemoryExceeded)) s += "MemoryExceeded|";
 if (HasTrigger(t, FallbackTrigger::GPUInitFailed)) s += "GPUInitFailed|";
 if (HasTrigger(t, FallbackTrigger::PluginMissing)) s += "PluginMissing|";
 if (HasTrigger(t, FallbackTrigger::UnsupportedFile)) s += "UnsupportedFile|";
 if (HasTrigger(t, FallbackTrigger::CorruptData)) s += "CorruptData|";
 if (!s.empty() && s.back() == '|') s.pop_back();
 return s.empty() ? "None" : s;
}

// ─── Decoder stage in fallback chain ─────────────────────────────────────────

struct FallbackStage {
 std::string decoderName; // e.g., "JXLDecoder"
 uint32_t qualityScore { 100 }; // 0-100 (higher = better)
 uint32_t speedScore { 50 }; // 0-100
 FallbackTrigger activateOnTriggers { FallbackTrigger::None };
 bool isTerminal { false }; // badge thumbnail — last resort

 bool ShouldActivate(FallbackTrigger trigger) const {
 if (activateOnTriggers == FallbackTrigger::None) return false;
 return HasTrigger(trigger, FallbackTrigger::DecodeFailed) ||
 HasTrigger(trigger, activateOnTriggers);
 }
};

// ─── Per-format fallback chain ────────────────────────────────────────────────

struct FormatFallbackChain {
 std::string extension;
 std::vector<FallbackStage> stages; // ordered: primary → secondary → ... → badge

 const FallbackStage* SelectForTrigger(FallbackTrigger trigger) const {
 for (const auto& stage : stages)
 if (stage.ShouldActivate(trigger) || stage.activateOnTriggers == FallbackTrigger::None)
 return &stage;
 return stages.empty() ? nullptr : &stages.back();
 }

 bool HasTerminal() const {
 for (const auto& s : stages)
 if (s.isTerminal) return true;
 return false;
 }
};

// ─── Fallback event for telemetry ────────────────────────────────────────────

struct FallbackEvent {
 std::string extension;
 std::string primaryDecoder;
 std::string usedDecoder;
 FallbackTrigger trigger;
 uint32_t stagesTraversed { 0 };
 double totalDecodeMs { 0.0 };

 std::string ToLogLine() const {
 return "[Fallback] " + extension + ": " + primaryDecoder + " → " +
 usedDecoder + " trigger=" + ToString(trigger) +
 " stages=" + std::to_string(stagesTraversed);
 }
};

// ─── Fallback engine ──────────────────────────────────────────────────────────

struct FormatFallbackEngine {
 std::vector<FormatFallbackChain> chains;
 bool enableTelemetry { true };
 std::function<void(FallbackEvent)> onFallback;

 const FormatFallbackChain* FindChain(const std::string& ext) const {
 for (const auto& c : chains)
 if (c.extension == ext) return &c;
 return nullptr;
 }

 static FormatFallbackEngine CreateDefault() {
 FormatFallbackEngine e;
 // JXL fallback chain
 e.chains.push_back({
 ".jxl",
 {
 { "JXLDecoder", 100, 70, FallbackTrigger::None, false },
 { "WICDecoder", 60, 80, FallbackTrigger::DecodeFailed, false },
 { "BadgeThumbnail", 0, 100, FallbackTrigger::MemoryExceeded | FallbackTrigger::Timeout, true },
 }
 });
 // HEIF fallback chain
 e.chains.push_back({
 ".heic",
 {
 { "HEIFDecoder", 100, 60, FallbackTrigger::None, false },
 { "WICDecoder", 55, 80, FallbackTrigger::DecodeFailed, false },
 { "BadgeThumbnail", 0, 100, FallbackTrigger::PluginMissing, true },
 }
 });
 // RAW camera fallback chain
 e.chains.push_back({
 ".raw",
 {
 { "RAWDecoder(LibRaw)", 100, 40, FallbackTrigger::None, false },
 { "WICDecoder", 50, 80, FallbackTrigger::DecodeFailed,false },
 { "BadgeThumbnail", 0, 100, FallbackTrigger::MemoryExceeded, true },
 }
 });
 // CAD fallback chain
 e.chains.push_back({
 ".dwg",
 {
 { "CADPlugin", 100, 30, FallbackTrigger::None, false },
 { "BadgeThumbnail", 0, 100, FallbackTrigger::PluginMissing, true },
 }
 });
 return e;
 }
};

} // namespace ExplorerLens::Pipeline

