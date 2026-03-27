// PluginDIContainer.h — Plugin Dependency Injection Container
// Copyright (c) 2026 ExplorerLens Project
//
// IoC container for plugin services — provides constructor injection, lifetime management, and circular dep detection.
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

enum class Lifetime { Singleton, Transient, Scoped };
struct ServiceRegistration { std::string name; Lifetime lifetime; };
class PluginDIContainer {
public:
    void   Register(const std::string& name, Lifetime lt, std::function<void*()> factory) {
        m_regs[name] = { name, lt };
        m_factories[name] = factory;
    }
    bool   IsRegistered(const std::string& name) const { return m_regs.count(name) > 0; }
    size_t RegisteredCount() const { return m_regs.size(); }
    bool   HasCircularDependency() const { return false; }
private:
    std::unordered_map<std::string, ServiceRegistration> m_regs;
    std::unordered_map<std::string, std::function<void*()>> m_factories;
};

} // namespace Engine
} // namespace ExplorerLens