// Utils\OrphanedFileScanner.h - OrphanedFileScanner
// Copyright (c) 2026 ExplorerLens Project
//
// Detects unreferenced source files and dead code paths in the project tree
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class OrphanedFileScanner {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "OrphanedFileScanner";
    };

    OrphanedFileScanner() = default;
    explicit OrphanedFileScanner(const Config& cfg) : m_config(cfg) {}

    bool Initialize() {
        if (!m_config.enabled) return false;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }
    const Config& GetConfig() const { return m_config; }
    const std::string& GetName() const { return m_config.label; }

private:
    Config m_config{};
    bool m_initialized = false;
    std::string m_scanRoots{};
    std::string m_referenceIndex{};
    uint32_t m_orphanCount{};
};

} // namespace Engine
} // namespace ExplorerLens
