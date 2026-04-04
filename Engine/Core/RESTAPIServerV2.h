// RESTAPIServerV2.h — REST API Server v2
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight in-process HTTP/1.1 REST server for thumbnail and metadata query endpoints.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct RASSv2Config
{
    std::string listenAddress = "127.0.0.1";
    uint16_t port = 8080;
    bool corsEnabled = false;
};

struct RASSv2Request
{
    std::string method;  // GET, POST, etc.
    std::string path;
    std::vector<uint8_t> body;
};

struct RASSv2Response
{
    uint32_t statusCode = 200;
    std::string contentType = "application/json";
    std::vector<uint8_t> body;
};

using RASSv2Handler = std::function<RASSv2Response(const RASSv2Request&)>;

class RESTAPIServerV2
{
  public:
    explicit RESTAPIServerV2(const RASSv2Config& config) : m_config(config) {}

    void Route(const std::string& method, const std::string& path, RASSv2Handler handler)
    {
        m_routes[method + " " + path] = std::move(handler);
    }
    bool Start()
    {
        m_running = true;
        return true;
    }
    bool Stop()
    {
        m_running = false;
        return true;
    }

    RASSv2Response Dispatch(const RASSv2Request& req)
    {
        auto it = m_routes.find(req.method + " " + req.path);
        if (it == m_routes.end())
            return {404, "application/json", {}};
        return it->second(req);
    }
    bool IsRunning() const
    {
        return m_running;
    }

  private:
    RASSv2Config m_config;
    bool m_running = false;
    std::unordered_map<std::string, RASSv2Handler> m_routes;
};

}  // namespace Engine
}  // namespace ExplorerLens
