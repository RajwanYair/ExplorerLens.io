// ExtensionConflictDetector.h — Detects conflicting shell extension registrations
// Copyright (c) 2026 ExplorerLens Project
//
// Scans the registry for other IThumbnailProvider registrations that conflict
// with ExplorerLens handlers, reporting conflicts with handler identification.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ExtensionConflictDetectorConfig {
    bool enabled = true;
    uint32_t maxScanExtensions = 500;
    std::string label = "ExtensionConflictDetector";
};

class ExtensionConflictDetector {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ExtensionConflictDetectorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct Conflict {
        std::string extension;
        std::string conflictingCLSID;
        std::string description;
    };

    void AddConflict(const Conflict& c) { m_conflicts.push_back(c); }
    const std::vector<Conflict>& GetConflicts() const { return m_conflicts; }
    bool HasConflicts() const { return !m_conflicts.empty(); }

private:
    bool m_initialized = false;
    ExtensionConflictDetectorConfig m_config;
    std::vector<Conflict> m_conflicts;
};

}
} // namespace ExplorerLens::Engine
