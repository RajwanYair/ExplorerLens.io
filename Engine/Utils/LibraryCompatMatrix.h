// Utils\LibraryCompatMatrix.h - LibraryCompatMatrix
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a compatibility matrix for external library versions and CRT
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class LibraryCompatMatrix {
public:
    struct Config {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "LibraryCompatMatrix";
    };

    LibraryCompatMatrix() = default;
    explicit LibraryCompatMatrix(const Config& cfg) : m_config(cfg) {}

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
    std::string m_matrix{};
    uint32_t m_libraryCount{};
    uint32_t m_compatScore{};
};

} // namespace Engine
} // namespace ExplorerLens
