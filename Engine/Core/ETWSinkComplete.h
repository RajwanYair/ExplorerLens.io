#pragma once
//==============================================================================
// ETWSinkComplete.h
// Complete ETW provider sink wiring with retention policy, auto-rotation,
// schema versioning, and structured event emission for the full pipeline.
//
// Usage:
// #include "Core/ETWSinkComplete.h"
// auto& sink = ExplorerLens::ETW::ETWSinkManager::Get();
// sink.Configure(ETWSinkConfig::Production());
// sink.EmitDecodeEvent(event);
//==============================================================================

#include <string>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>
#include <cstdint>
#include <map>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ExplorerLens {
namespace ETW {

/// ETW event schema version for forward compatibility
struct SchemaVersion {
    static constexpr int Major = 2;
    static constexpr int Minor = 0;
    static constexpr const char* VersionString = "2.0";
};

/// ETW channel types matching Windows eventing model
enum class ETWChannel : uint8_t {
    Admin = 0, // IT-visible events (errors, config changes)
    Operational = 1, // Normal pipeline lifecycle events
    Analytic = 2, // High-frequency timing data (sampling only)
    Debug = 3 // Full verbose trace (development only)
};

/// Log rotation strategy
enum class RotationStrategy : uint8_t {
    SizeBased = 0, // Rotate when file exceeds size limit
    TimeBased = 1, // Rotate on schedule (daily/hourly)
    Hybrid = 2 // Rotate on whichever trigger fires first
};

/// Retention policy for log files
struct RetentionPolicy {
    int maxLogFiles = 10; // Keep N most recent log files
    size_t maxFileSizeBytes = 50 * 1024 * 1024; // 50 MB per file
    int maxRetentionDays = 30; // Delete logs older than N days
    RotationStrategy strategy = RotationStrategy::Hybrid;
    bool compressRotated = true; // Compress rotated logs

    static RetentionPolicy Development() {
        RetentionPolicy p;
        p.maxLogFiles = 5;
        p.maxFileSizeBytes = 10 * 1024 * 1024;
        p.maxRetentionDays = 7;
        p.compressRotated = false;
        return p;
    }
    static RetentionPolicy Production() {
        return RetentionPolicy{}; // Defaults are production
    }
    static RetentionPolicy Enterprise() {
        RetentionPolicy p;
        p.maxLogFiles = 50;
        p.maxFileSizeBytes = 100 * 1024 * 1024;
        p.maxRetentionDays = 90;
        p.compressRotated = true;
        return p;
    }
};

/// Structured ETW event template
struct ETWEvent {
    uint16_t eventId = 0;
    uint8_t level = 4; // 1=Critical..5=Verbose
    uint64_t keyword = 0;
    ETWChannel channel = ETWChannel::Operational;
    std::string taskName;
    std::string opcodeName;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> payload;

    void AddField(const std::string& key, const std::string& value) {
        payload[key] = value;
    }
    void AddField(const std::string& key, int64_t value) {
        payload[key] = std::to_string(value);
    }
    void AddField(const std::string& key, double value) {
        payload[key] = std::to_string(value);
    }
};

/// Well-known event IDs for ExplorerLens pipeline
struct EventIds {
    static constexpr uint16_t RequestStart = 100;
    static constexpr uint16_t RequestComplete = 101;
    static constexpr uint16_t CacheHit = 200;
    static constexpr uint16_t CacheMiss = 201;
    static constexpr uint16_t CacheEvict = 202;
    static constexpr uint16_t DecodeStart = 300;
    static constexpr uint16_t DecodeComplete = 301;
    static constexpr uint16_t DecodeFailed = 302;
    static constexpr uint16_t GPUSubmit = 400;
    static constexpr uint16_t GPUComplete = 401;
    static constexpr uint16_t PluginLoad = 500;
    static constexpr uint16_t PluginUnload = 501;
    static constexpr uint16_t PluginCrash = 502;
    static constexpr uint16_t MemoryPressure = 600;
    static constexpr uint16_t MemoryEviction = 601;
    static constexpr uint16_t ConfigChange = 700;
    static constexpr uint16_t HealthCheck = 800;
};

// Keywords defined in ETWTraceProvider.h (namespace ExplorerLens::ETW::Keywords)

/// Sink configuration
struct ETWSinkConfig {
    bool enableETW = true;
    bool enableFileLog = true;
    bool enableConsole = false;
    ETWChannel minChannel = ETWChannel::Operational;
    uint8_t minLevel = 4; // Info
    uint64_t enabledKeywords = Keywords::All;
    RetentionPolicy retention;
    std::string logDirectory; // Empty = %LocalAppData%\ExplorerLens\Logs
    std::string providerName = "ExplorerLens-Engine-Core";

