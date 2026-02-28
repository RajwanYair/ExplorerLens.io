// =============================================================================
// ipc_protocol.h - Inter-Process Communication Protocol v1.0
// =============================================================================
// Defines communication between ShellHost (in-proc) and Worker (out-of-proc)
// =============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>

namespace ExplorerLens {
namespace IPC {

// =============================================================================
// Protocol Version
// =============================================================================

constexpr uint32_t IPC_PROTOCOL_VERSION = 1;
constexpr uint32_t IPC_MAGIC_NUMBER = 0x44545031; // "DTP1"

// =============================================================================
// Message Types
// =============================================================================

enum class MessageType : uint32_t {
    // Request Messages (ShellHost → Worker)
    REQUEST_THUMBNAIL = 0x0001,
    REQUEST_CANCEL = 0x0002,
    REQUEST_SHUTDOWN = 0x0003,
    REQUEST_PING = 0x0004,
    
    // Response Messages (Worker → ShellHost)
    RESPONSE_THUMBNAIL = 0x1001,
    RESPONSE_PROGRESS = 0x1002,
    RESPONSE_ERROR = 0x1003,
    RESPONSE_PONG = 0x1004,
    
    // Control Messages
    CONTROL_HEARTBEAT = 0x2001,
    CONTROL_RESTART = 0x2002,
};

// =============================================================================
// Message Priority
// =============================================================================

enum class Priority : uint8_t {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    URGENT = 3,
};

// =============================================================================
// Request Flags
// =============================================================================

enum RequestFlags : uint32_t {
    FLAG_ALLOW_GPU = 1 << 0,
    FLAG_ALLOW_NETWORK = 1 << 1,
    FLAG_ALLOW_PLUGINS = 1 << 2,
    FLAG_USE_CACHE = 1 << 3,
    FLAG_HIGH_QUALITY = 1 << 4,
    FLAG_ASYNC_MODE = 1 << 5,
};

// =============================================================================
// Error Codes
// =============================================================================

enum class ErrorCode : uint32_t {
    SUCCESS = 0,
    
    // IPC Errors (0x1000 - 0x1FFF)
    IPC_CONNECTION_FAILED = 0x1001,
    IPC_TIMEOUT = 0x1002,
    IPC_PIPE_BROKEN = 0x1003,
    IPC_SERIALIZATION_ERROR = 0x1004,
    IPC_PROTOCOL_MISMATCH = 0x1005,
    
    // Worker Errors (0x2000 - 0x2FFF)
    WORKER_NOT_READY = 0x2001,
    WORKER_CRASHED = 0x2002,
    WORKER_TIMEOUT = 0x2003,
    WORKER_OVERLOADED = 0x2004,
    
    // Processing Errors (0x3000 - 0x3FFF)
    FORMAT_NOT_SUPPORTED = 0x3001,
    FILE_NOT_FOUND = 0x3002,
    FILE_ACCESS_DENIED = 0x3003,
    DECODE_FAILED = 0x3004,
    RENDER_FAILED = 0x3005,
    OUT_OF_MEMORY = 0x3006,
    
    // Policy Errors (0x4000 - 0x4FFF)
    POLICY_DENIED_GPU = 0x4001,
    POLICY_DENIED_NETWORK = 0x4002,
    POLICY_DENIED_PLUGIN = 0x4003,
};

// =============================================================================
// Message Header (Common to all messages)
// =============================================================================

#pragma pack(push, 1)

struct MessageHeader {
    uint32_t magicNumber;      // Always IPC_MAGIC_NUMBER
    uint32_t protocolVersion;  // IPC_PROTOCOL_VERSION
    MessageType messageType;
    uint32_t messageId;        // Unique ID for request/response correlation
    uint64_t correlationId;    // Trace ID for logging
    uint32_t payloadSize;      // Size of payload following this header
    Priority priority;
    uint8_t reserved[3];       // Padding for alignment
    uint64_t timestamp;        // Microseconds since epoch
};

static_assert(sizeof(MessageHeader) == 40, "MessageHeader must be 40 bytes");

#pragma pack(pop)

// =============================================================================
// Thumbnail Request (ShellHost → Worker)
// =============================================================================

struct ThumbnailRequest {
    MessageHeader header;
    
    // Request details
    uint32_t sizePx;           // Requested thumbnail size
    uint32_t flags;            // RequestFlags bitfield
    uint32_t timeoutMs;        // Maximum processing time
    uint32_t pathLength;       // Length of file path in bytes
    
    // Variable-length payload follows:
    // - wchar_t path[pathLength/2]  // File path (UTF-16)
    
    static constexpr size_t MAX_PATH_LENGTH = 32767 * 2; // 32KB path limit
};

// =============================================================================
// Thumbnail Response (Worker → ShellHost)
// =============================================================================

struct ThumbnailResponse {
    MessageHeader header;
    
    // Result details
    ErrorCode errorCode;
    uint32_t width;            // Actual thumbnail width
    uint32_t height;           // Actual thumbnail height
    uint32_t pixelFormat;      // DXGI_FORMAT or similar
    uint32_t stride;           // Row stride in bytes
    uint32_t dataSize;         // Size of pixel data
    
