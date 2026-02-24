#pragma once
//==============================================================================
// NetworkProviderEngine
// Handles thumbnail generation for network paths (UNC, SMB, WebDAV, FTP).
// Includes timeout management, credential caching, and retry logic.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class NetworkProtocol : uint8_t {
    UNC    = 0,
    SMB    = 1,
    WebDAV = 2,
    FTP    = 3,
    SFTP   = 4,
    HTTP   = 5,
    ProtocolCount = 6
};

enum class NetworkStatus : uint8_t {
    Connected    = 0,
    Disconnected = 1,
    Timeout      = 2,
    AuthRequired = 3,
    Error        = 4
};

struct NetworkPath {
    std::wstring fullPath;
    std::wstring server;
    std::wstring share;
    std::wstring relativePath;
    NetworkProtocol protocol = NetworkProtocol::UNC;
};

struct NetworkThumbnailResult {
    bool success = false;
    NetworkStatus status = NetworkStatus::Disconnected;
    double latencyMs = 0.0;
    uint32_t retryCount = 0;
    uint64_t bytesTransferred = 0;
    std::wstring errorMessage;
};

class NetworkProviderEngine {
public:
    NetworkProviderEngine();

    NetworkPath ParsePath(const std::wstring& path) const;
    NetworkThumbnailResult FetchThumbnail(const std::wstring& networkPath);
    NetworkStatus CheckConnectivity(const std::wstring& server) const;

    void SetTimeout(uint32_t timeoutMs) { m_timeoutMs = timeoutMs; }
    uint32_t GetTimeout() const { return m_timeoutMs; }
    void SetMaxRetries(uint32_t retries) { m_maxRetries = retries; }
    uint32_t GetMaxRetries() const { return m_maxRetries; }

    static bool IsNetworkPath(const std::wstring& path);
    static NetworkProtocol DetectProtocol(const std::wstring& path);
    static const wchar_t* GetProtocolName(NetworkProtocol protocol);
    static const wchar_t* GetStatusName(NetworkStatus status);
    static uint32_t GetProtocolCount() { return static_cast<uint32_t>(NetworkProtocol::ProtocolCount); }

private:
    uint32_t m_timeoutMs = 5000;
    uint32_t m_maxRetries = 3;
};

}} // namespace ExplorerLens::Engine

