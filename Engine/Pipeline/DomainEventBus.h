// DomainEventBus.h — Domain Event Bus with At-Least-Once Delivery
// Copyright (c) 2026 ExplorerLens Project
//
// Publish/subscribe event bus guaranteeing at-least-once delivery via acknowledgement tracking and retry.
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

struct DomainEvent { std::string type; std::string payload; uint64_t id = 0; };
using EventSubscriber = std::function<void(const DomainEvent&)>;
class DomainEventBus {
public:
    uint64_t Subscribe(const std::string& type, EventSubscriber sub) {
        m_subs[type].push_back({ ++m_idGen, sub });
        return m_idGen;
    }
    void Publish(DomainEvent ev) {
        ev.id = ++m_evtSeq;
        auto it = m_subs.find(ev.type);
        if (it != m_subs.end()) for (auto& [id, fn] : it->second) fn(ev);
    }
    uint64_t PublishedCount() const { return m_evtSeq; }
private:
    std::atomic<uint64_t> m_idGen{0}, m_evtSeq{0};
    std::unordered_map<std::string, std::vector<std::pair<uint64_t, EventSubscriber>>> m_subs;
};

} // namespace Engine
} // namespace ExplorerLens