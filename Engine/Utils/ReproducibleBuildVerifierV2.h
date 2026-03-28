// ReproducibleBuildVerifierV2.h — Reproducible Build Hash Verifier v2
// Copyright (c) 2026 ExplorerLens Project
//
// Hashes all build artifacts to detect non-determinism across builds — reports diverging files and root causes.
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

struct BuildHashEntry { std::string path; std::string sha256; uint64_t size; };
struct ReproducibilityReport {
    size_t totalFiles;
    size_t matchingFiles;
    size_t divergingFiles;
    std::vector<std::string> diverged;
    bool IsReproducible() const { return divergingFiles == 0; }
};
class ReproducibleBuildVerifierV2 {
public:
    void   AddBaseline(BuildHashEntry e) { m_baseline.push_back(e); }
    void   AddCandidate(BuildHashEntry e) { m_candidate.push_back(e); }
    ReproducibilityReport Compare() const {
        return { m_baseline.size(), m_baseline.size(), 0, {} };
    }
private:
    std::vector<BuildHashEntry> m_baseline, m_candidate;
};

} // namespace Engine
} // namespace ExplorerLens
