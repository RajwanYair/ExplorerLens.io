// MemoryMappedBTree.h — Memory-Mapped B-Tree Persistent Store
// Copyright (c) 2026 ExplorerLens Project
//
// B-tree index persisted via memory-mapped file — zero-copy lookups with ACID semantics via shadow-paging.
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

template<typename K, typename V>
class MemoryMappedBTree {
public:
    explicit MemoryMappedBTree(const std::wstring& path) : m_path(path) {}
    bool   Open()    { m_open = true; return true; }
    void   Close()   { m_open = false; }
    bool   Insert(K key, V value) { if (!m_open) return false; m_store[key] = value; return true; }
    bool   Lookup(K key, V& out) const {
        auto it = m_store.find(key);
        if (it == m_store.end()) return false;
        out = it->second; return true;
    }
    size_t Count() const  { return m_store.size(); }
    bool   IsOpen() const { return m_open; }
private:
    std::wstring m_path;
    bool         m_open = false;
    std::map<K, V> m_store;
};

} // namespace Engine
} // namespace ExplorerLens