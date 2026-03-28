// FleetHealthDashboard.h — Aggregated Fleet Health Metrics Endpoint
// Copyright (c) 2026 ExplorerLens Project
//
// Collects per-machine health signals (decode latency, cache hit rate, GPU errors,
// AI model load status, memory pressure) and exposes them as a JSON health document
// for Azure Monitor, Prometheus scrape, or Intune device health reporting.
//
#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <sstream>
#include <ctime>

namespace ExplorerLens { namespace Engine { namespace Enterprise {

enum class FleetHealthLevel : uint8_t {
    Healthy   = 0,
    Degraded  = 1,
    Critical  = 2,
    Unknown   = 3
};

struct FleetSubsystemHealth {
    std::string  name;
    FleetHealthLevel  level         = FleetHealthLevel::Healthy;
    std::string  statusMessage;
    uint64_t     totalOps      = 0;
    uint64_t     errorCount    = 0;
    float        p50Ms         = 0.f;
    float        p95Ms         = 0.f;
    float        p99Ms         = 0.f;
};

struct FleetHealthSnapshot {
    std::string   machineId;
    std::string   engineVersion;
    FleetHealthLevel   overall         = FleetHealthLevel::Healthy;
    uint64_t      decodeTotal     = 0;
    uint64_t      decodeErrors    = 0;
    float         cacheHitRate    = 0.f;   // 0.0–1.0
    uint32_t      cacheUsedMB     = 0;
    uint32_t      gpuVRAMUsedMB   = 0;
    uint8_t       memPressure     = 0;     // 0=None … 4=Critical
    bool          aiModelsLoaded  = false;
    bool          policyCompliant = true;
    std::vector<FleetSubsystemHealth>          subsystems;
    std::chrono::system_clock::time_point capturedAt;
};

class FleetHealthDashboard {
public:
    static FleetHealthDashboard& Instance() {
        static FleetHealthDashboard inst;
        return inst;
    }

    // Update subsystem metrics (thread-safe)
    void ReportDecodeOp(bool success, float latencyMs) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.decodeTotal++;
        if (!success) m_snapshot.decodeErrors++;
        UpdateLatencyBucket(latencyMs);
    }

    void SetCacheMetrics(float hitRate, uint32_t usedMB) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.cacheHitRate  = hitRate;
        m_snapshot.cacheUsedMB   = usedMB;
    }

    void SetGPUMetrics(uint32_t vramMB) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.gpuVRAMUsedMB = vramMB;
    }

    void SetMemPressure(uint8_t level) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.memPressure = level;
    }

    void SetAIModelsLoaded(bool loaded) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.aiModelsLoaded = loaded;
    }

    void SetPolicyCompliant(bool ok) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.policyCompliant = ok;
    }

    void RegisterSubsystem(const FleetSubsystemHealth& sh) {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& s : m_snapshot.subsystems) {
            if (s.name == sh.name) { s = sh; return; }
        }
        m_snapshot.subsystems.push_back(sh);
    }

    // Build and return current snapshot
    FleetHealthSnapshot Capture() {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_snapshot.capturedAt = std::chrono::system_clock::now();
        m_snapshot.overall    = ComputeOverall();
        return m_snapshot;
    }

    // Prometheus text format for /metrics scrape endpoint
    std::string ToPrometheus() const {
        std::ostringstream ss;
        auto& s = m_snapshot;
        ss << "# HELP explorerlens_decode_total Total decode operations\n"
           << "# TYPE explorerlens_decode_total counter\n"
           << "explorerlens_decode_total " << s.decodeTotal << "\n"
           << "explorerlens_decode_errors_total " << s.decodeErrors << "\n"
           << "explorerlens_cache_hit_ratio " << s.cacheHitRate << "\n"
           << "explorerlens_cache_used_mb " << s.cacheUsedMB << "\n"
           << "explorerlens_gpu_vram_used_mb " << s.gpuVRAMUsedMB << "\n"
           << "explorerlens_memory_pressure " << static_cast<int>(s.memPressure) << "\n"
           << "explorerlens_health_level "
           << static_cast<int>(s.overall) << "\n";
        return ss.str();
    }

    // JSON for Azure Monitor custom metrics or Intune
    std::string ToJson() const {
        std::ostringstream j;
        auto& s = m_snapshot;
        auto t = std::chrono::system_clock::to_time_t(s.capturedAt);
        char ts[32]; struct tm tm_buf;
        gmtime_s(&tm_buf, &t);
        strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &tm_buf);

        j << "{"
          << "\"machine\":\"" << s.machineId << "\","
          << "\"version\":\"" << s.engineVersion << "\","
          << "\"capturedAt\":\"" << ts << "\","
          << "\"overall\":" << static_cast<int>(s.overall) << ","
          << "\"decodeTotal\":" << s.decodeTotal << ","
          << "\"decodeErrors\":" << s.decodeErrors << ","
          << "\"cacheHitRate\":" << s.cacheHitRate << ","
          << "\"cacheUsedMB\":" << s.cacheUsedMB << ","
          << "\"gpuVRAMMB\":" << s.gpuVRAMUsedMB << ","
          << "\"memPressure\":" << static_cast<int>(s.memPressure) << ","
          << "\"aiLoaded\":" << (s.aiModelsLoaded ? "true" : "false") << ","
          << "\"policyCompliant\":" << (s.policyCompliant ? "true" : "false") << "}";
        return j.str();
    }

private:
    FleetHealthDashboard() {
        wchar_t cn[256] = {}; DWORD sz = 256;
        GetComputerNameExW(ComputerNameDnsHostname, cn, &sz);
        int len = WideCharToMultiByte(CP_UTF8, 0, cn, -1, nullptr, 0, nullptr, nullptr);
        m_snapshot.machineId.resize(len - 1);
        WideCharToMultiByte(CP_UTF8, 0, cn, -1, &m_snapshot.machineId[0], len, nullptr, nullptr);
        m_snapshot.engineVersion = "19.0.0";
    }

    FleetHealthLevel ComputeOverall() const {
        if (!m_snapshot.policyCompliant || m_snapshot.memPressure >= 4)
            return FleetHealthLevel::Critical;
        if (m_snapshot.memPressure >= 2 || m_snapshot.cacheHitRate < 0.3f)
            return FleetHealthLevel::Degraded;
        for (auto& s : m_snapshot.subsystems)
            if (s.level == FleetHealthLevel::Critical) return FleetHealthLevel::Critical;
        return FleetHealthLevel::Healthy;
    }

    void UpdateLatencyBucket(float ms) {
        // Simple running percentile approximation
        static thread_local std::vector<float> samples;
        samples.push_back(ms);
        if (samples.size() > 1000) samples.erase(samples.begin());
        // Use decode subsystem health
        for (auto& s : m_snapshot.subsystems) {
            if (s.name == "Decode") {
                s.p50Ms = ms; // simplified; real impl uses t-digest
                break;
            }
        }
    }

    FleetHealthSnapshot m_snapshot;
    std::mutex          m_mutex;
};

}}} // namespace ExplorerLens::Engine::Enterprise
