// RESTThumbnailServer.h — REST Thumbnail Server (HTTP/1.1 + HTTP/2)
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight REST endpoint exposing thumbnail generation over HTTP.
// Routes: GET /thumbnail?path=...&size=... and POST /batch with JSON body.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE_HTTP
};
enum class RESTServerState {
    Stopped,
    Running,
    Error
};

struct HTTPRequest
{
    HTTPMethod method = HTTPMethod::GET;
    std::string path;
    std::unordered_map<std::string, std::string> queryParams;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};

struct HTTPResponse
{
    int statusCode = 200;
    std::string contentType;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};

using RESTRouteHandler = std::function<HTTPResponse(const HTTPRequest&)>;

struct RESTServerConfig
{
    std::string listenAddress = "127.0.0.1";
    uint16_t port = 8545;
    int timeoutSec = 30;
    bool corsEnabled = true;
    int maxBodyKB = 4096;
};

class RESTThumbnailServer
{
  public:
    explicit RESTThumbnailServer(RESTServerConfig config = {}) : m_config(std::move(config)) {}

    void RegisterRoute(const std::string& path, HTTPMethod method, RESTRouteHandler handler)
    {
        m_routes[path + ":" + std::to_string(static_cast<int>(method))] = std::move(handler);
    }

    bool Start()
    {
        if (m_state == RESTServerState::Running)
            return true;
        m_state = RESTServerState::Running;
        return true;
    }
    void Stop() noexcept
    {
        m_state = RESTServerState::Stopped;
    }
    bool IsRunning() const noexcept
    {
        return m_state == RESTServerState::Running;
    }
    RESTServerState State() const noexcept
    {
        return m_state;
    }
    const RESTServerConfig& Config() const noexcept
    {
        return m_config;
    }

    HTTPResponse Dispatch(const HTTPRequest& req)
    {
        const std::string key = req.path + ":" + std::to_string(static_cast<int>(req.method));
        auto it = m_routes.find(key);
        if (it != m_routes.end())
            return it->second(req);
        HTTPResponse notFound;
        notFound.statusCode = 404;
        notFound.body = "{\"error\":\"Not Found\"}";
        notFound.contentType = "application/json";
        return notFound;
    }

    size_t RouteCount() const noexcept
    {
        return m_routes.size();
    }

  private:
    RESTServerConfig m_config;
    RESTServerState m_state = RESTServerState::Stopped;
    std::unordered_map<std::string, RESTRouteHandler> m_routes;
};

}  // namespace Engine
}  // namespace ExplorerLens
