// LensServer.h — Minimal HTTP REST thumbnail server
// Copyright (c) 2026 ExplorerLens Project
//
// Exposes a single-endpoint HTTP/1.1 server over WinHTTP/TCP:
//   GET /thumbnail?path=<url-encoded-path>&size=<px>
//       → 200 image/png  with the generated thumbnail, or
//         400/404/500 with JSON error body
//   GET /health
//       → 200 {"status":"ok","version":"X.Y.Z"}
//   GET /metrics
//       → 200 JSON cache metrics snapshot
//
// Relies only on Winsock2 — no third-party HTTP library dependency.
// Intended for headless / container use (CI thumbnail generation, REST API).
//
#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

namespace ExplorerLens {
namespace Server {

struct ServerConfig {
    uint16_t    port          = 8765;
    std::string bindAddress   = "127.0.0.1";   // Use "0.0.0.0" for all interfaces
    uint32_t    maxConnections = 16;
    uint32_t    defaultSize    = 256;
    uint32_t    maxSize        = 1024;
    bool        gpuEnabled     = true;
    bool        verboseLog     = false;
    std::string cacheDir;                       // Empty → default %LOCALAPPDATA% path
};

class LensServer {
public:
    explicit LensServer(ServerConfig config = {});
    ~LensServer();

    // Start listening on config.port; non-blocking (spawns accept thread)
    bool Start();

    // Graceful shutdown — waits for in-flight requests to complete
    void Stop();

    bool IsRunning() const { return m_running.load(); }
    uint16_t BoundPort() const { return m_config.port; }

    LensServer(const LensServer&) = delete;
    LensServer& operator=(const LensServer&) = delete;

private:
    void AcceptLoop();
    void HandleConnection(uintptr_t clientSocket);

    // Request parsing
    struct HttpRequest {
        std::string method;
        std::string path;
        std::string query;
    };
    static HttpRequest ParseRequest(const char* buf, size_t len);
    static std::string UrlDecode(const std::string& encoded);
    static std::string GetQueryParam(const std::string& query, const std::string& key);

    // Response helpers
    static std::string MakeResponse(int status, const std::string& contentType,
                                    const std::string& body);
    static std::string JsonError(int status, const std::string& message);

    // Route handlers — return full HTTP response string
    std::string HandleThumbnail(const HttpRequest& req);
    std::string HandleHealth(const HttpRequest& req);
    std::string HandleMetrics(const HttpRequest& req);

    ServerConfig        m_config;
    std::atomic<bool>   m_running{ false };
    std::thread         m_acceptThread;
    uintptr_t           m_listenSocket{ static_cast<uintptr_t>(~0) };
};

} // namespace Server
} // namespace ExplorerLens
