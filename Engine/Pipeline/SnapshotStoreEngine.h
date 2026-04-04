// SnapshotStoreEngine.h — Aggregate Snapshot Store
// Copyright (c) 2026 ExplorerLens Project
//
// Persists periodic aggregate snapshots to avoid replaying the full event log on startup.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct SnapshotRecord
{
    uint64_t aggregateId;
    uint64_t version;
    std::string data;  // serialized state (JSON/binary)
    uint64_t takenAt = 0;
};
class SnapshotStoreEngine
{
  public:
    void Save(SnapshotRecord rec)
    {
        m_snaps[rec.aggregateId] = std::move(rec);
    }
    bool Load(uint64_t id, SnapshotRecord& out) const
    {
        auto it = m_snaps.find(id);
        if (it == m_snaps.end())
            return false;
        out = it->second;
        return true;
    }
    size_t Count() const
    {
        return m_snaps.size();
    }
    void Prune(uint64_t olderThan)
    {
        (void)olderThan;
    }

  private:
    std::unordered_map<uint64_t, SnapshotRecord> m_snaps;
};

}  // namespace Engine
}  // namespace ExplorerLens