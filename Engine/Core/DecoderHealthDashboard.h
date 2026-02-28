#pragma once
// Decoder Health Dashboard
// Live status panel model with circuit breaker visualization.
// Provides data model for real-time decoder health monitoring UI.

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>

namespace ExplorerLens::Core {

// ─── Circuit breaker state ──────────────────────────────────────
enum class CircuitState : uint8_t {
 Closed, // Normal operation
 Open, // Tripped — decoder disabled
 HalfOpen // Testing recovery
};

inline const char* CircuitStateName(CircuitState s) {
 switch (s) {
 case CircuitState::Closed: return "Closed";
 case CircuitState::Open: return "Open";
 case CircuitState::HalfOpen: return "HalfOpen";
 default: return "Unknown";
 }
}

// ─── Health status ──────────────────────────────────────────────
enum class HealthStatus : uint8_t {
 Healthy,
 Degraded,
 Unhealthy,
 Disabled,
 Unknown
};

inline const char* HealthStatusName(HealthStatus h) {
 switch (h) {
 case HealthStatus::Healthy: return "Healthy";
 case HealthStatus::Degraded: return "Degraded";
 case HealthStatus::Unhealthy: return "Unhealthy";
 case HealthStatus::Disabled: return "Disabled";
 case HealthStatus::Unknown: return "Unknown";
 default: return "???";
 }
}

// ─── Decoder metrics ────────────────────────────────────────────
struct DecoderMetrics {
 uint64_t totalDecodes = 0;
 uint64_t successfulDecodes = 0;
 uint64_t failedDecodes = 0;
 uint64_t totalTimeMs = 0;
 uint64_t peakTimeMs = 0;
 size_t peakMemoryBytes = 0;

 double SuccessRate() const {
 if (totalDecodes == 0) return 0.0;
 return static_cast<double>(successfulDecodes) / static_cast<double>(totalDecodes) * 100.0;
 }

 double AverageTimeMs() const {
 if (successfulDecodes == 0) return 0.0;
 return static_cast<double>(totalTimeMs) / static_cast<double>(successfulDecodes);
 }
};

// ─── Individual decoder health entry ────────────────────────────
struct DecoderHealthEntry {
 std::string decoderName;
 HealthStatus health = HealthStatus::Unknown;
 CircuitState circuit = CircuitState::Closed;
 DecoderMetrics metrics;
 uint32_t consecutiveFailures = 0;
 uint32_t circuitTripCount = 0;
 uint64_t lastActivityMs = 0;
 std::vector<std::string> extensions;

 bool IsOperational() const {
 return health != HealthStatus::Disabled &&
 health != HealthStatus::Unhealthy &&
 circuit != CircuitState::Open;
 }
};

// ─── Dashboard configuration ────────────────────────────────────
struct DashboardConfig {
 uint32_t refreshIntervalMs = 1000;
 uint32_t failureThreshold = 5; // consecutive failures to trip circuit
 double degradedThresholdPct = 90.0; // below this = degraded
 double unhealthyThresholdPct = 50.0; // below this = unhealthy
 uint32_t circuitResetTimeMs = 30000; // time before half-open retry
 bool showMetrics = true;
 bool showExtensions = true;

 static DashboardConfig Default() { return {}; }
 static DashboardConfig Compact() {
 DashboardConfig c;
 c.showMetrics = false;
 c.showExtensions = false;
 c.refreshIntervalMs = 5000;
 return c;
 }
};

// ─── Dashboard aggregate stats ──────────────────────────────────
struct DashboardStats {
 size_t totalDecoders = 0;
 size_t healthyCount = 0;
 size_t degradedCount = 0;
 size_t unhealthyCount = 0;
 size_t disabledCount = 0;
 size_t openCircuits = 0;
 uint64_t totalDecodes = 0;
 double overallSuccessRate = 0.0;

 double HealthPercentage() const {
 if (totalDecoders == 0) return 0.0;
 return static_cast<double>(healthyCount) / static_cast<double>(totalDecoders) * 100.0;
 }
};

// ─── Health dashboard manager ───────────────────────────────────
class DecoderHealthDashboard {
public:
 static DecoderHealthDashboard Create(const DashboardConfig& config = DashboardConfig::Default()) {
 DecoderHealthDashboard d;
 d.m_config = config;
 return d;
 }

