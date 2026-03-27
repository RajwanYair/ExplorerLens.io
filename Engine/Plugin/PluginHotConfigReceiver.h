// PluginHotConfigReceiver.h — Plugin Runtime Hot-Config Push Receiver
// Copyright (c) 2026 ExplorerLens Project
//
// Receives runtime configuration pushes from the manager without plugin restart — updates feature flags and limits live.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct HotConfigPayload { std::string pluginId; std::string key; std::string value; uint64_t revision; };
using HotConfigCallback = std::function<void(const HotConfigPayload&)>;
class PluginHotConfigReceiver {
public:
    void   Subscribe(const std::string& key, HotConfigCallback cb) {
        m_callbacks[key].push_back(cb);
    }
    void   Push(HotConfigPayload payload) {
        m_lastRevision = payload.revision;
        auto it = m_callbacks.find(payload.key);
        if (it != m_callbacks.end()) for (auto& cb : it->second) cb(payload);
    }
    uint64_t LastRevision() const { return m_lastRevision; }
    size_t   ListenerCount(const std::string& key) const {
        auto it = m_callbacks.find(key);
        return it != m_callbacks.end() ? it->second.size() : 0;
    }
private:
    uint64_t m_lastRevision = 0;
    std::unordered_map<std::string, std::vector<HotConfigCallback>> m_callbacks;
};

} // namespace Engine
} // namespace ExplorerLens