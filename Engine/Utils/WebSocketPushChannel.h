// WebSocketPushChannel.h — WebSocket Push Channel (Thumbnail Change Notifications)
// Copyright (c) 2026 ExplorerLens Project
//
// Manages WebSocket-based push notifications to connected clients when thumbnails
// are (re-)generated, invalidated, or annotations are updated in real time.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WSMessageType {
    ThumbnailReady,
    ThumbnailInvalidated,
    AnnotationUpdated,
    Ping,
    Pong,
    Close
};
enum class WSChannelState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

struct WSMessage
{
    WSMessageType type = WSMessageType::Ping;
    std::string payload;
    uint64_t clientId = 0;
    int64_t timestampMs = 0;

    std::string TypeName() const noexcept
    {
        switch (type) {
            case WSMessageType::ThumbnailReady:
                return "ThumbnailReady";
            case WSMessageType::ThumbnailInvalidated:
                return "ThumbnailInvalidated";
            case WSMessageType::AnnotationUpdated:
                return "AnnotationUpdated";
            case WSMessageType::Ping:
                return "Ping";
            case WSMessageType::Pong:
                return "Pong";
            case WSMessageType::Close:
                return "Close";
        }
        return "Unknown";
    }
};

struct WSChannelConfig
{
    uint16_t port = 8546;
    int maxClients = 64;
    int pingIntervalS = 30;
    int timeoutS = 60;
};

struct WSPushStats
{
    std::atomic<int> clientsConnected{0};
    std::atomic<int> messagesSent{0};
    std::atomic<int> messagesDropped{0};
};

using WSMessageCallback = std::function<void(uint64_t clientId, const WSMessage&)>;

class WebSocketPushChannel
{
  public:
    explicit WebSocketPushChannel(WSChannelConfig config = {}) : m_config(std::move(config)) {}

    void SetMessageCallback(WSMessageCallback cb)
    {
        m_callback = std::move(cb);
    }

    bool Start()
    {
        if (m_state == WSChannelState::Connected)
            return true;
        m_state = WSChannelState::Connected;
        return true;
    }
    void Stop() noexcept
    {
        m_state = WSChannelState::Disconnected;
    }
    bool IsRunning() const noexcept
    {
        return m_state == WSChannelState::Connected;
    }
    WSChannelState State() const noexcept
    {
        return m_state;
    }

    void Broadcast(const WSMessage& msg)
    {
        int sent = m_stats.clientsConnected.load();
        m_stats.messagesSent += sent;
        if (m_callback)
            m_callback(0, msg);
    }

    void SendTo(uint64_t clientId, const WSMessage& msg)
    {
        m_stats.messagesSent++;
        if (m_callback)
            m_callback(clientId, msg);
    }

    void SimulateClientConnect() noexcept
    {
        m_stats.clientsConnected++;
    }
    void SimulateClientDisconnect() noexcept
    {
        if (m_stats.clientsConnected > 0)
            m_stats.clientsConnected--;
    }

    int ClientCount() const noexcept
    {
        return m_stats.clientsConnected.load();
    }
    int MessagesSent() const noexcept
    {
        return m_stats.messagesSent.load();
    }
    const WSChannelConfig& Config() const noexcept
    {
        return m_config;
    }

  private:
    WSChannelConfig m_config;
    WSChannelState m_state = WSChannelState::Disconnected;
    WSPushStats m_stats;
    WSMessageCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
