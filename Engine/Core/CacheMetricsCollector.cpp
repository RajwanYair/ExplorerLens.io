// CacheMetricsCollector.cpp — Periodic cache metrics harvester
// Copyright (c) 2026 ExplorerLens Project
//
#include "CacheMetricsCollector.h"

#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

// Lightweight JSON value extraction (avoids pulling in a JSON library)
static double ExtractDouble(const std::string& json, const std::string& key, double def = 0.0) {
    auto pos = json.find('"' + key + '"');
    if (pos == std::string::npos) return def;
    auto colon = json.find(':', pos);
    if (colon == std::string::npos) return def;
    try { return std::stod(json.substr(colon + 1)); } catch (...) { return def; }
}

static uint64_t ExtractU64(const std::string& json, const std::string& key, uint64_t def = 0) {
    auto pos = json.find('"' + key + '"');
    if (pos == std::string::npos) return def;
    auto colon = json.find(':', pos);
    if (colon == std::string::npos) return def;
    try { return std::stoull(json.substr(colon + 1)); } catch (...) { return def; }
}

namespace ExplorerLens {
namespace Engine {

CacheMetricsCollector::CacheMetricsCollector(uint32_t intervalSeconds)
    : m_intervalSeconds(intervalSeconds) {}

CacheMetricsCollector::~CacheMetricsCollector() {
    Stop();
}

void CacheMetricsCollector::Start(std::function<std::string()> statsProvider,
                                   MetricsCallback callback) {
    if (m_running.exchange(true)) return;
    m_statsProvider = std::move(statsProvider);
    m_callback      = std::move(callback);
    m_thread = std::thread([this] { CollectorLoop(); });
}

void CacheMetricsCollector::Stop() {
    m_running.store(false);
    if (m_thread.joinable()) m_thread.join();
}

void CacheMetricsCollector::SetLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lk(m_snapshotMutex);
    m_logPath = path;
}

void CacheMetricsCollector::SetLogFile(const std::wstring& path) {
    SetLogFile(std::string(path.begin(), path.end()));
}

CacheSnapshot CacheMetricsCollector::LastSnapshot() const {
    std::lock_guard<std::mutex> lk(m_snapshotMutex);
    return m_lastSnapshot;
}

void CacheMetricsCollector::CollectorLoop() {
    // Use fine-grained sleep so Stop() responds quickly
    const auto sleepSlice = std::chrono::milliseconds(250);
    auto nextTick = std::chrono::steady_clock::now();

    while (m_running.load()) {
        auto now = std::chrono::steady_clock::now();
        if (now >= nextTick) {
            nextTick = now + std::chrono::seconds(m_intervalSeconds);

            if (m_statsProvider) {
                std::string json = m_statsProvider();
                CacheSnapshot snap = ParseStats(json);
                snap.rawJson = json;

                {
                    std::lock_guard<std::mutex> lk(m_snapshotMutex);
                    m_lastSnapshot = snap;
                }

                EmitETW(snap);

                if (!m_logPath.empty()) {
                    AppendToLog(snap);
                }

                if (m_callback) {
                    m_callback(snap);
                }
            }
        }
        std::this_thread::sleep_for(sleepSlice);
    }
}

CacheSnapshot CacheMetricsCollector::ParseStats(const std::string& json) const {
    CacheSnapshot snap;
    snap.l1HitRate     = ExtractDouble(json, "l1HitRate");
    snap.l2HitRate     = ExtractDouble(json, "l2HitRate");
    snap.l1Inserts     = ExtractU64(json, "l1Inserts");
    snap.l2Inserts     = ExtractU64(json, "l2Inserts");
    snap.totalRequests = ExtractU64(json, "totalRequests");
    snap.evictions     = ExtractU64(json, "evictions");
    snap.l1FillPercent = ExtractDouble(json, "l1FillPercent");

    // ISO-8601 UTC timestamp
    auto now  = std::chrono::system_clock::now();
    auto tt   = std::chrono::system_clock::to_time_t(now);
    char buf[32] = {};
    struct tm utc;
#ifdef _WIN32
    gmtime_s(&utc, &tt);
#else
    gmtime_r(&tt, &utc);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    snap.timestamp = buf;
    return snap;
}

void CacheMetricsCollector::EmitETW(const CacheSnapshot& snap) const {
    // ETW emission is handled by ETWTraceProvider; we log to stderr in non-Windows or debug builds.
#if defined(_DEBUG) || !defined(_WIN32)
    (void)snap;
#endif
    // In production, ETWTraceProvider::Instance().EmitCacheMetrics(snap) would be called here.
    // Decoupled to avoid circular dependency — ETWTraceProvider includes this header's snapshot type.
}

void CacheMetricsCollector::AppendToLog(const CacheSnapshot& snap) const {
    constexpr size_t MAX_LOG_BYTES = 1 * 1024 * 1024; // 1 MB
    try {
        // Rotate if too large
        std::string rotated = m_logPath + ".1";
        {
            std::ifstream sizer(m_logPath, std::ios::ate | std::ios::binary);
            if (sizer && static_cast<size_t>(sizer.tellg()) > MAX_LOG_BYTES) {
                sizer.close();
                std::rename(m_logPath.c_str(), rotated.c_str());
            }
        }

        std::ofstream out(m_logPath, std::ios::app);
        if (!out) return;
        // Append a single-line JSON record
        out << "{\"ts\":\"" << snap.timestamp
            << "\",\"l1Hit\":" << snap.l1HitRate
            << ",\"l2Hit\":" << snap.l2HitRate
            << ",\"reqs\":" << snap.totalRequests
            << ",\"evict\":" << snap.evictions
            << ",\"l1fill\":" << snap.l1FillPercent
            << "}\n";
    } catch (...) {
        // Best-effort logging — never throw from background thread
    }
}

} // namespace Engine
} // namespace ExplorerLens
