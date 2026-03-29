// CrossPlatformUIBridge.h — Cross-Platform UI Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges Electron renderer IPC messages to the C++ preview pipeline,
// marshalling thumbnail requests through a structured message channel.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class UIBridgeEvent {
    ThumbnailRequest, ThumbnailReady, ThumbnailError,
    PreviewOpen, PreviewClose, SettingsUpdate
};

struct UIMessage {
    UIBridgeEvent   event;
    std::string     channel;
    std::string     payload;
    uint64_t        requestId = 0;
};

class CrossPlatformUIBridge {
public:
    using MessageHandler = std::function<void(const UIMessage&)>;
    using SendFn         = std::function<void(const UIMessage&)>;

    CrossPlatformUIBridge() = default;

    void SetSendFunction(SendFn fn) { m_sendFn = fn; }
    void SetMessageHandler(UIBridgeEvent event, MessageHandler handler) {
        m_handlers[static_cast<int>(event)] = handler;
    }

    bool Open(const std::string& channelName) {
        if (channelName.empty()) return false;
        m_channelName = channelName;
        m_open = true;
        return true;
    }

    void Close() { m_open = false; }
    bool IsOpen() const { return m_open; }

    bool Send(const UIMessage& msg) {
        if (!m_open) return false;
        if (m_sendFn) m_sendFn(msg);
        return true;
    }

    bool Dispatch(const UIMessage& msg) {
        int idx = static_cast<int>(msg.event);
        if (idx >= 0 && idx < static_cast<int>(m_handlers.size()) && m_handlers[idx]) {
            m_handlers[idx](msg);
            return true;
        }
        return false;
    }

    bool PostThumbnailReady(uint64_t reqId, const std::vector<uint8_t>& pngData) {
        UIMessage msg;
        msg.event     = UIBridgeEvent::ThumbnailReady;
        msg.channel   = m_channelName;
        msg.requestId = reqId;
        msg.payload   = std::to_string(pngData.size()) + " bytes";
        return Send(msg);
    }

    const std::string& GetChannelName() const { return m_channelName; }

private:
    std::string             m_channelName;
    bool                    m_open = false;
    SendFn                  m_sendFn;
    std::vector<MessageHandler> m_handlers{6};
};

}} // namespace ExplorerLens::Engine
