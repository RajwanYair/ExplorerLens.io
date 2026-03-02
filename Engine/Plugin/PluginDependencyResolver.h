#pragma once
// ============================================================================
// PluginDependencyResolver.h — Topological plugin dependency resolution
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Plugin dependency declaration
struct PluginDependencyDecl {
    std::wstring pluginId;
    std::wstring dependsOnId;
    uint32_t     minVersionMajor = 0;
    uint32_t     minVersionMinor = 0;
    bool         optional = false;  // If true, missing dep doesn't block
};

// Plugin node for dependency graph
struct PluginNode {
    std::wstring pluginId;
    uint32_t     versionMajor = 1;
    uint32_t     versionMinor = 0;
    std::vector<PluginDependencyDecl> dependencies;
    bool         resolved = false;
    uint32_t     loadOrder = 0;
};

// Resolution result
enum class DependencyResolutionStatus : uint32_t {
    Success = 0,
    CyclicDependency = 1,
    MissingDependency = 2,
    VersionConflict = 3,
    EmptyGraph = 4
};

struct DependencyResolutionResult {
    DependencyResolutionStatus status = DependencyResolutionStatus::Success;
    std::vector<std::wstring>  loadOrder;       // Topologically sorted
    std::vector<std::wstring>  unresolvedDeps;  // Missing dependencies
    std::vector<std::wstring>  cyclicPlugins;   // Plugins in cycles
    std::wstring               errorMessage;
};

// ========================================================================
// PluginDependencyResolver — Graph-based plugin dependency ordering
// ========================================================================
class PluginDependencyResolver {
public:
    static PluginDependencyResolver& Instance() {
        static PluginDependencyResolver instance;
        return instance;
    }

    void Initialize() {
        m_nodes.clear();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Register a plugin
    void RegisterPlugin(const std::wstring& pluginId, uint32_t versionMajor = 1, uint32_t versionMinor = 0) {
        PluginNode node;
        node.pluginId = pluginId;
        node.versionMajor = versionMajor;
        node.versionMinor = versionMinor;
        m_nodes[pluginId] = node;
    }

    // Add a dependency
    void AddDependency(const std::wstring& pluginId, const std::wstring& dependsOnId,
        uint32_t minMajor = 0, uint32_t minMinor = 0, bool optional = false) {
        auto it = m_nodes.find(pluginId);
        if (it == m_nodes.end()) return;

        PluginDependencyDecl dep;
        dep.pluginId = pluginId;
        dep.dependsOnId = dependsOnId;
        dep.minVersionMajor = minMajor;
        dep.minVersionMinor = minMinor;
        dep.optional = optional;
        it->second.dependencies.push_back(dep);
    }

    // Resolve all dependencies (topological sort)
    DependencyResolutionResult Resolve() {
        DependencyResolutionResult result;

        if (m_nodes.empty()) {
            result.status = DependencyResolutionStatus::EmptyGraph;
            return result;
        }

        // Check for missing non-optional dependencies
        for (auto& [id, node] : m_nodes) {
            for (auto& dep : node.dependencies) {
                if (!dep.optional && m_nodes.find(dep.dependsOnId) == m_nodes.end()) {
                    result.unresolvedDeps.push_back(dep.dependsOnId);
                }
            }
        }

        if (!result.unresolvedDeps.empty()) {
            result.status = DependencyResolutionStatus::MissingDependency;
            result.errorMessage = L"Missing dependencies: " + result.unresolvedDeps[0];
            return result;
        }

        // Check version constraints
        for (auto& [id, node] : m_nodes) {
            for (auto& dep : node.dependencies) {
                auto depIt = m_nodes.find(dep.dependsOnId);
                if (depIt != m_nodes.end()) {
                    auto& depNode = depIt->second;
                    if (depNode.versionMajor < dep.minVersionMajor ||
                        (depNode.versionMajor == dep.minVersionMajor && depNode.versionMinor < dep.minVersionMinor)) {
                        result.status = DependencyResolutionStatus::VersionConflict;
                        result.errorMessage = L"Version conflict: " + dep.dependsOnId;
                        return result;
                    }
                }
            }
        }

        // Topological sort using Kahn's algorithm
        std::unordered_map<std::wstring, uint32_t> inDegree;
        for (auto& [id, node] : m_nodes) inDegree[id] = 0;

        for (auto& [id, node] : m_nodes) {
            for (auto& dep : node.dependencies) {
                if (!dep.optional || m_nodes.count(dep.dependsOnId)) {
                    inDegree[id]++;
                }
            }
        }

        std::vector<std::wstring> queue;
        for (auto& [id, deg] : inDegree) {
            if (deg == 0) queue.push_back(id);
        }

        while (!queue.empty()) {
            std::wstring current = queue.back();
            queue.pop_back();
            result.loadOrder.push_back(current);

            // Reduce in-degree for plugins that depend on current
            for (auto& [id, node] : m_nodes) {
                for (auto& dep : node.dependencies) {
                    if (dep.dependsOnId == current) {
                        inDegree[id]--;
                        if (inDegree[id] == 0) {
                            queue.push_back(id);
                        }
                    }
                }
            }
        }

        // Check for cycles
        if (result.loadOrder.size() < m_nodes.size()) {
            result.status = DependencyResolutionStatus::CyclicDependency;
            result.errorMessage = L"Cyclic dependencies detected";
            for (auto& [id, deg] : inDegree) {
                if (deg > 0) result.cyclicPlugins.push_back(id);
            }
            return result;
        }

        result.status = DependencyResolutionStatus::Success;
        return result;
    }

    // Get registered plugin count
    uint32_t GetPluginCount() const { return static_cast<uint32_t>(m_nodes.size()); }

    // Check if a plugin is registered
    bool HasPlugin(const std::wstring& pluginId) const {
        return m_nodes.find(pluginId) != m_nodes.end();
    }

private:
    PluginDependencyResolver() = default;

    std::unordered_map<std::wstring, PluginNode> m_nodes;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
