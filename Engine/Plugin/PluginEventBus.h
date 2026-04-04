// PluginEventBus.h — Publish-subscribe event bus for plugin communication
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a typed event bus for plugins to publish and subscribe to events
// without direct coupling — supports decode-complete, cache-hit, error events.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PluginEventBusConfig
{
    bool enabled = true;
    uint32_t maxSubscriptionsPerEvent = 32;
    std::string label = "PluginEventBus";
};

class PluginEventBus
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    PluginEventBusConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    using EventCallback = std::function<void(const std::string&)>;

    bool Subscribe(const std::string& eventType, EventCallback cb)
    {
        auto& subs = m_subscriptions[eventType];
        if (subs.size() >= m_config.maxSubscriptionsPerEvent)
            return false;
        subs.push_back(std::move(cb));
        return true;
    }

    void Publish(const std::string& eventType, const std::string& payload)
    {
        auto it = m_subscriptions.find(eventType);
        if (it != m_subscriptions.end()) {
            for (auto& cb : it->second)
                cb(payload);
        }
    }

    size_t GetSubscriberCount(const std::string& eventType) const
    {
        auto it = m_subscriptions.find(eventType);
        return (it != m_subscriptions.end()) ? it->second.size() : 0;
    }

  private:
    bool m_initialized = false;
    PluginEventBusConfig m_config;
    std::unordered_map<std::string, std::vector<EventCallback>> m_subscriptions;
};

}  // namespace Engine
}  // namespace ExplorerLens
