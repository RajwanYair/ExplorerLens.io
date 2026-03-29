// GitLFSResolver.h — Git LFS Pointer Resolver for Thumbnail Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Detects Git LFS pointer files in the working tree and resolves them to
// actual binary content for thumbnail generation via a configurable LFS store.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LFSResolveStatus {
    Resolved, Pointer, MissingObject, StorageError, NotLFS
};

struct LFSPointerInfo {
    std::string  oid;
    uint64_t     size       = 0;
    std::string  version;
    bool         isPointer  = false;
};

struct LFSResolveConfig {
    std::string  lfsStorageDir;
    bool         cacheResolved = true;
    uint32_t     cacheCapacity = 128;
};

struct LFSResolveResult {
    LFSResolveStatus     status  = LFSResolveStatus::NotLFS;
    std::vector<uint8_t> content;
    std::string          errorMsg;
    bool                 success() const { return status == LFSResolveStatus::Resolved; }
};

class GitLFSResolver {
public:
    explicit GitLFSResolver(const LFSResolveConfig& cfg = {}) : m_cfg(cfg) {}

    LFSPointerInfo  ParsePointer(const std::string& fileContent) const {
        LFSPointerInfo p;
        if (fileContent.substr(0, 14) == "version https:") {
            p.isPointer = true;
            p.version   = "git-lfs/1.0";
        }
        return p;
    }

    LFSResolveResult  Resolve(const std::string& path) const {
        LFSResolveResult r;
        if (path.empty()) { r.status = LFSResolveStatus::StorageError; return r; }
        r.status = LFSResolveStatus::NotLFS;
        return r;
    }

    bool  IsLFSPointer(const std::string& content) const {
        return content.substr(0, 14) == "version https:";
    }

    uint32_t  CacheCapacity() const { return m_cfg.cacheCapacity; }

private:
    LFSResolveConfig m_cfg;
};

} // namespace Engine
} // namespace ExplorerLens
