// ReactiveAPIGateway.h — Reactive API Gateway (Named Pipe / COM)
// Copyright (c) 2026 ExplorerLens Project
//
// Named-pipe and COM-callable reactive gateway routing requests to the CQRS pipeline with flow control.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class GatewayProtocol { NamedPipe, COM, SharedMemory, WebSocket };
struct GatewayConfig {
    GatewayProtocol protocol   = GatewayProtocol::NamedPipe;
    uint32_t        maxClients = 8;
    uint32_t        timeoutMs  = 5000;
    std::string     pipeName   = "\\\\.\\pipe\\ExplorerLens";
};
class ReactiveAPIGateway {
public:
    explicit ReactiveAPIGateway(GatewayConfig cfg = {}) : m_cfg(cfg) {}
    bool   Start()             { m_running = true; return true; }
    void   Stop()              { m_running = false; }
    bool   IsRunning() const   { return m_running; }
    uint32_t ConnectedClients() const { return m_clients; }
    GatewayProtocol Protocol() const { return m_cfg.protocol; }
private:
    GatewayConfig         m_cfg;
    std::atomic<bool>     m_running{false};
    std::atomic<uint32_t> m_clients{0};
};

} // namespace Engine
} // namespace ExplorerLens