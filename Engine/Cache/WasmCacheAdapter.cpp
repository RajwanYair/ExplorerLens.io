// WasmCacheAdapter.cpp — IndexedDB Cache Adapter for WASM Thumbnail Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
#include "WasmCacheAdapter.h"

#include <algorithm>

namespace ExplorerLens { namespace Engine {

WasmCacheStatus WasmCacheAdapter::Store(const WasmCacheEntry& entry)
{
    // Replace existing entry if key already present
    auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [&entry](const WasmCacheEntry& e) { return e.key == entry.key; });

    if (it != m_entries.end())
    {
        m_totalBytes -= it->sizeBytes;
        *it = entry;
    }
    else
    {
        m_entries.push_back(entry);
    }
    m_totalBytes += entry.sizeBytes;
    return WasmCacheStatus::OK;
}

WasmCacheEntry WasmCacheAdapter::Get(const std::string& key) const
{
    const auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [&key](const WasmCacheEntry& e) { return e.key == key; });

    return (it != m_entries.end()) ? *it : WasmCacheEntry{};
}

void WasmCacheAdapter::Evict(const std::string& key)
{
    const auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [&key](const WasmCacheEntry& e) { return e.key == key; });

    if (it != m_entries.end())
    {
        m_totalBytes -= it->sizeBytes;
        m_entries.erase(it);
    }
}

void WasmCacheAdapter::Clear()
{
    m_entries.clear();
    m_totalBytes = 0u;
}

size_t   WasmCacheAdapter::EntryCount()     const { return m_entries.size(); }
uint64_t WasmCacheAdapter::TotalSizeBytes() const { return m_totalBytes;     }

}} // namespace ExplorerLens::Engine
