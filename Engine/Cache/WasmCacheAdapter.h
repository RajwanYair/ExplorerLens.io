// WasmCacheAdapter.h — IndexedDB Cache Adapter for WASM Thumbnail Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Maps the PartialDecodeStateCache API surface to browser IndexedDB storage,
// enabling persistent thumbnail caching in-browser across page loads without
// server coordination.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class WasmCacheStatus : uint32_t
{
    OK              = 0,
    NOT_FOUND       = 1,
    QUOTA_EXCEEDED  = 2,
    STORE_ERROR     = 3
};

struct WasmCacheEntry
{
    std::string          key;
    std::vector<uint8_t> data;
    size_t               sizeBytes = 0u;
    uint64_t             expiresMs = 0u;   // 0 = no expiry
};

class WasmCacheAdapter
{
public:
    WasmCacheAdapter()  = default;
    ~WasmCacheAdapter() = default;

    WasmCacheAdapter(const WasmCacheAdapter&)            = delete;
    WasmCacheAdapter& operator=(const WasmCacheAdapter&) = delete;
    WasmCacheAdapter(WasmCacheAdapter&&)                 = default;
    WasmCacheAdapter& operator=(WasmCacheAdapter&&)      = default;

    WasmCacheStatus Store(const WasmCacheEntry& entry);
    WasmCacheEntry  Get(const std::string& key) const;
    void            Evict(const std::string& key);
    void            Clear();

    size_t   EntryCount()    const;
    uint64_t TotalSizeBytes() const;

private:
    std::vector<WasmCacheEntry> m_entries;
    uint64_t                    m_totalBytes = 0u;
};

}} // namespace ExplorerLens::Engine
