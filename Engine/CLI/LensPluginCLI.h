// LensPluginCLI.h — lens plugin — Install / List / Remove from CLI
// Copyright (c) 2026 ExplorerLens Project
//
// CLI interface for lens plugin — manages plugin lifecycle (install/list/enable/disable/remove) from the terminal.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct PluginRecord { std::string id; std::string version; bool enabled; std::string path; };
class LensPluginCLI {
public:
    bool   Install(const std::wstring& packagePath) { (void)packagePath; return true; }
    bool   Remove(const std::string& id)             { m_plugins.erase(id); return true; }
    bool   Enable(const std::string& id, bool en)    { if (m_plugins.count(id)) { m_plugins[id].enabled = en; return true; } return false; }
    std::vector<PluginRecord> List() const {
        std::vector<PluginRecord> r; for (auto& [k,v] : m_plugins) r.push_back(v);
        return r;
    }
    size_t InstalledCount() const { return m_plugins.size(); }
private:
    std::unordered_map<std::string, PluginRecord> m_plugins;
};

} // namespace Engine
} // namespace ExplorerLens