// CacheInvalidationBroadcaster.h — Broadcasts cache invalidation events
// Copyright (c) 2026 ExplorerLens Project
//
// Notifies all cache consumers when entries are invalidated — supports
// file-change, manual, and TTL-based invalidation with subscriber pattern.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

struct CacheInvalidationBroadcasterConfig {
    bool enabled = true;
    uint32_t maxSubscribers = 32;
    std::string label = "CacheInvalidationBroadcaster";
};

class CacheInvalidationBroadcaster {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheInvalidationBroadcasterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Reason : uint8_t { FileChanged, TTLExpired, ManualPurge, MemoryPressure };

    using Callback = std::function<void(const std::string&, Reason)>;

    bool Subscribe(Callback cb) {
        if (m_subscribers.size() >= m_config.maxSubscribers) return false;
        m_subscribers.push_back(std::move(cb));
        return true;
    }

    void Broadcast(const std::string& key, Reason reason) {
        for (auto& cb : m_subscribers) cb(key, reason);
    }

    size_t GetSubscriberCount() const { return m_subscribers.size(); }

private:
    bool m_initialized = false;
    CacheInvalidationBroadcasterConfig m_config;
    std::vector<Callback> m_subscribers;
};

}
} // namespace ExplorerLens::Engine
