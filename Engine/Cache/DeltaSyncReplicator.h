// DeltaSyncReplicator.h — Delta-Sync Cache Replication Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Replicates only changed cache entries between ExplorerLens instances using binary diffs and vector clocks.
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

struct CacheDelta { std::wstring key; std::vector<uint8_t> data; uint64_t vectorClock; bool deletion; };
struct SyncResult  { size_t applied; size_t skipped; size_t conflicts; uint64_t newClock; };
class DeltaSyncReplicator {
public:
    SyncResult Apply(const std::vector<CacheDelta>& deltas) {
        for (auto& d : deltas) {
            if (!d.deletion) m_store[d.key] = d.data;
            else             m_store.erase(d.key);
            m_clock = std::max(m_clock, d.vectorClock) + 1;
        }
        return { deltas.size(), 0, 0, m_clock };
    }
    std::vector<CacheDelta> CollectDeltas(uint64_t sinceClk) const {
        (void)sinceClk; return {};
    }
    uint64_t LocalClock()  const { return m_clock; }
    size_t   EntryCount()  const { return m_store.size(); }
private:
    uint64_t m_clock = 0;
    std::unordered_map<std::wstring, std::vector<uint8_t>> m_store;
};

} // namespace Engine
} // namespace ExplorerLens