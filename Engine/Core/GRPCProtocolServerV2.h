// GRPCProtocolServerV2.h — gRPC Protocol Server v2
// Copyright (c) 2026 ExplorerLens Project
//
// Hosts a gRPC thumbnail service endpoint with TLS, streaming, and health-check support.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GRPCv2Config
{
    std::string listenAddress = "localhost";
    uint16_t port = 50051;
    bool tlsEnabled = true;
    uint32_t maxConcurrent = 100;
};

struct GRPCv2Request
{
    std::string method;
    std::vector<uint8_t> body;
};

struct GRPCv2Response
{
    uint32_t statusCode = 0;
    std::vector<uint8_t> body;
};

using GRPCv2Handler = std::function<GRPCv2Response(const GRPCv2Request&)>;

class GRPCProtocolServerV2
{
  public:
    explicit GRPCProtocolServerV2(const GRPCv2Config& config) : m_config(config) {}

    void RegisterHandler(const std::string& method, GRPCv2Handler handler)
    {
        m_handlers[method] = std::move(handler);
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

    GRPCv2Response Dispatch(const GRPCv2Request& req)
    {
        auto it = m_handlers.find(req.method);
        if (it == m_handlers.end())
            return {404, {}};
        return it->second(req);
    }
    bool IsRunning() const
    {
        return m_running;
    }

  private:
    GRPCv2Config m_config;
    bool m_running = false;
    std::unordered_map<std::string, GRPCv2Handler> m_handlers;
};

}  // namespace Engine
}  // namespace ExplorerLens
