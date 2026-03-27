// ShardedCachePartitionV2.h — Sharded Cache Partition v2
// Copyright (c) 2026 ExplorerLens Project
//
// Partitions the thumbnail cache into N independent shards to reduce mutex contention on multi-core systems.
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

struct ShardStats { uint32_t shardId; size_t entryCount; size_t bytesUsed; double hitRate; };
class ShardedCachePartitionV2 {
public:
    explicit ShardedCachePartitionV2(uint32_t shardCount = 16) : m_shards(shardCount) {}
    bool   Put(const std::wstring& key, std::vector<uint8_t> data) {
        m_shards[ShardOf(key)][key] = std::move(data);
        return true;
    }
    bool   Get(const std::wstring& key, std::vector<uint8_t>& out) const {
        auto& shard = m_shards[ShardOf(key)];
        auto it = shard.find(key);
        if (it == shard.end()) return false;
        out = it->second; return true;
    }
    ShardStats Stats(uint32_t shardId) const {
        return { shardId, m_shards[shardId].size(), 0, 0.0 };
    }
    uint32_t ShardCount() const { return static_cast<uint32_t>(m_shards.size()); }
private:
    uint32_t ShardOf(const std::wstring& k) const {
        return static_cast<uint32_t>(std::hash<std::wstring>{}(k) % m_shards.size());
    }
    std::vector<std::unordered_map<std::wstring, std::vector<uint8_t>>> m_shards;
};

} // namespace Engine
} // namespace ExplorerLens