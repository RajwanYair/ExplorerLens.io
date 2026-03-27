// ConsistentHashRing.h — Consistent-Hash Ring for Distributed Cache
// Copyright (c) 2026 ExplorerLens Project
//
// Consistent-hashing ring with virtual nodes for distributing cache keys across multiple process instances.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct RingNode { std::string id; uint32_t weight; };
class ConsistentHashRing {
public:
    void   AddNode(RingNode node) {
        for (uint32_t i = 0; i < node.weight; i++) {
            uint64_t h = std::hash<std::string>{}(node.id + "#" + std::to_string(i));
            m_ring[h] = node.id;
        }
    }
    void   RemoveNode(const std::string& id) {
        for (auto it = m_ring.begin(); it != m_ring.end(); )
            it = (it->second == id) ? m_ring.erase(it) : std::next(it);
    }
    std::string Lookup(const std::wstring& key) const {
        if (m_ring.empty()) return "";
        uint64_t h = std::hash<std::wstring>{}(key);
        auto it = m_ring.lower_bound(h);
        if (it == m_ring.end()) it = m_ring.begin();
        return it->second;
    }
    size_t NodeCount()    const { return m_ring.empty() ? 0 : m_ring.size(); }
private:
    std::map<uint64_t, std::string> m_ring;
};

} // namespace Engine
} // namespace ExplorerLens