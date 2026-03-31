// ThumbnailStream.h — Unified Streaming, Protocol & Transport
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header consolidating: ThumbnailStreamProtocol.h,
// ThumbnailStreamMultiplexer.h, ThumbnailPreviewEngine.h,
// ThumbnailVersionControl.h.
// Part of v31.2.0 consolidation pass.
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

// -- Stream protocol types (from StreamProtocol) ------------------------------

enum class StreamProtoType : uint8_t { HTTP, WebSocket, GRPC, NamedPipe, SharedMemory };

enum class ThumbnailStreamState : uint8_t { Idle, Connecting, Streaming, Paused, Error, Closed };

// -- Multiplexer load-balance (from StreamMultiplexer) ------------------------

enum class StreamRouteStrategy : uint8_t { RoundRobin, LeastLoaded, FormatAffinity };

// -- Version control (from VersionControl) ------------------------------------

enum class ThumbnailVersion : uint8_t { Original, Modified, Current, Rollback, Draft };

enum class VersionAction : uint8_t { Create, Update, Rollback, Delete, Purge, Archive };

inline const char* ThumbnailVersionName(ThumbnailVersion v) noexcept {
    switch (v) {
    case ThumbnailVersion::Original: return "Original";
    case ThumbnailVersion::Modified: return "Modified";
    case ThumbnailVersion::Current:  return "Current";
    case ThumbnailVersion::Rollback: return "Rollback";
    case ThumbnailVersion::Draft:    return "Draft";
    default: return "Unknown";
    }
}

inline const char* VersionActionName(VersionAction a) noexcept {
    switch (a) {
    case VersionAction::Create:  return "Create";
    case VersionAction::Update:  return "Update";
    case VersionAction::Rollback:return "Rollback";
    case VersionAction::Delete:  return "Delete";
    case VersionAction::Purge:   return "Purge";
    case VersionAction::Archive: return "Archive";
    default: return "Unknown";
    }
}

// -- Preview state (from PreviewEngine) ---------------------------------------

struct PreviewState {
    float zoomLevel = 1.0f;
    float panX      = 0.0f;
    float panY      = 0.0f;
    bool  showGrid  = false;
    bool  showAlpha = true;
};

// -- Config & result structs --------------------------------------------------

struct StreamEndpoint {
    StreamProtoType protocol = StreamProtoType::NamedPipe;
    std::string     address;
    uint16_t        port     = 0;
    uint32_t        timeoutMs = 5000;
};

struct StreamMultiplexerConfig {
    uint32_t maxStreams          = 8;
    uint32_t queueDepthPerStream = 32;
    StreamRouteStrategy strategy = StreamRouteStrategy::LeastLoaded;
    bool enabled                = true;
};

struct VersionEntry {
    ThumbnailVersion version   = ThumbnailVersion::Original;
    VersionAction    action    = VersionAction::Create;
    uint64_t         timestamp = 0;
    uint32_t         crc32     = 0;
    uint32_t         sizeBytes = 0;
    std::string      label;
};

// -- Unified stream engine ----------------------------------------------------

class ThumbnailStreamEngine {
public:
    bool Connect(const StreamEndpoint& ep) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_endpoint = ep;
        m_state = ThumbnailStreamState::Connecting;
        m_state = ThumbnailStreamState::Streaming;
        return true;
    }

    void Disconnect() {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_state = ThumbnailStreamState::Closed;
    }

    ThumbnailStreamState GetState() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_state;
    }

    uint32_t RouteRequest(uint32_t formatHint = 0) {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_config.maxStreams == 0) return 0;
        uint32_t best = 0, bestLoad = m_streamLoads.empty() ? 0 : m_streamLoads[0];
        for (uint32_t i = 1; i < m_config.maxStreams && i < m_streamLoads.size(); ++i) {
            if (m_streamLoads[i] < bestLoad) { bestLoad = m_streamLoads[i]; best = i; }
        }
        if (!m_streamLoads.empty()) ++m_streamLoads[best];
        (void)formatHint;
        return best;
    }

    void AddVersion(const std::string& key, const VersionEntry& entry) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_history.push_back({key, entry});
    }

    size_t VersionCount() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_history.size();
    }

    void SetPreviewState(const PreviewState& state) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_preview = state;
    }

    PreviewState GetPreviewState() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_preview;
    }

    void Configure(const StreamMultiplexerConfig& cfg) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_config = cfg;
        m_streamLoads.assign(cfg.maxStreams, 0);
    }

private:
    mutable std::mutex m_mutex;
    StreamEndpoint m_endpoint;
    ThumbnailStreamState m_state = ThumbnailStreamState::Idle;
    StreamMultiplexerConfig m_config;
    std::vector<uint32_t> m_streamLoads;
    std::vector<std::pair<std::string, VersionEntry>> m_history;
    PreviewState m_preview;
};

// -- TSProto aliases (for tests using TSProtoType / TSProtoState names) -------

using TSProtoType  = StreamProtoType;
using TSProtoState = ThumbnailStreamState;

inline const char* TSProtoTypeName(TSProtoType t) noexcept {
    switch (t) {
    case TSProtoType::HTTP:         return "HTTP";
    case TSProtoType::WebSocket:    return "WebSocket";
    case TSProtoType::GRPC:         return "GRPC";
    case TSProtoType::NamedPipe:    return "NamedPipe";
    case TSProtoType::SharedMemory: return "SharedMemory";
    default: return "Unknown";
    }
}

inline const char* TSProtoStateName(TSProtoState s) noexcept {
    switch (s) {
    case ThumbnailStreamState::Idle:        return "Idle";
    case ThumbnailStreamState::Connecting:  return "Connecting";
    case ThumbnailStreamState::Streaming:   return "Streaming";
    case ThumbnailStreamState::Paused:      return "Paused";
    case ThumbnailStreamState::Error:       return "Error";
    case ThumbnailStreamState::Closed:      return "Closed";
    default: return "Unknown";
    }
}

class ThumbnailStreamProtocol {
public:
    static constexpr uint32_t TIMEOUT_MS = 5000;
};

// -- Version control engine (from ThumbnailVersionControl.h) -----------------

class ThumbnailVersionControl {
public:
    bool CreateVersion(const std::wstring& label, uint32_t crc32, uint32_t sizeBytes) {
        VersionEntry e;
        e.version   = ThumbnailVersion::Modified;
        e.action    = VersionAction::Create;
        e.crc32     = crc32;
        e.sizeBytes = sizeBytes;
        e.label     = std::string(label.begin(), label.end());
        e.timestamp = ++m_counter;
        m_history.push_back(e);
        return true;
    }
    bool Rollback() {
        VersionEntry e;
        e.version   = ThumbnailVersion::Rollback;
        e.action    = VersionAction::Rollback;
        e.timestamp = ++m_counter;
        m_history.push_back(e);
        ++m_rollbackCount;
        return true;
    }
    const std::vector<VersionEntry>& GetHistory() const noexcept { return m_history; }
    uint32_t GetRollbackCount() const noexcept { return m_rollbackCount; }
private:
    std::vector<VersionEntry> m_history;
    uint32_t                  m_rollbackCount = 0;
    uint64_t                  m_counter       = 0;
};

} // namespace Engine
} // namespace ExplorerLens
