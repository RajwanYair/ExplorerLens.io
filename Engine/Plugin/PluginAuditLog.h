// PluginAuditLog.h — Security Audit Logging for Plugin Actions
// Copyright (c) 2026 ExplorerLens Project
//
// Thread-safe ring-buffer audit log for recording plugin security events.
// Supports configurable buffer size, severity-based filtering, time-range
// queries, and JSON-formatted export. Default capacity: 1024 entries.
//
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Audit severity levels
enum class PluginAuditSeverity : uint32_t {
    Info = 0,
    Warning = 1,
    SecurityEvent = 2,
    Violation = 3,
    Critical = 4
};

inline const wchar_t* PluginAuditSeverityName(PluginAuditSeverity s)
{
    switch (s) {
        case PluginAuditSeverity::Info:
            return L"Info";
        case PluginAuditSeverity::Warning:
            return L"Warning";
        case PluginAuditSeverity::SecurityEvent:
            return L"SecurityEvent";
        case PluginAuditSeverity::Violation:
            return L"Violation";
        case PluginAuditSeverity::Critical:
            return L"Critical";
        default:
            return L"Unknown";
    }
}

// Single audit log entry
struct PluginAuditEntry
{
    uint64_t sequenceId = 0;
    std::chrono::system_clock::time_point timestamp;
    PluginAuditSeverity severity = PluginAuditSeverity::Info;
    std::wstring pluginId;
    std::wstring action;
    std::wstring result;
    std::wstring details;
};

// ========================================================================
// PluginAuditLog — ring-buffer audit log with thread-safe operations
// ========================================================================
class PluginAuditLog
{
  public:
    static constexpr uint32_t DEFAULT_CAPACITY = 1024;

    static PluginAuditLog& Instance()
    {
        static PluginAuditLog instance;
        return instance;
    }

    void Initialize(uint32_t capacity = DEFAULT_CAPACITY)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_capacity = (capacity > 0) ? capacity : DEFAULT_CAPACITY;
        m_entries.clear();
        m_entries.reserve(m_capacity);
        m_head = 0;
        m_count = 0;
        m_nextSequenceId = 1;
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }
    uint32_t GetCapacity() const
    {
        return m_capacity;
    }

    // Append an entry to the ring buffer (thread-safe)
    void Append(PluginAuditSeverity severity, const std::wstring& pluginId, const std::wstring& action,
                const std::wstring& result, const std::wstring& details = L"")
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        PluginAuditEntry entry;
        entry.sequenceId = m_nextSequenceId++;
        entry.timestamp = std::chrono::system_clock::now();
        entry.severity = severity;
        entry.pluginId = pluginId;
        entry.action = action;
        entry.result = result;
        entry.details = details;

        if (m_count < m_capacity) {
            m_entries.push_back(std::move(entry));
            m_count++;
        } else {
            // Overwrite oldest entry (ring buffer)
            m_entries[m_head] = std::move(entry);
            m_head = (m_head + 1) % m_capacity;
        }
    }

    // Get total entries currently stored
    uint32_t GetEntryCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }

    // Get all entries in chronological order
    std::vector<PluginAuditEntry> GetAllEntries() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return GetOrderedEntries();
    }

    // Query entries by severity
    std::vector<PluginAuditEntry> QueryBySeverity(PluginAuditSeverity severity) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto ordered = GetOrderedEntries();
        std::vector<PluginAuditEntry> filtered;
        for (auto& e : ordered) {
            if (e.severity == severity)
                filtered.push_back(e);
        }
        return filtered;
    }

    // Query entries by plugin ID
    std::vector<PluginAuditEntry> QueryByPluginId(const std::wstring& pluginId) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto ordered = GetOrderedEntries();
        std::vector<PluginAuditEntry> filtered;
        for (auto& e : ordered) {
            if (e.pluginId == pluginId)
                filtered.push_back(e);
        }
        return filtered;
    }

    // Query entries by time range
    std::vector<PluginAuditEntry> QueryByTimeRange(std::chrono::system_clock::time_point startTime,
                                                   std::chrono::system_clock::time_point endTime) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto ordered = GetOrderedEntries();
        std::vector<PluginAuditEntry> filtered;
        for (auto& e : ordered) {
            if (e.timestamp >= startTime && e.timestamp <= endTime) {
                filtered.push_back(e);
            }
        }
        return filtered;
    }

    // Export to JSON-formatted string
    std::wstring ExportToJSON() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto ordered = GetOrderedEntries();

        std::wstring json = L"[\n";
        for (size_t i = 0; i < ordered.size(); i++) {
            auto& e = ordered[i];
            json += L"  {\n";
            json += L"    \"sequenceId\": " + std::to_wstring(e.sequenceId) + L",\n";
            json += L"    \"severity\": \"" + std::wstring(PluginAuditSeverityName(e.severity)) + L"\",\n";
            json += L"    \"pluginId\": \"" + EscapeJSON(e.pluginId) + L"\",\n";
            json += L"    \"action\": \"" + EscapeJSON(e.action) + L"\",\n";
            json += L"    \"result\": \"" + EscapeJSON(e.result) + L"\"";
            if (!e.details.empty()) {
                json += L",\n    \"details\": \"" + EscapeJSON(e.details) + L"\"";
            }
            json += L"\n  }";
            if (i + 1 < ordered.size())
                json += L",";
            json += L"\n";
        }
        json += L"]";
        return json;
    }

    // Get count of entries at or above a severity threshold
    uint32_t CountBySeverity(PluginAuditSeverity minSeverity) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t count = 0;
        for (auto& e : m_entries) {
            if (static_cast<uint32_t>(e.severity) >= static_cast<uint32_t>(minSeverity)) {
                count++;
            }
        }
        return count;
    }

    // Clear all entries
    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        m_head = 0;
        m_count = 0;
    }

  private:
    PluginAuditLog() = default;

    // Get entries in chronological order (must hold lock)
    std::vector<PluginAuditEntry> GetOrderedEntries() const
    {
        std::vector<PluginAuditEntry> ordered;
        ordered.reserve(m_count);
        if (m_count < m_capacity) {
            // Buffer not yet full — entries are in order
            ordered.assign(m_entries.begin(), m_entries.end());
        } else {
            // Ring buffer wrapped — read from head to end, then start to head
            for (uint32_t i = 0; i < m_count; i++) {
                uint32_t idx = (m_head + i) % m_capacity;
                ordered.push_back(m_entries[idx]);
            }
        }
        return ordered;
    }

    // Escape special characters for JSON
    static std::wstring EscapeJSON(const std::wstring& str)
    {
        std::wstring escaped;
        escaped.reserve(str.size() + 8);
        for (wchar_t c : str) {
            switch (c) {
                case L'\"':
                    escaped += L"\\\"";
                    break;
                case L'\\':
                    escaped += L"\\\\";
                    break;
                case L'\n':
                    escaped += L"\\n";
                    break;
                case L'\r':
                    escaped += L"\\r";
                    break;
                case L'\t':
                    escaped += L"\\t";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }

    mutable std::mutex m_mutex;
    std::vector<PluginAuditEntry> m_entries;
    uint32_t m_capacity = DEFAULT_CAPACITY;
    uint32_t m_head = 0;   // Index of oldest entry when buffer is full
    uint32_t m_count = 0;  // Current number of entries
    uint64_t m_nextSequenceId = 1;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