 // Register a decoder
 void RegisterDecoder(const std::string& name, const std::vector<std::string>& extensions = {}) {
 for (auto& e : m_entries) {
 if (e.decoderName == name) return;
 }
 DecoderHealthEntry entry;
 entry.decoderName = name;
 entry.health = HealthStatus::Healthy;
 entry.extensions = extensions;
 m_entries.push_back(std::move(entry));
 }

 // Record a decode result
 void RecordDecode(const std::string& name, bool success, uint64_t timeMs, size_t memBytes = 0) {
 auto* e = FindEntry(name);
 if (!e) return;

 e->metrics.totalDecodes++;
 e->metrics.totalTimeMs += timeMs;
 e->lastActivityMs = timeMs;

 if (success) {
 e->metrics.successfulDecodes++;
 e->consecutiveFailures = 0;
 if (e->circuit == CircuitState::HalfOpen) {
 e->circuit = CircuitState::Closed;
 }
 } else {
 e->metrics.failedDecodes++;
 e->consecutiveFailures++;
 if (e->consecutiveFailures >= m_config.failureThreshold) {
 e->circuit = CircuitState::Open;
 e->circuitTripCount++;
 }
 }

 if (timeMs > e->metrics.peakTimeMs) e->metrics.peakTimeMs = timeMs;
 if (memBytes > e->metrics.peakMemoryBytes) e->metrics.peakMemoryBytes = memBytes;

 // Update health status
 UpdateHealth(*e);
 }

 // Get aggregate stats
 DashboardStats GetStats() const {
 DashboardStats stats;
 stats.totalDecoders = m_entries.size();
 for (const auto& e : m_entries) {
 switch (e.health) {
 case HealthStatus::Healthy: stats.healthyCount++; break;
 case HealthStatus::Degraded: stats.degradedCount++; break;
 case HealthStatus::Unhealthy: stats.unhealthyCount++; break;
 case HealthStatus::Disabled: stats.disabledCount++; break;
 default: break;
 }
 if (e.circuit == CircuitState::Open) stats.openCircuits++;
 stats.totalDecodes += e.metrics.totalDecodes;
 }

 uint64_t totalSuccess = 0;
 for (const auto& e : m_entries) totalSuccess += e.metrics.successfulDecodes;
 stats.overallSuccessRate = (stats.totalDecodes > 0)
 ? static_cast<double>(totalSuccess) / static_cast<double>(stats.totalDecodes) * 100.0
 : 0.0;

 return stats;
 }

 // Get entry by name
 const DecoderHealthEntry* GetEntry(const std::string& name) const {
 for (const auto& e : m_entries)
 if (e.decoderName == name) return &e;
 return nullptr;
 }

 // Get all entries
 const std::vector<DecoderHealthEntry>& Entries() const { return m_entries; }
 size_t DecoderCount() const { return m_entries.size(); }

 // Attempt half-open recovery
 bool AttemptRecovery(const std::string& name) {
 auto* e = FindEntry(name);
 if (!e || e->circuit != CircuitState::Open) return false;
 e->circuit = CircuitState::HalfOpen;
 return true;
 }

 // Disable a decoder
 void DisableDecoder(const std::string& name) {
 auto* e = FindEntry(name);
 if (e) { e->health = HealthStatus::Disabled; e->circuit = CircuitState::Open; }
 }

 std::string Summary() const {
 auto stats = GetStats();
 std::string s = "DecoderHealth: " + std::to_string(stats.totalDecoders) + " decoders";
 s += " [H:" + std::to_string(stats.healthyCount);
 s += " D:" + std::to_string(stats.degradedCount);
 s += " U:" + std::to_string(stats.unhealthyCount);
 s += " X:" + std::to_string(stats.disabledCount) + "]";
 s += " circuits_open=" + std::to_string(stats.openCircuits);
 return s;
 }

private:
 DashboardConfig m_config;
 std::vector<DecoderHealthEntry> m_entries;

 DecoderHealthEntry* FindEntry(const std::string& name) {
 for (auto& e : m_entries)
 if (e.decoderName == name) return &e;
 return nullptr;
 }

 void UpdateHealth(DecoderHealthEntry& entry) {
 if (entry.health == HealthStatus::Disabled) return;
 double rate = entry.metrics.SuccessRate();
 if (rate >= m_config.degradedThresholdPct)
 entry.health = HealthStatus::Healthy;
 else if (rate >= m_config.unhealthyThresholdPct)
 entry.health = HealthStatus::Degraded;
 else
 entry.health = HealthStatus::Unhealthy;
 }
};

} // namespace ExplorerLens::Core

