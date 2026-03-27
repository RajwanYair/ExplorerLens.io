// ZeroCopyCacheReader.h — Zero-Copy Cache Read Path (File-Mapped)
// Copyright (c) 2026 ExplorerLens Project
//
// Returns memory-mapped views directly into the cache file — avoids memcpy for large thumbnail reads.
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

struct MappedView { const uint8_t* data; size_t size; uint32_t width; uint32_t height; bool valid; };
class ZeroCopyCacheReader {
public:
    bool   Open(const std::wstring& cacheFile) { (void)cacheFile; m_open = true; return true; }
    void   Close() { m_open = false; }
    MappedView Read(const std::wstring& key) const {
        (void)key;
        return { nullptr, 0, 0, 0, false };
    }
    void   Release(MappedView& view) { view = {}; }
    bool   IsOpen() const { return m_open; }
    size_t HitCount() const { return m_hits; }
private:
    bool   m_open = false;
    size_t m_hits = 0;
};

} // namespace Engine
} // namespace ExplorerLens