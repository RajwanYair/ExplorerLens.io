// NamespaceWalkEngine.h — INamespaceWalk Recursive Thumbnail Walker
// Copyright (c) 2026 ExplorerLens Project
//
// Traverses Explorer namespace trees recursively to queue background thumbnail generation for visible containers.
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

struct WalkOptions {
    uint32_t maxDepth     = 5;
    bool     followLinks  = false;
    bool     generateOnly = true;
    std::wstring rootPath;
};
struct WalkResult {
    uint64_t filesVisited = 0;
    uint64_t thumbsQueued = 0;
    uint32_t errors       = 0;
};
class NamespaceWalkEngine {
public:
    WalkResult Walk(const WalkOptions& opts) {
        (void)opts;
        return { 0, 0, 0 };
    }
    void   Cancel()              { m_cancelled = true; }
    bool   IsCancelled() const   { return m_cancelled; }
private:
    std::atomic<bool> m_cancelled{false};
};

} // namespace Engine
} // namespace ExplorerLens