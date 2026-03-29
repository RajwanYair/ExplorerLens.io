// BranchAwareCacheKey.h — Branch-Aware Thumbnail Cache Key Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Extends the standard cache key with the active Git branch name so that
// VCS-status overlays are correctly invalidated on branch switches.
//
#pragma once
#include <string>
#include <cstdint>
#include <functional>

namespace ExplorerLens {
namespace Engine {

struct BranchCacheKey {
    std::string filePath;
    std::string branchName;
    uint64_t    contentHash   = 0;
    uint32_t    thumbnailSize = 0;
    std::string repoRoot;

    std::string Composite() const {
        return filePath + "|" + branchName + "|" +
               std::to_string(contentHash) + "|" +
               std::to_string(thumbnailSize);
    }

    bool  IsValid() const { return !filePath.empty(); }

    bool  operator==(const BranchCacheKey& o) const {
        return filePath == o.filePath && branchName == o.branchName &&
               contentHash == o.contentHash && thumbnailSize == o.thumbnailSize;
    }
};

class BranchAwareCacheKey {
public:
    BranchAwareCacheKey() = default;

    BranchCacheKey  Build(const std::string& path, uint32_t size) const {
        BranchCacheKey k;
        k.filePath      = path;
        k.branchName    = m_currentBranch;
        k.thumbnailSize = size;
        k.contentHash   = ComputeHash(path);
        return k;
    }

    void  SetActiveBranch(const std::string& branch) { m_currentBranch = branch; }
    const std::string& GetActiveBranch() const        { return m_currentBranch; }

    void  InvalidateForBranchSwitch(const std::string& newBranch) {
        m_currentBranch = newBranch;
        ++m_switchCount;
    }

    uint32_t  SwitchCount() const { return m_switchCount; }

    static uint64_t  ComputeHash(const std::string& path) {
        uint64_t h = 14695981039346656037ULL;
        for (unsigned char c : path) { h ^= c; h *= 1099511628211ULL; }
        return h;
    }

private:
    std::string m_currentBranch = "main";
    uint32_t    m_switchCount   = 0;
};

} // namespace Engine
} // namespace ExplorerLens
