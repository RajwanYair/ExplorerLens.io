// GlobalShortcutManager.h — System-Wide Shortcut Key Registration
// Copyright (c) 2026 ExplorerLens Project
//
// Registers and manages global hotkeys for quick access to thumbnail
// operations: force-refresh folder, clear cache, toggle overlay.
//
#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class ShortcutAction : uint8_t {
    RefreshFolder,
    ClearCache,
    ToggleOverlay,
    OpenSettings,
    RunBenchmark,
    ExportDiagnostics
};

struct ShortcutBinding {
    uint32_t modifiers = 0;   // MOD_ALT | MOD_CONTROL etc.
    uint32_t vkCode = 0;      // Virtual key code
    ShortcutAction action = ShortcutAction::RefreshFolder;
    bool enabled = true;
};

class GlobalShortcutManager {
public:
    static GlobalShortcutManager& Instance() {
        static GlobalShortcutManager s;
        return s;
    }

    bool RegisterShortcut(ShortcutAction action, uint32_t modifiers, uint32_t vk) {
        ShortcutBinding b{ modifiers, vk, action, true };
        m_bindings[static_cast<uint8_t>(action)] = b;
        return true;
    }

    bool UnregisterShortcut(ShortcutAction action) {
        auto it = m_bindings.find(static_cast<uint8_t>(action));
        if (it != m_bindings.end()) {
            it->second.enabled = false;
            return true;
        }
        return false;
    }

    void SetCallback(ShortcutAction action, std::function<void()> cb) {
        m_callbacks[static_cast<uint8_t>(action)] = std::move(cb);
    }

    bool HandleHotkey(uint32_t id) {
        auto cb = m_callbacks.find(static_cast<uint8_t>(id));
        if (cb != m_callbacks.end() && cb->second) {
            cb->second();
            return true;
        }
        return false;
    }

    size_t ActiveCount() const {
        size_t n = 0;
        for (auto& [k, v] : m_bindings)
            if (v.enabled) ++n;
        return n;
    }

private:
    GlobalShortcutManager() = default;
    std::unordered_map<uint8_t, ShortcutBinding> m_bindings;
    std::unordered_map<uint8_t, std::function<void()>> m_callbacks;
};

} // namespace Engine
} // namespace ExplorerLens
