// LensServer.cpp — Minimal HTTP REST thumbnail server implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "LensServer.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
using SocketType = SOCKET;
static constexpr SocketType INVALID_SOCK = INVALID_SOCKET;
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
using SocketType = int;
static constexpr SocketType INVALID_SOCK = -1;
#endif

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace ExplorerLens {
namespace Server {

// ── Version string (updated by Bump-Version.ps1) ─────────────────────────────
static constexpr const char* SERVER_VERSION = "35.5.0";

// ── Construction / destruction ────────────────────────────────────────────────
LensServer::LensServer(ServerConfig config)
    : m_config(std::move(config)) {
#ifdef _WIN32
    WSADATA wsa{};
    ::WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

LensServer::~LensServer() {
    Stop();
#ifdef _WIN32
    ::WSACleanup();
#endif
}

// ── Start ─────────────────────────────────────────────────────────────────────
bool LensServer::Start() {
    if (m_running.exchange(true)) return true;

    SocketType sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCK) {
        m_running.store(false);
        return false;
    }

    int yes = 1;
#ifdef _WIN32
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&yes), sizeof(yes));
#else
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(m_config.port);
    ::inet_pton(AF_INET, m_config.bindAddress.c_str(), &addr.sin_addr);

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 ||
        ::listen(sock, static_cast<int>(m_config.maxConnections)) != 0) {
        m_running.store(false);
#ifdef _WIN32
        ::closesocket(sock);
#else
        ::close(sock);
#endif
        return false;
    }

    m_listenSocket = static_cast<uintptr_t>(sock);

    m_acceptThread = std::thread([this] { AcceptLoop(); });

    std::cout << "[LensServer] Listening on "
              << m_config.bindAddress << ":" << m_config.port << "\n";
    return true;
}

// ── Stop ──────────────────────────────────────────────────────────────────────
void LensServer::Stop() {
    m_running.store(false);
    SocketType sock = static_cast<SocketType>(m_listenSocket);
    if (sock != INVALID_SOCK) {
#ifdef _WIN32
        ::closesocket(sock);
#else
        ::close(sock);
#endif
        m_listenSocket = static_cast<uintptr_t>(INVALID_SOCK);
    }
    if (m_acceptThread.joinable()) m_acceptThread.join();
}

// ── AcceptLoop ────────────────────────────────────────────────────────────────
void LensServer::AcceptLoop() {
    SocketType listen = static_cast<SocketType>(m_listenSocket);
    while (m_running.load()) {
        sockaddr_in clientAddr{};
        socklen_t   clientLen = sizeof(clientAddr);
        SocketType  client    = ::accept(listen,
                                          reinterpret_cast<sockaddr*>(&clientAddr),
                                          &clientLen);
        if (client == INVALID_SOCK) break;

        // Each connection handled on its own short-lived thread
        std::thread([this, client] {
            HandleConnection(static_cast<uintptr_t>(client));
        }).detach();
    }
}

// ── HandleConnection ──────────────────────────────────────────────────────────
void LensServer::HandleConnection(uintptr_t socketHandle) {
    SocketType sock = static_cast<SocketType>(socketHandle);
    char buf[4096] = {};

#ifdef _WIN32
    int received = ::recv(sock, buf, sizeof(buf) - 1, 0);
#else
    ssize_t received = ::recv(sock, buf, sizeof(buf) - 1, 0);
#endif
    if (received <= 0) {
#ifdef _WIN32
        ::closesocket(sock);
#else
        ::close(sock);
#endif
        return;
    }

    HttpRequest req = ParseRequest(buf, static_cast<size_t>(received));
    std::string response;

    if (req.method == "GET") {
        if (req.path == "/health" || req.path == "/healthz") {
            response = HandleHealth(req);
        } else if (req.path == "/metrics") {
            response = HandleMetrics(req);
        } else if (req.path == "/thumbnail") {
            response = HandleThumbnail(req);
        } else {
            response = JsonError(404, "not found: " + req.path);
        }
    } else {
        response = JsonError(405, "method not allowed");
    }

    if (m_config.verboseLog) {
        std::cout << "[LensServer] " << req.method << " " << req.path
                  << (req.query.empty() ? "" : "?" + req.query) << "\n";
    }

#ifdef _WIN32
    ::send(sock, response.c_str(), static_cast<int>(response.size()), 0);
    ::closesocket(sock);
#else
    ::send(sock, response.c_str(), response.size(), 0);
    ::close(sock);
#endif
}