    // Performance metrics
    uint64_t totalTimeUs;      // Total processing time
    uint64_t decodeTimeUs;     // Decode stage time
    uint64_t renderTimeUs;     // Render stage time
    uint32_t stageMask;        // Which stages executed
    uint8_t cacheHit;          // 1 if cache hit, 0 otherwise
    uint8_t gpuUsed;           // 1 if GPU used, 0 otherwise
    uint8_t reserved[2];
    
    // Variable-length payload follows:
    // - uint8_t pixelData[dataSize]  // BGRA or similar format
};

// =============================================================================
// Progress Update (Worker → ShellHost)
// =============================================================================

struct ProgressUpdate {
    MessageHeader header;
    
    uint32_t percentComplete;  // 0-100
    uint32_t currentStage;     // Stage identifier
    uint32_t messageLength;    // Optional status message length
    
    // Variable-length payload:
    // - wchar_t message[messageLength/2]
};

// =============================================================================
// Cancel Request (ShellHost → Worker)
// =============================================================================

struct CancelRequest {
    MessageHeader header;
    
    uint32_t requestIdToCancel; // MessageId of request to cancel
};

// =============================================================================
// Error Response (Worker → ShellHost)
// =============================================================================

struct ErrorResponse {
    MessageHeader header;
    
    ErrorCode errorCode;
    uint32_t errorMessageLength;
    
    // Variable-length payload:
    // - wchar_t errorMessage[errorMessageLength/2]
};

// =============================================================================
// Ping/Pong (Health Check)
// =============================================================================

struct PingRequest {
    MessageHeader header;
    uint64_t sendTime;         // For latency measurement
};

struct PongResponse {
    MessageHeader header;
    uint64_t sendTime;         // Echo from ping
    uint64_t receiveTime;      // Worker receive time
    uint32_t queueDepth;       // Requests in worker queue
    uint32_t workerLoad;       // 0-100 CPU usage estimate
};

// =============================================================================
// Heartbeat (Keep-alive)
// =============================================================================

struct HeartbeatMessage {
    MessageHeader header;
    uint64_t uptimeSeconds;
    uint32_t processedCount;   // Total thumbnails processed
    uint32_t errorCount;       // Total errors
    uint32_t cacheHitRate;     // 0-10000 (0.00% - 100.00%)
};

// =============================================================================
// Transport Configuration
// =============================================================================

struct TransportConfig {
    // Named Pipe Configuration
    std::wstring pipeName = L"\\\\.\\pipe\\ExplorerLens_Worker_";
    uint32_t pipeBufferSize = 65536;     // 64 KB
    uint32_t maxInstances = 4;           // Max concurrent connections
    
    // Shared Memory Configuration (optional for large payloads)
    bool useSharedMemory = false;
    uint32_t sharedMemorySize = 16777216; // 16 MB
    std::wstring sharedMemoryName = L"ExplorerLens_SharedMem_";
    
    // Timeouts
    uint32_t connectTimeoutMs = 5000;    // 5 seconds
    uint32_t readTimeoutMs = 30000;      // 30 seconds
    uint32_t writeTimeoutMs = 10000;     // 10 seconds
    
    // Performance
    uint32_t maxQueueDepth = 100;
    uint32_t workerPoolSize = 2;         // Warm workers to maintain
};

// =============================================================================
// Protocol Constants
// =============================================================================

namespace Constants {
    constexpr uint32_t MAX_MESSAGE_SIZE = 16777216;      // 16 MB
    constexpr uint32_t MAX_THUMBNAIL_SIZE = 8388608;     // 8 MB pixels
    constexpr uint32_t DEFAULT_TIMEOUT_MS = 30000;       // 30 seconds
    constexpr uint32_t HEARTBEAT_INTERVAL_MS = 5000;     // 5 seconds
    constexpr uint32_t WORKER_STARTUP_TIMEOUT_MS = 10000; // 10 seconds
    constexpr uint32_t MAX_RETRY_ATTEMPTS = 3;
    constexpr uint32_t RETRY_BACKOFF_MS = 1000;          // 1 second
}

// =============================================================================
// Helper Functions
// =============================================================================

inline uint64_t GetTimestampUs() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()
    ).count();
}

inline uint32_t GenerateMessageId() {
    static std::atomic<uint32_t> s_messageIdCounter{1};
    return s_messageIdCounter.fetch_add(1, std::memory_order_relaxed);
}

inline MessageHeader CreateHeader(MessageType type, uint64_t correlationId, 
                                   uint32_t payloadSize, Priority priority = Priority::NORMAL) {
    MessageHeader header = {};
    header.magicNumber = IPC_MAGIC_NUMBER;
    header.protocolVersion = IPC_PROTOCOL_VERSION;
    header.messageType = type;
    header.messageId = GenerateMessageId();
    header.correlationId = correlationId;
    header.payloadSize = payloadSize;
    header.priority = priority;
    header.timestamp = GetTimestampUs();
    return header;
}

inline bool ValidateHeader(const MessageHeader& header) {
    return header.magicNumber == IPC_MAGIC_NUMBER &&
           header.protocolVersion == IPC_PROTOCOL_VERSION &&
           header.payloadSize <= Constants::MAX_MESSAGE_SIZE;
}

// =============================================================================
// Status Codes for IPC Operations
// =============================================================================

enum class IPCStatus {
    OK,
    TIMEOUT,
    CONNECTION_CLOSED,
    PROTOCOL_ERROR,
    OUT_OF_MEMORY,
    INVALID_PARAMETER,
};

} // namespace IPC
} // namespace ExplorerLens

