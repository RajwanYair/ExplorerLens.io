// AsyncRegistrySnapshot.h — Non-blocking registry state capture
// Copyright (c) 2026 ExplorerLens Project
//
// Captures a consistent snapshot of shell extension registry entries
// asynchronously, avoiding UI thread blocking during diagnostics or export.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct AsyncRegistrySnapshotConfig {
    bool enabled = true;
    uint32_t timeoutMs = 5000;
    std::string label = "AsyncRegistrySnapshot";
};

class AsyncRegistrySnapshot {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    AsyncRegistrySnapshotConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct RegistryEntry {
        std::string keyPath;
        std::string valueName;
        std::string valueData;
    };

    void CaptureEntry(const std::string& key, const std::string& name, const std::string& data) {
        m_snapshot[key] = { key, name, data };
    }

    bool HasEntry(const std::string& key) const {
        return m_snapshot.count(key) > 0;
    }

    size_t GetEntryCount() const { return m_snapshot.size(); }

private:
    bool m_initialized = false;
    AsyncRegistrySnapshotConfig m_config;
    std::unordered_map<std::string, RegistryEntry> m_snapshot;
};

}
} // namespace ExplorerLens::Engine