// ── Request parsing ───────────────────────────────────────────────────────────
LensServer::HttpRequest LensServer::ParseRequest(const char* buf, size_t /*len*/) {
    HttpRequest req;
    std::istringstream ss(buf);
    std::string requestLine;
    if (!std::getline(ss, requestLine)) return req;

    std::istringstream rl(requestLine);
    std::string url;
    rl >> req.method >> url;

    auto qpos = url.find('?');
    if (qpos != std::string::npos) {
        req.path  = url.substr(0, qpos);
        req.query = url.substr(qpos + 1);
    } else {
        req.path = url;
    }
    return req;
}

std::string LensServer::UrlDecode(const std::string& encoded) {
    std::string out;
    out.reserve(encoded.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            char hex[3] = { encoded[i+1], encoded[i+2], '\0' };
            out += static_cast<char>(std::strtol(hex, nullptr, 16));
            i   += 2;
        } else if (encoded[i] == '+') {
            out += ' ';
        } else {
            out += encoded[i];
        }
    }
    return out;
}

std::string LensServer::GetQueryParam(const std::string& query, const std::string& key) {
    std::string search = key + "=";
    auto pos = query.find(search);
    if (pos == std::string::npos) return {};
    pos += search.size();
    auto end = query.find('&', pos);
    return UrlDecode(query.substr(pos, end == std::string::npos ? std::string::npos : end - pos));
}

// ── Response helpers ──────────────────────────────────────────────────────────
std::string LensServer::MakeResponse(int status, const std::string& contentType,
                                      const std::string& body) {
    std::string reason = status == 200 ? "OK"
                       : status == 400 ? "Bad Request"
                       : status == 404 ? "Not Found"
                       : status == 405 ? "Method Not Allowed"
                                       : "Internal Server Error";
    std::ostringstream ss;
    ss << "HTTP/1.1 " << status << " " << reason << "\r\n"
       << "Content-Type: "   << contentType << "\r\n"
       << "Content-Length: " << body.size()  << "\r\n"
       << "Connection: close\r\n"
       << "\r\n"
       << body;
    return ss.str();
}

std::string LensServer::JsonError(int status, const std::string& message) {
    return MakeResponse(status, "application/json",
                        "{\"error\":\"" + message + "\",\"status\":" +
                        std::to_string(status) + "}");
}

// ── Route handlers ────────────────────────────────────────────────────────────
std::string LensServer::HandleHealth(const HttpRequest& /*req*/) {
    return MakeResponse(200, "application/json",
        std::string("{\"status\":\"ok\",\"version\":\"") + SERVER_VERSION + "\"}");
}

std::string LensServer::HandleMetrics(const HttpRequest& /*req*/) {
    // Placeholder — wired to CacheMetricsCollector::LastSnapshot() in production
    return MakeResponse(200, "application/json",
        "{\"l1HitRate\":0.0,\"l2HitRate\":0.0,\"totalRequests\":0,\"note\":\"metrics not yet wired\"}");
}

std::string LensServer::HandleThumbnail(const HttpRequest& req) {
    std::string path  = GetQueryParam(req.query, "path");
    std::string sizeS = GetQueryParam(req.query, "size");

    if (path.empty()) {
        return JsonError(400, "missing required parameter 'path'");
    }

    uint32_t size = 256;
    if (!sizeS.empty()) {
        try { size = static_cast<uint32_t>(std::stoul(sizeS)); } catch (...) {}
        size = (std::min)(size, m_config.maxSize);
        size = (std::max)(size, 16u);
    }

    // Integration point: call LensEngine API here.
    // In headless/dry-run contexts (no LENSShell.dll), the server returns 501
    // with a JSON stub so CI pipelines can still exercise the HTTP layer.
    return MakeResponse(501, "application/json",
        "{\"error\":\"engine not loaded\",\"path\":\"" + path +
        "\",\"size\":" + std::to_string(size) +
        ",\"note\":\"link LensServer against ExplorerLensEngine.lib for full decode\"}");
}

} // namespace Server
} // namespace ExplorerLens
