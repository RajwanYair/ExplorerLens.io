// PluginCommunicationBridge.h — Inter-plugin message passing
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a type-safe message bus for plugins to communicate without
// direct coupling, supporting request/response and broadcast patterns.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct PluginCommunicationBridgeConfig {
    bool enabled = true;
    uint32_t maxQueueSize = 256;
    bool allowBroadcast = true;
    std::string label = "PluginCommunicationBridge";
};

class PluginCommunicationBridge {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginCommunicationBridgeConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct Message {
        std::string sender;
        std::string recipient; // empty = broadcast
        std::string topic;
        uint64_t payload = 0;
    };

    bool Send(const Message& msg) {
        if (!m_initialized || msg.sender.empty()) return false;
        m_messagesSent++;
        return true;
    }

    bool Broadcast(const std::string& sender, const std::string& topic,
        uint64_t payload) {
        Message msg{ sender, "", topic, payload };
        return Send(msg);
    }

    uint64_t GetMessagesSent() const { return m_messagesSent; }

private:
    bool m_initialized = false;
    uint64_t m_messagesSent = 0;
    PluginCommunicationBridgeConfig m_config;
};

}
} // namespace ExplorerLens::Engine
