// PluginDependencyGraph.h — Plugin Dependency DAG Resolution
// Copyright (c) 2026 ExplorerLens Project
//
// Builds and resolves a directed acyclic graph of plugin dependencies,
// with cycle detection and topological sort for deterministic load order.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct PluginDependencyNode {
    std::string              pluginId;
    std::string              displayName;
    uint32_t                 version = 0;
    bool                     optional = false;
    std::vector<std::string> dependencies; // IDs of required plugins
};

struct DependencyEdge {
    std::string from;  // Dependent plugin
    std::string to;    // Required plugin
    bool        optional = false;
};

class PluginDependencyGraph {
public:
    static PluginDependencyGraph& Instance() {
        static PluginDependencyGraph s;
        return s;
    }

    void AddNode(const PluginDependencyNode& node) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_nodes[node.pluginId] = node;
        for (const auto& dep : node.dependencies) {
            DependencyEdge edge;
            edge.from = node.pluginId;
            edge.to = dep;
            edge.optional = node.optional;
            m_edges.push_back(edge);
        }
    }

    bool Resolve() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_resolved = false;
        m_loadOrder.clear();
        m_missingDeps.clear();

        // Check for missing dependencies
        for (const auto& [id, node] : m_nodes) {
            for (const auto& dep : node.dependencies) {
                if (m_nodes.find(dep) == m_nodes.end()) {
                    m_missingDeps.push_back(dep);
                }
            }
        }

        if (!m_missingDeps.empty()) return false;
        if (DetectCyclesInternal()) return false;

        // Topological sort using Kahn's algorithm
        std::unordered_map<std::string, uint32_t> inDegree;
        for (const auto& [id, node] : m_nodes) inDegree[id] = 0;
        for (const auto& edge : m_edges) {
            if (m_nodes.count(edge.from) && m_nodes.count(edge.to))
                inDegree[edge.from]++;
        }

        std::vector<std::string> queue;
        for (const auto& [id, deg] : inDegree) {
            if (deg == 0) queue.push_back(id);
        }

        while (!queue.empty()) {
            std::string current = queue.back();
            queue.pop_back();
            m_loadOrder.push_back(current);

            for (const auto& edge : m_edges) {
                if (edge.to == current && m_nodes.count(edge.from)) {
                    inDegree[edge.from]--;
                    if (inDegree[edge.from] == 0) {
                        queue.push_back(edge.from);
                    }
                }
            }
        }

        m_resolved = (m_loadOrder.size() == m_nodes.size());
        return m_resolved;
    }

    bool DetectCycles() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return DetectCyclesInternal();
    }

    std::vector<std::string> GetLoadOrder() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_loadOrder;
    }

    std::vector<std::string> GetMissingDependencies() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_missingDeps;
    }

    std::vector<std::string> GetDependenciesOf(const std::string& pluginId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_nodes.find(pluginId);
        return it != m_nodes.end() ? it->second.dependencies : std::vector<std::string>{};
    }

    size_t GetNodeCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_nodes.size();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_nodes.clear();
        m_edges.clear();
        m_loadOrder.clear();
        m_missingDeps.clear();
        m_resolved = false;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [id, node] : m_nodes) {
            if (node.pluginId.empty()) return false;
            if (node.pluginId != id) return false;
        }
        if (m_resolved && m_loadOrder.size() != m_nodes.size()) return false;
        return true;
    }

private:
    PluginDependencyGraph() = default;
    ~PluginDependencyGraph() = default;
    PluginDependencyGraph(const PluginDependencyGraph&) = delete;
    PluginDependencyGraph& operator=(const PluginDependencyGraph&) = delete;

    bool DetectCyclesInternal() const {
        const uint8_t WHITE = 0, GRAY = 1, BLACK = 2;
        std::unordered_map<std::string, uint8_t> color;
        for (const auto& [id, node] : m_nodes) color[id] = WHITE;

        for (const auto& [id, node] : m_nodes) {
            if (color[id] == WHITE) {
                if (DFSHasCycle(id, color)) return true;
            }
        }
        return false;
    }

    bool DFSHasCycle(const std::string& /*nodeId*/,
        std::unordered_map<std::string, int>& /*not used*/) const {
        // Overload disambiguation — real DFS uses the uint8_t color map.
        return false;
    }

    bool DFSHasCycle(const std::string& nodeId,
        std::unordered_map<std::string, uint8_t>& color) const {
        color[nodeId] = 1; // Gray
        auto it = m_nodes.find(nodeId);
        if (it != m_nodes.end()) {
            for (const auto& dep : it->second.dependencies) {
                if (m_nodes.count(dep)) {
                    if (color[dep] == 1) return true; // Back edge = cycle
                    if (color[dep] == 0 && DFSHasCycle(dep, color)) return true;
                }
            }
        }
        color[nodeId] = 2; // Black
        return false;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PluginDependencyNode> m_nodes;
    std::vector<DependencyEdge> m_edges;
    std::vector<std::string> m_loadOrder;
    std::vector<std::string> m_missingDeps;
    bool m_resolved = false;
};

}
} // namespace ExplorerLens::Engine
