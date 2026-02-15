/******************************************************************************
 * DarkThumbs Plugin IPC Protocol
 * Copyright (c) 2026 - DarkThumbs Project
 * 
 * Defines the inter-process communication protocol between the Engine and
 * PluginHost processes for secure plugin isolation.
 *****************************************************************************/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs {
namespace IPC {

//============================================================================
// Protocol Constants
//============================================================================

// Magic number for message validation
constexpr uint32_t PLUGIN_IPC_MAGIC = 0x44545048;  // 'DTPH' (DarkThumbs Plugin Host)

// Protocol version
constexpr uint32_t PLUGIN_IPC_VERSION = 1;

// Maximum message size (1 MB for metadata, use shared memory for large data)
constexpr uint32_t MAX_MESSAGE_SIZE = 1024 * 1024;

// Shared memory threshold (files > 1MB use shared memory)
constexpr uint32_t SHARED_MEMORY_THRESHOLD = 1024 * 1024;

// Timeout values (milliseconds)
constexpr uint32_t DEFAULT_REQUEST_TIMEOUT = 30000;  // 30 seconds
constexpr uint32_t HEARTBEAT_INTERVAL = 5000;        // 5 seconds
constexpr uint32_t HEARTBEAT_TIMEOUT = 15000;        // 15 seconds (3 missed heartbeats)

//============================================================================
// Message Types
//============================================================================

enum class MessageType : uint32_t {
    // Request/Response
    REQUEST_THUMBNAIL = 0x01,
    RESPONSE_THUMBNAIL = 0x02,
    
    // Error handling
    ERROR_RESPONSE = 0x03,
    
    // Lifecycle
    SHUTDOWN = 0x04,
    
    // Keepalive
    HEARTBEAT_PING = 0x05,
    HEARTBEAT_PONG = 0x06,
    
    // Plugin info
    GET_PLUGIN_INFO = 0x07,
    PLUGIN_INFO_RESPONSE = 0x08,
    
    // Can decode query
    CAN_DECODE_QUERY = 0x09,
    CAN_DECODE_RESPONSE = 0x0A
};

//============================================================================
// Error Codes
//============================================================================

enum class IPCErrorCode : uint32_t {
    SUCCESS = 0,
    
    // Protocol errors
    INVALID_MAGIC = 0x1000,
    INVALID_VERSION = 0x1001,
    INVALID_MESSAGE_TYPE = 0x1002,
    MESSAGE_TOO_LARGE = 0x1003,
    MALFORMED_MESSAGE = 0x1004,
    
    // Connection errors
    PIPE_CONNECTION_FAILED = 0x2000,
    PIPE_WRITE_FAILED = 0x2001,
    PIPE_READ_FAILED = 0x2002,
    PIPE_BROKEN = 0x2003,
    
    // Timeout errors
    REQUEST_TIMEOUT = 0x3000,
    HEARTBEAT_TIMEOUT = 0x3001,
    
    // Process errors
    PROCESS_SPAWN_FAILED = 0x4000,
    PROCESS_TERMINATED = 0x4001,
    PROCESS_CRASHED = 0x4002,
    
    // Resource errors
    OUT_OF_MEMORY = 0x5000,
    RESOURCE_LIMIT_EXCEEDED = 0x5001,
    
    // Plugin errors
    PLUGIN_LOAD_FAILED = 0x6000,
    PLUGIN_INIT_FAILED = 0x6001,
    PLUGIN_DECODE_FAILED = 0x6002,
    PLUGIN_UNSUPPORTED_FORMAT = 0x6003,
    
    // Shared memory errors
    SHARED_MEMORY_CREATE_FAILED = 0x7000,
    SHARED_MEMORY_MAP_FAILED = 0x7001,
    SHARED_MEMORY_ACCESS_DENIED = 0x7002
};

//============================================================================
// Message Header
//============================================================================

#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;          // Must be PLUGIN_IPC_MAGIC
    uint32_t version;        // Protocol version
    uint32_t messageType;    // Cast to MessageType
    uint64_t correlationId;  // Matches requests to responses
    uint32_t dataSize;       // Size of payload following this header
    
    MessageHeader()
        : magic(PLUGIN_IPC_MAGIC)
        , version(PLUGIN_IPC_VERSION)
        , messageType(0)
        , correlationId(0)
        , dataSize(0)
    {}
    
    bool IsValid() const {
        return magic == PLUGIN_IPC_MAGIC &&
               version == PLUGIN_IPC_VERSION &&
               dataSize <= MAX_MESSAGE_SIZE;
    }
};
#pragma pack(pop)

static_assert(sizeof(MessageHeader) == 24, "MessageHeader size must be 24 bytes");

//============================================================================
// Request: Thumbnail Generation
//============================================================================

#pragma pack(push, 1)
struct ThumbnailRequest {
    // File information
    uint32_t filePathLength;     // Length of file path in wide characters
    uint32_t targetWidth;        // Target thumbnail width
    uint32_t targetHeight;       // Target thumbnail height
    uint32_t flags;              // Decode flags (future use)
    uint32_t timeoutMs;          // Request timeout in milliseconds
    
    // Shared memory info (for large files)
    uint32_t useSharedMemory;    // 0 = inline data, 1 = shared memory
    uint64_t sharedMemorySize;   // Size of file data in shared memory
    char sharedMemoryName[256];  // Name of shared memory section
    
    // Inline data (for small files)
    uint32_t inlineDataSize;     // Size of inline file data
    // Followed by:
    // - wchar_t filePath[filePathLength]
    // - uint8_t inlineData[inlineDataSize] (if useSharedMemory == 0)
    
    ThumbnailRequest()
        : filePathLength(0)
        , targetWidth(0)
        , targetHeight(0)
        , flags(0)
        , timeoutMs(DEFAULT_REQUEST_TIMEOUT)
        , useSharedMemory(0)
        , sharedMemorySize(0)
        , inlineDataSize(0)
    {
        memset(sharedMemoryName, 0, sizeof(sharedMemoryName));
    }
};
#pragma pack(pop)

//============================================================================
// Response: Thumbnail Result
//============================================================================

#pragma pack(push, 1)
struct ThumbnailResponse {
    uint32_t resultCode;         // IPCErrorCode
    uint32_t width;              // Actual thumbnail width
    uint32_t height;             // Actual thumbnail height
    uint32_t pixelFormat;        // 0 = BGRA32, 1 = RGBA32
    uint32_t stride;             // Bytes per row
    uint64_t decodeTimeUs;       // Time taken in microseconds
    
    // Bitmap data
    uint32_t useSharedMemory;    // 0 = inline, 1 = shared memory
    char sharedMemoryName[256];  // Shared memory section name
    uint32_t bitmapDataSize;     // Size of bitmap data
    // Followed by:
    // - uint8_t bitmapData[bitmapDataSize] (if useSharedMemory == 0)
    
    ThumbnailResponse()
        : resultCode(static_cast<uint32_t>(IPCErrorCode::SUCCESS))
        , width(0)
        , height(0)
        , pixelFormat(0)
        , stride(0)
        , decodeTimeUs(0)
        , useSharedMemory(0)
        , bitmapDataSize(0)
    {
        memset(sharedMemoryName, 0, sizeof(sharedMemoryName));
    }
};
#pragma pack(pop)

//============================================================================
// Error Response
//============================================================================

#pragma pack(push, 1)
struct ErrorResponse {
    uint32_t errorCode;          // IPCErrorCode
    uint32_t messageLength;      // Length of error message
    // Followed by:
    // - char errorMessage[messageLength]
    
    ErrorResponse()
        : errorCode(static_cast<uint32_t>(IPCErrorCode::SUCCESS))
        , messageLength(0)
    {}
};
#pragma pack(pop)

//============================================================================
// Heartbeat
//============================================================================

#pragma pack(push, 1)
struct HeartbeatMessage {
    uint64_t timestamp;          // High-resolution timestamp
    uint32_t processId;          // Process ID of sender
    
    HeartbeatMessage()
        : timestamp(0)
        , processId(0)
    {}
};
#pragma pack(pop)

//============================================================================
// Can Decode Query
//============================================================================

#pragma pack(push, 1)
struct CanDecodeQuery {
    uint32_t filePathLength;     // Length of file path
    uint32_t hasHeader;          // 0 = path only, 1 = include file header
    uint32_t headerSize;         // Size of file header data
    // Followed by:
    // - wchar_t filePath[filePathLength]
    // - uint8_t headerData[headerSize] (if hasHeader == 1)
    
    CanDecodeQuery()
        : filePathLength(0)
        , hasHeader(0)
        , headerSize(0)
    {}
};
#pragma pack(pop)

//============================================================================
// Can Decode Response
//============================================================================

#pragma pack(push, 1)
struct CanDecodeResponse {
    uint32_t canDecode;          // 0 = no, 1 = yes
    uint32_t confidence;         // 0-100 (percentage)
    
    CanDecodeResponse()
        : canDecode(0)
        , confidence(0)
    {}
};
#pragma pack(pop)

//============================================================================
// Plugin Info Response
//============================================================================

#pragma pack(push, 1)
struct PluginInfoResponse {
    uint32_t apiVersion;
    char pluginId[64];
    char pluginName[128];
    char pluginVersion[32];
    char vendor[128];
    uint32_t capabilities;
    uint32_t extensionCount;
    // Followed by:
    // - char extensions[extensionCount][16]  (e.g., ".webp", ".avif")
    
    PluginInfoResponse()
        : apiVersion(0)
        , capabilities(0)
        , extensionCount(0)
    {
        memset(pluginId, 0, sizeof(pluginId));
        memset(pluginName, 0, sizeof(pluginName));
        memset(pluginVersion, 0, sizeof(pluginVersion));
        memset(vendor, 0, sizeof(vendor));
    }
};
#pragma pack(pop)

//============================================================================
// Helper Functions
//============================================================================

inline const char* MessageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::REQUEST_THUMBNAIL: return "REQUEST_THUMBNAIL";
        case MessageType::RESPONSE_THUMBNAIL: return "RESPONSE_THUMBNAIL";
        case MessageType::ERROR_RESPONSE: return "ERROR_RESPONSE";
        case MessageType::SHUTDOWN: return "SHUTDOWN";
        case MessageType::HEARTBEAT_PING: return "HEARTBEAT_PING";
        case MessageType::HEARTBEAT_PONG: return "HEARTBEAT_PONG";
        case MessageType::GET_PLUGIN_INFO: return "GET_PLUGIN_INFO";
        case MessageType::PLUGIN_INFO_RESPONSE: return "PLUGIN_INFO_RESPONSE";
        case MessageType::CAN_DECODE_QUERY: return "CAN_DECODE_QUERY";
        case MessageType::CAN_DECODE_RESPONSE: return "CAN_DECODE_RESPONSE";
        default: return "UNKNOWN";
    }
}

