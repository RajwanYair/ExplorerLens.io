// UsageStats.h — Anonymous Feature Usage Analytics Collector
// Copyright (c) 2026 ExplorerLens Project
//
// Collects opt-in anonymous telemetry about feature usage (format popularity,
// decode latency percentiles, GPU usage) to guide product development. All
// collection is gated by TelemetryConsentManager; no PII is ever captured.
//
#pragma once
#include <windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class UsageEvent : uint32_t {
    ThumbnailGenerated   = 1,
    FormatDecoded        = 2,
    CacheHit             = 3,
    CacheMiss            = 4,
    GpuDecodeUsed        = 5,
    CpuFallback          = 6,
    PluginLoaded         = 7,
    SettingsOpened       = 8,
    LicenseChecked       = 9,
    ErrorOccurred        = 10
};

struct UsageRecord {
    UsageEvent   event;
    std::wstring dimension; // Format name, plugin name, error code, etc.
    int64_t      value;     // Duration ms, file size, count, etc.
    SYSTEMTIME   timestamp;
};

struct UsageSnapshot {
    uint64_t                               sessionId;
    std::unordered_map<uint32_t, uint64_t> eventCounts;  // eventId -> count
    std::unordered_map<std::wstring, uint64_t> formatCounts;
    double   avgDecodeMs      = 0.0;
    double   p95DecodeMs      = 0.0;
    uint64_t cacheHitRate100  = 0; // Percentage * 100
    uint32_t gpuDecodePercent = 0;
    uint32_t sessionDurationSec = 0;
    std::wstring version     = L"20.0.0";
    std::wstring osVersion;
    std::wstring arch        = L"x64";
};

class UsageStats {
public:
    static UsageStats& Get() {
        static UsageStats s_inst;
        return s_inst;
    }

    void SetEnabled(bool enabled) {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_enabled = enabled;
    }

    bool IsEnabled() const { return m_enabled; }

    void Record(UsageEvent evt, const std::wstring& dimension = L"", int64_t value = 0) {
        if (!m_enabled) return;
        std::lock_guard<std::mutex> lk(m_mtx);
        UsageRecord rec{};
        rec.event     = evt;
        rec.dimension = dimension;
        rec.value     = value;
        GetLocalTime(&rec.timestamp);
        m_records.push_back(rec);

        if (m_records.size() >= k_FLUSH_THRESHOLD)
            FlushLocked();
    }

    // Convenience wrappers
    void RecordDecode(const std::wstring& format, int64_t durationMs) {
        Record(UsageEvent::ThumbnailGenerated, format, durationMs);
        Record(UsageEvent::FormatDecoded, format, 1);
        m_decodeTimes.push_back(static_cast<double>(durationMs));
    }

    void RecordCacheHit()  { Record(UsageEvent::CacheHit,  L"", 1); m_cacheHits++; }
    void RecordCacheMiss() { Record(UsageEvent::CacheMiss, L"", 1); m_cacheMisses++; }
    void RecordGpuDecode() { Record(UsageEvent::GpuDecodeUsed, L"", 1); }
    void RecordCpuFallback(const std::wstring& reason) {
        Record(UsageEvent::CpuFallback, reason, 1);
    }

    UsageSnapshot BuildSnapshot() const {
        std::lock_guard<std::mutex> lk(m_mtx);
        UsageSnapshot snap{};
        snap.sessionId = m_sessionId;
        snap.version   = L"20.0.0";

        // Event counts
        for (const auto& r : m_records)
            snap.eventCounts[static_cast<uint32_t>(r.event)]++;

        // Format counts
        for (const auto& r : m_records)
            if (r.event == UsageEvent::FormatDecoded)
                snap.formatCounts[r.dimension]++;

        // Decode stats
        if (!m_decodeTimes.empty()) {
            double sum = 0;
            for (double t : m_decodeTimes) sum += t;
            snap.avgDecodeMs = sum / m_decodeTimes.size();
            auto sorted = m_decodeTimes;
            std::sort(sorted.begin(), sorted.end());
            size_t p95i = static_cast<size_t>(sorted.size() * 0.95);
            snap.p95DecodeMs = sorted[p95i < sorted.size() ? p95i : sorted.size()-1];
        }

        uint64_t total = m_cacheHits + m_cacheMisses;
        if (total > 0) snap.cacheHitRate100 = (m_cacheHits * 10000) / total;

        // OS version
        OSVERSIONINFOW osv = {}; osv.dwOSVersionInfoSize = sizeof(osv);
        // Use RtlGetVersion via ntdll — avoid GetVersionEx
        auto pfn = reinterpret_cast<LONG(WINAPI*)(OSVERSIONINFOW*)>(
            GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
        if (pfn) pfn(&osv);
        snap.osVersion = std::to_wstring(osv.dwMajorVersion) + L"." +
                         std::to_wstring(osv.dwMinorVersion) + L"." +
                         std::to_wstring(osv.dwBuildNumber);

        return snap;
    }

    // Serialize snapshot as JSON for backend ingestion
    static std::wstring SnapshotToJson(const UsageSnapshot& s) {
        std::wstring j = L"{";
        j += L"\"sessionId\":" + std::to_wstring(s.sessionId) + L",";
        j += L"\"version\":\"" + s.version + L"\",";
        j += L"\"os\":\"" + s.osVersion + L"\",";
        j += L"\"arch\":\"" + s.arch + L"\",";
        j += L"\"avgDecodeMs\":" + std::to_wstring(static_cast<int>(s.avgDecodeMs)) + L",";
        j += L"\"p95DecodeMs\":" + std::to_wstring(static_cast<int>(s.p95DecodeMs)) + L",";
        j += L"\"cacheHitPct\":" + std::to_wstring(s.cacheHitRate100 / 100) + L",";
        j += L"\"eventCounts\":{";
        bool first = true;
        for (const auto& [k, v] : s.eventCounts) {
            if (!first) j += L",";
            j += L"\"" + std::to_wstring(k) + L"\":" + std::to_wstring(v);
            first = false;
        }
        j += L"}}";
        return j;
    }

    void OnFlush(std::function<void(const UsageSnapshot&)> cb) {
        m_onFlush = std::move(cb);
    }

private:
    UsageStats() {
        FILETIME ft = {}; GetSystemTimeAsFileTime(&ft);
        m_sessionId = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    }

    void FlushLocked() {
        if (m_onFlush) m_onFlush(BuildSnapshot());
        m_records.clear();
    }

    static constexpr size_t k_FLUSH_THRESHOLD = 1000;

    mutable std::mutex m_mtx;
    bool               m_enabled   = false; // Disabled by default; gated by consent
    uint64_t           m_sessionId = 0;
    std::vector<UsageRecord>     m_records;
    std::vector<double>          m_decodeTimes;
    uint64_t                     m_cacheHits   = 0;
    uint64_t                     m_cacheMisses = 0;
    std::function<void(const UsageSnapshot&)> m_onFlush;
};

}} // namespace ExplorerLens::Engine
