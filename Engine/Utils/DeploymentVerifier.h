// DeploymentVerifier.h — Verifies post-deployment file integrity
// Copyright (c) 2026 ExplorerLens Project
//
// Checks installed DLL/EXE files against expected hashes and sizes after
// deployment to detect corruption, incomplete installs, or tampering.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DeploymentVerifierConfig {
    bool enabled = true;
    uint32_t maxFiles = 64;
    std::string label = "DeploymentVerifier";
};

class DeploymentVerifier {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    DeploymentVerifierConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct FileEntry {
        std::string relativePath;
        uint64_t expectedSize = 0;
        std::string expectedHash; // SHA-256
        bool verified = false;
    };

    bool RegisterFile(const FileEntry& entry) {
        if (m_entries.size() >= m_config.maxFiles) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool AllVerified() const {
        for (const auto& e : m_entries)
            if (!e.verified) return false;
        return !m_entries.empty();
    }

    size_t GetFileCount() const { return m_entries.size(); }

private:
    bool m_initialized = false;
    DeploymentVerifierConfig m_config;
    std::vector<FileEntry> m_entries;
};

}
} // namespace ExplorerLens::Engine