inline const char* ErrorCodeToString(IPCErrorCode code) {
    switch (code) {
        case IPCErrorCode::SUCCESS: return "SUCCESS";
        case IPCErrorCode::INVALID_MAGIC: return "INVALID_MAGIC";
        case IPCErrorCode::INVALID_VERSION: return "INVALID_VERSION";
        case IPCErrorCode::INVALID_MESSAGE_TYPE: return "INVALID_MESSAGE_TYPE";
        case IPCErrorCode::MESSAGE_TOO_LARGE: return "MESSAGE_TOO_LARGE";
        case IPCErrorCode::MALFORMED_MESSAGE: return "MALFORMED_MESSAGE";
        case IPCErrorCode::PIPE_CONNECTION_FAILED: return "PIPE_CONNECTION_FAILED";
        case IPCErrorCode::PIPE_WRITE_FAILED: return "PIPE_WRITE_FAILED";
        case IPCErrorCode::PIPE_READ_FAILED: return "PIPE_READ_FAILED";
        case IPCErrorCode::PIPE_BROKEN: return "PIPE_BROKEN";
        case IPCErrorCode::REQUEST_TIMEOUT: return "REQUEST_TIMEOUT";
        case IPCErrorCode::HEARTBEAT_TIMEOUT: return "HEARTBEAT_TIMEOUT";
        case IPCErrorCode::PROCESS_SPAWN_FAILED: return "PROCESS_SPAWN_FAILED";
        case IPCErrorCode::PROCESS_TERMINATED: return "PROCESS_TERMINATED";
        case IPCErrorCode::PROCESS_CRASHED: return "PROCESS_CRASHED";
        case IPCErrorCode::OUT_OF_MEMORY: return "OUT_OF_MEMORY";
        case IPCErrorCode::RESOURCE_LIMIT_EXCEEDED: return "RESOURCE_LIMIT_EXCEEDED";
        case IPCErrorCode::PLUGIN_LOAD_FAILED: return "PLUGIN_LOAD_FAILED";
        case IPCErrorCode::PLUGIN_INIT_FAILED: return "PLUGIN_INIT_FAILED";
        case IPCErrorCode::PLUGIN_DECODE_FAILED: return "PLUGIN_DECODE_FAILED";
        case IPCErrorCode::PLUGIN_UNSUPPORTED_FORMAT: return "PLUGIN_UNSUPPORTED_FORMAT";
        case IPCErrorCode::SHARED_MEMORY_CREATE_FAILED: return "SHARED_MEMORY_CREATE_FAILED";
        case IPCErrorCode::SHARED_MEMORY_MAP_FAILED: return "SHARED_MEMORY_MAP_FAILED";
        case IPCErrorCode::SHARED_MEMORY_ACCESS_DENIED: return "SHARED_MEMORY_ACCESS_DENIED";
        default: return "UNKNOWN_ERROR";
    }
}

} // namespace IPC
} // namespace DarkThumbs