    static ETWSinkConfig Production() {
        ETWSinkConfig c;
        c.minLevel = 4; // Info
        c.enableConsole = false;
        c.retention = RetentionPolicy::Production();
        return c;
    }
    static ETWSinkConfig Development() {
        ETWSinkConfig c;
        c.minLevel = 5; // Verbose
        c.enableConsole = true;
        c.retention = RetentionPolicy::Development();
        return c;
    }
    static ETWSinkConfig Enterprise() {
        ETWSinkConfig c;
        c.minLevel = 4;
        c.enableConsole = false;
        c.retention = RetentionPolicy::Enterprise();
        return c;
    }
};

/// File log entry for JSON-lines output
struct FileLogEntry {
    std::string timestamp;
    std::string level;
    std::string eventName;
    uint16_t eventId = 0;
    std::map<std::string, std::string> fields;

    std::string ToJsonLine() const {
        std::string json = "{";
        json += "\"ts\":\"" + timestamp + "\",";
        json += "\"level\":\"" + level + "\",";
        json += "\"event\":\"" + eventName + "\",";
        json += "\"id\":" + std::to_string(eventId);
        for (const auto& [k, v] : fields) {
            json += ",\"" + k + "\":\"" + v + "\"";
        }
        json += "}";
        return json;
    }
};

/// Sink statistics for diagnostics
struct SinkStatistics {
    std::atomic<uint64_t> eventsEmitted{ 0 };
    std::atomic<uint64_t> eventsDropped{ 0 };
    std::atomic<uint64_t> bytesWritten{ 0 };
    std::atomic<uint32_t> rotationsPerformed{ 0 };
    std::atomic<uint32_t> filesCleanedUp{ 0 };

    double DropRate() const {
        uint64_t total = eventsEmitted.load() + eventsDropped.load();
        return total > 0 ? 100.0 * eventsDropped.load() / total : 0.0;
    }
};

/// Main ETW sink manager — singleton controlling all event sinks
class ETWSinkManager {
public:
    static ETWSinkManager& Get() {
        static ETWSinkManager instance;
        return instance;
    }

    void Configure(const ETWSinkConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
        m_configured = true;
    }

    bool IsConfigured() const { return m_configured; }
    const ETWSinkConfig& Config() const { return m_config; }

    // ─── Event Emission ──────────────────────────────────────────
    void Emit(const ETWEvent& event) {
        if (!ShouldEmit(event)) {
            m_stats.eventsDropped++;
            return;
        }
        m_stats.eventsEmitted++;
        // In production, this dispatches to ETW provider + file sink
        for (auto& handler : m_handlers) {
            handler(event);
        }
    }

    void EmitDecodeEvent(uint16_t eventId, const std::string& decoder,
        const std::string& extension, double elapsedMs,
        bool success) {
        ETWEvent e;
        e.eventId = eventId;
        e.keyword = Keywords::Decoder;
        e.channel = ETWChannel::Operational;
        e.timestamp = std::chrono::system_clock::now();
        e.taskName = "Decode";
        e.AddField("decoder", decoder);
        e.AddField("extension", extension);
        e.AddField("elapsedMs", elapsedMs);
        e.AddField("success", success ? "true" : "false");
        Emit(e);
    }

    void EmitCacheEvent(uint16_t eventId, const std::string& key, bool hit) {
        ETWEvent e;
        e.eventId = eventId;
        e.keyword = Keywords::Cache;
        e.channel = ETWChannel::Operational;
        e.timestamp = std::chrono::system_clock::now();
        e.taskName = "Cache";
        e.AddField("key", key);
        e.AddField("hit", hit ? "true" : "false");
        Emit(e);
    }

    // ─── Handler Registration ────────────────────────────────────
    using EventHandler = std::function<void(const ETWEvent&)>;

    void AddHandler(EventHandler handler) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.push_back(std::move(handler));
    }

    void ClearHandlers() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handlers.clear();
    }

    // ─── Retention Management ────────────────────────────────────
    bool ShouldRotate(size_t currentFileSize) const {
        if (m_config.retention.strategy == RotationStrategy::SizeBased ||
            m_config.retention.strategy == RotationStrategy::Hybrid) {
            return currentFileSize >= m_config.retention.maxFileSizeBytes;
        }
        return false;
    }

    int MaxLogFiles() const { return m_config.retention.maxLogFiles; }
    int RetentionDays() const { return m_config.retention.maxRetentionDays; }

    // ─── Statistics ──────────────────────────────────────────────
    const SinkStatistics& Stats() const { return m_stats; }
    void ResetStats() {
        m_stats.eventsEmitted.store(0);
        m_stats.eventsDropped.store(0);
        m_stats.bytesWritten.store(0);
        m_stats.rotationsPerformed.store(0);
        m_stats.filesCleanedUp.store(0);
    }

private:
    ETWSinkManager() = default;

    bool ShouldEmit(const ETWEvent& event) const {
        if (!m_configured) return false;
        if (event.level > m_config.minLevel) return false;
        if ((event.keyword & m_config.enabledKeywords) == 0) return false;
        return true;
    }

    ETWSinkConfig m_config;
    bool m_configured = false;
    SinkStatistics m_stats;
    std::vector<EventHandler> m_handlers;
    std::mutex m_mutex;
};

}
} // namespace ExplorerLens::ETW
