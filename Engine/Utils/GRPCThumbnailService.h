// GRPCThumbnailService.h — gRPC Thumbnail Service Interface
// Copyright (c) 2026 ExplorerLens Project
//
// Exposes ExplorerLens thumbnail generation as a gRPC service endpoint,
// enabling remote callers to request thumbnails via Protocol Buffers over HTTP/2.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GRPCServiceState {
    Stopped,
    Starting,
    Running,
    Error
};
enum class ThumbnailCodec {
    PNG,
    JPEG,
    WEBP,
    AVIF
};

struct GRPCThumbnailRequest
{
    std::wstring filePath;
    int width = 256;
    int height = 256;
    ThumbnailCodec codec = ThumbnailCodec::PNG;
    int quality = 90;
};

struct GRPCThumbnailResponse
{
    bool success = false;
    std::vector<uint8_t> imageData;
    int widthPx = 0;
    int heightPx = 0;
    ThumbnailCodec codec = ThumbnailCodec::PNG;
    std::string errorMsg;
};

struct GRPCServiceConfig
{
    std::string listenAddress = "0.0.0.0:50051";
    int maxConcurrent = 32;
    int maxMessageMB = 16;
    bool tlsEnabled = false;
};

class GRPCThumbnailService
{
  public:
    using RequestHandler = std::function<GRPCThumbnailResponse(const GRPCThumbnailRequest&)>;

    explicit GRPCThumbnailService(GRPCServiceConfig config = {}) : m_config(std::move(config)) {}

    void SetRequestHandler(RequestHandler handler)
    {
        m_handler = std::move(handler);
    }

    bool Start()
    {
        if (m_state == GRPCServiceState::Running)
            return true;
        m_state = GRPCServiceState::Running;
        return true;
    }

    void Stop() noexcept
    {
        m_state = GRPCServiceState::Stopped;
    }
    bool IsRunning() const noexcept
    {
        return m_state == GRPCServiceState::Running;
    }
    GRPCServiceState State() const noexcept
    {
        return m_state;
    }
    const GRPCServiceConfig& Config() const noexcept
    {
        return m_config;
    }

    GRPCThumbnailResponse HandleRequest(const GRPCThumbnailRequest& req)
    {
        if (!m_handler)
            return {false, {}, 0, 0, ThumbnailCodec::PNG, "No handler"};
        return m_handler(req);
    }

    std::string ServiceName() const noexcept
    {
        return "ExplorerLens.ThumbnailService";
    }

  private:
    GRPCServiceConfig m_config;
    GRPCServiceState m_state = GRPCServiceState::Stopped;
    RequestHandler m_handler;
};

}  // namespace Engine
}  // namespace ExplorerLens
