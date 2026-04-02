#include "LocalApiService.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../Worker/ipc_transport.h"

namespace ExplorerLens::Service {

namespace {
std::string MakeRouteKey(const std::string& method, const std::string& path) {
    return method + " " + path;
}
}

LocalApiService::LocalApiService(ServiceConfig config)
    : m_config(std::move(config)), m_isRunning(false) {
}

LocalApiService::~LocalApiService() {
    Stop();
}

void LocalApiService::Start() {
    if (m_isRunning) {
        return;
    }

    m_isRunning = true;

    if (m_config.transport == ServiceTransport::HttpLocalhost) {
        m_listenerThread = std::make_unique<std::thread>(&LocalApiService::ListenHttp, this);
    } else {
        m_listenerThread = std::make_unique<std::thread>(&LocalApiService::ListenPipe, this);
    }
}

void LocalApiService::Stop() {
    m_isRunning = false;

    if (m_listenerThread && m_listenerThread->joinable()) {
        m_listenerThread->join();
    }

    m_listenerThread.reset();
}

void LocalApiService::RegisterRoute(const std::string& method,
                                    const std::string& path,
                                    std::function<std::string()> handler) {
    std::scoped_lock lock(m_routeMutex);
    m_routes[MakeRouteKey(method, path)] = std::move(handler);
}

void LocalApiService::ListenHttp() const {
    // Minimal Phase R1 runtime: keep service alive and dispatch internal handlers.
    // A full HTTP parser/router is Phase R3.
    while (m_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void LocalApiService::ListenPipe() const {
    IPC::TransportConfig transport{};
    transport.pipeName = m_config.pipeName.empty() ? L"\\.\\pipe\\ExplorerLens.LocalApi" : m_config.pipeName;

    IPC::IPCServer server(transport);
    if (!server.Start()) {
        return;
    }

    while (m_isRunning) {
        auto conn = server.Accept(250);
        if (!conn) {
            continue;
        }

        IPC::MessageHeader header{};
        header.messageType = IPC::MessageType::REQUEST_PING; // placeholder; overwritten by Receive()
        std::vector<uint8_t> payload;
        const IPC::IPCStatus status = conn->Receive(header, payload, 1000);
        if (status != IPC::IPCStatus::OK) {
            continue;
        }

        // Lightweight status response for initial integration.
        const std::string body = R"({"service":"ExplorerLens.LocalApi","status":"ok"})";
        const IPC::MessageHeader response = IPC::CreateHeader(
            IPC::MessageType::RESPONSE_PONG,
            header.correlationId,
            static_cast<uint32_t>(body.size())
        );
        conn->Send(response, body.data());
    }

    server.Stop();
}

} // namespace ExplorerLens::Service

