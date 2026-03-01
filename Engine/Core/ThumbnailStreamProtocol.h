#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <mutex>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// ThumbnailStreamProtocol — Streaming protocol for remote thumbnails
// ============================================================================

enum class TSProtoType {
    HTTP,
    WebSocket,
    GRPC,
    NamedPipe,
    SharedMemory
};

inline const char* TSProtoTypeName(TSProtoType value) {
    switch (value) {
    case TSProtoType::HTTP:         return "HTTP";
    case TSProtoType::WebSocket:    return "WebSocket";
    case TSProtoType::GRPC:         return "GRPC";
    case TSProtoType::NamedPipe:    return "NamedPipe";
    case TSProtoType::SharedMemory: return "SharedMemory";
    default:                           return "Unknown";
    }
}

enum class TSProtoState {
    Idle,
    Connecting,
    Streaming,
    Paused,
    Error
};

inline const char* TSProtoStateName(TSProtoState value) {
    switch (value) {
    case TSProtoState::Idle:        return "Idle";
    case TSProtoState::Connecting:  return "Connecting";
    case TSProtoState::Streaming:   return "Streaming";
    case TSProtoState::Paused:      return "Paused";
    case TSProtoState::Error:       return "Error";
    default:                       return "Unknown";
    }
}

struct StreamEndpoint {
    TSProtoType protocol = TSProtoType::NamedPipe;
    std::string    host = "localhost";
    uint16_t       port = 0;
    std::string    path = "";
    std::string    authToken = "";
    uint32_t       timeoutMs = 5000;

    std::string GetConnectionString() const {
        if (protocol == TSProtoType::NamedPipe) {
            return "\\\\.\\pipe\\" + path;
        }
        if (protocol == TSProtoType::SharedMemory) {
            return "shm://" + path;
        }
        return host + ":" + std::to_string(port) + "/" + path;
    }

    bool RequiresAuth() const {
        return !authToken.empty();
    }
};

struct ThumbnailStreamRequest {
    std::wstring filePath;
    uint32_t     requestedWidth = 256;
    uint32_t     requestedHeight = 256;
    uint32_t     quality = 85;
    uint64_t     requestId = 0;
    bool         forceRefresh = false;
};

struct ThumbnailStreamResponse {
    uint64_t              requestId = 0;
    std::vector<uint8_t>  imageData;
    uint32_t              width = 0;
    uint32_t              height = 0;
    bool                  success = false;
    std::string           errorMessage;
    double                latencyMs = 0.0;
};

class ThumbnailStreamProtocol {
public:
    static constexpr uint32_t TIMEOUT_MS = 5000;
    static constexpr uint32_t MAX_CONCURRENT_STREAMS = 16;
    static constexpr uint32_t MAX_THUMBNAIL_SIZE = 4096;
    static constexpr uint32_t RECONNECT_DELAY_MS = 1000;

    ThumbnailStreamProtocol() = default;
    ~ThumbnailStreamProtocol() = default;

    ThumbnailStreamProtocol(const ThumbnailStreamProtocol&) = delete;
    ThumbnailStreamProtocol& operator=(const ThumbnailStreamProtocol&) = delete;

    bool Connect(const StreamEndpoint& endpoint) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_state == TSProtoState::Streaming || m_state == TSProtoState::Connecting) {
            return false; // Already connected
        }

        m_endpoint = endpoint;
        m_state = TSProtoState::Connecting;

        // In production: establish actual connection based on protocol
        // For testability, simulate successful connection
        m_state = TSProtoState::Streaming;
        m_connectTimeMs = GetCurrentTimeMs();
        m_totalConnections++;
        return true;
    }

    ThumbnailStreamResponse RequestThumbnail(const ThumbnailStreamRequest& request) {
        std::lock_guard<std::mutex> lock(m_mutex);

        ThumbnailStreamResponse response;
        response.requestId = request.requestId;

        if (m_state != TSProtoState::Streaming) {
            response.success = false;
            response.errorMessage = "Not connected (state=" + std::string(TSProtoStateName(m_state)) + ")";
            return response;
        }

        if (request.requestedWidth > MAX_THUMBNAIL_SIZE || request.requestedHeight > MAX_THUMBNAIL_SIZE) {
            response.success = false;
            response.errorMessage = "Requested size exceeds maximum";
            return response;
        }

        // Simulate response
        response.width = request.requestedWidth;
        response.height = request.requestedHeight;
        response.success = true;
        response.latencyMs = 1.5; // Simulated latency

        m_totalRequests++;
        return response;
    }

    TSProtoState GetState() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_state;
    }

    bool Disconnect() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state == TSProtoState::Idle) {
            return false;
        }
        m_state = TSProtoState::Idle;
        return true;
    }

    bool Pause() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state != TSProtoState::Streaming) return false;
        m_state = TSProtoState::Paused;
        return true;
    }

    bool Resume() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_state != TSProtoState::Paused) return false;
        m_state = TSProtoState::Streaming;
        return true;
    }

    const StreamEndpoint& GetEndpoint() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_endpoint;
    }

    uint64_t GetTotalRequests() const { return m_totalRequests; }
    uint64_t GetTotalConnections() const { return m_totalConnections; }

private:
    uint64_t GetCurrentTimeMs() const {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    mutable std::mutex  m_mutex;
    StreamEndpoint      m_endpoint;
    TSProtoState         m_state = TSProtoState::Idle;
    uint64_t            m_connectTimeMs = 0;
    uint64_t            m_totalRequests = 0;
    uint64_t            m_totalConnections = 0;
};

} // namespace Engine
} // namespace ExplorerLens
