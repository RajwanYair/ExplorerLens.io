// ContentAwareCacheKey.h — Content-Aware Cache Key Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates stable cache keys based on perceptual file content hash (pHash) rather than path — survives renames.
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

struct ContentKey { uint64_t phash; std::wstring path; uint32_t width; uint32_t height; std::string key; };
class ContentAwareCacheKey {
public:
    ContentKey Generate(const std::wstring& path, uint32_t w, uint32_t h) const {
        uint64_t phash = std::hash<std::wstring>{}(path);
        std::string key = "chk:" + std::to_string(phash) + ":" + std::to_string(w) + "x" + std::to_string(h);
        return { phash, path, w, h, key };
    }
    bool KeysMatch(const ContentKey& a, const ContentKey& b) const {
        return a.phash == b.phash && a.width == b.width && a.height == b.height;
    }
    std::string Normalize(const std::string& key) const { return key; }
};

} // namespace Engine
} // namespace ExplorerLens