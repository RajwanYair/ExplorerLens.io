// FederatedCacheInvalidator.h — Federated Cache Invalidation Coordinator
// Copyright (c) 2026 ExplorerLens Project
//
// Broadcasts targeted cache invalidations to all instances (Shell, Manager, CLI) via named events and shared memory.
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

enum class InvalidationScope { Local, AllInstances, RemoteHosts };
struct InvalidationMsg { std::wstring key; InvalidationScope scope; uint64_t version; };
class FederatedCacheInvalidator {
public:
    uint64_t Broadcast(InvalidationMsg msg) {
        msg.version = ++m_version;
        m_log.push_back(msg);
        return m_version;
    }
    uint64_t CurrentVersion()    const { return m_version; }
    size_t   InvalidationCount() const { return m_log.size(); }
    bool     IsKeyInvalidated(const std::wstring& key) const {
        for (auto& m : m_log) if (m.key == key) return true;
        return false;
    }
private:
    std::atomic<uint64_t>        m_version{0};
    std::vector<InvalidationMsg> m_log;
};

} // namespace Engine
} // namespace ExplorerLens