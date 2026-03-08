// ShellOverlayDispatcher.h — Shell icon overlay routing for thumbnail badges
// Copyright (c) 2026 ExplorerLens Project
//
// Manages overlay icon dispatching for thumbnails — adds status badges
// (cached, error, processing) as shell icon overlays on Explorer items.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ShellOverlayDispatcherConfig {
    bool enabled = true;
    uint32_t maxOverlays = 15;
    std::string label = "ShellOverlayDispatcher";
};

class ShellOverlayDispatcher {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ShellOverlayDispatcherConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class OverlayType : uint8_t { None, Cached, Processing, Error, Stale };

    struct OverlayEntry {
        std::string extension;
        OverlayType type = OverlayType::None;
        int priority = 0;
    };

    bool RegisterOverlay(const OverlayEntry& entry) {
        if (m_entries.size() >= m_config.maxOverlays) return false;
        m_entries.push_back(entry);
        return true;
    }

    size_t GetRegisteredCount() const { return m_entries.size(); }

private:
    bool m_initialized = false;
    ShellOverlayDispatcherConfig m_config;
    std::vector<OverlayEntry> m_entries;
};

}
} // namespace ExplorerLens::Engine
