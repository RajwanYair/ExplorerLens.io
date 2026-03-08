// ShellPropertyStoreRouter.h — Routes IPropertyStore queries to format handlers
// Copyright (c) 2026 ExplorerLens Project
//
// Routes Windows Shell property store queries to format-specific metadata
// extractors, enabling rich metadata display in Explorer properties.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ShellPropertyStoreRouterConfig {
    bool enabled = true;
    uint32_t maxHandlers = 32;
    std::string label = "ShellPropertyStoreRouter";
};

class ShellPropertyStoreRouter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ShellPropertyStoreRouterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct PropertyDef {
        std::string propertyKey;
        std::string displayName;
        enum Type { String, Int64, DateTime, Double } type = String;
    };

    bool RegisterHandler(const std::string& formatExt, const std::vector<PropertyDef>& props) {
        if (m_handlers.size() >= m_config.maxHandlers) return false;
        m_handlers[formatExt] = props;
        return true;
    }

    bool HasHandler(const std::string& ext) const {
        return m_handlers.find(ext) != m_handlers.end();
    }

    uint32_t GetHandlerCount() const { return static_cast<uint32_t>(m_handlers.size()); }

    std::vector<PropertyDef> GetProperties(const std::string& ext) const {
        auto it = m_handlers.find(ext);
        return it != m_handlers.end() ? it->second : std::vector<PropertyDef>{};
    }

private:
    bool m_initialized = false;
    ShellPropertyStoreRouterConfig m_config;
    std::unordered_map<std::string, std::vector<PropertyDef>> m_handlers;
};

}
} // namespace ExplorerLens::Engine
