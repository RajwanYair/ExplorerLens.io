// PluginDependencyGraph.h — Plugin Dependency Resolver
// Copyright (c) 2026 ExplorerLens Project
//
// Directed dependency graph with cycle detection and topological install ordering.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace ExplorerLens::Engine {

enum class DependencyType : uint8_t {
    Required  = 0,
    Optional  = 1,
    Conflicts = 2,
    Replaces  = 3,
};

struct DependencyEdge {
    std::string    fromId;
    std::string    toId;
    DependencyType type              = DependencyType::Required;
    std::string    versionConstraint; // e.g. ">=1.2.0 <2.0.0"

    [[nodiscard]] bool IsValid() const noexcept {
        return !fromId.empty() && !toId.empty();
    }
};

struct ResolutionResult {
    std::vector<std::string> installOrder;   // topological order (leaves first)
    std::vector<DependencyEdge> conflicts;   // pairs of conflicting plugins
    std::vector<std::string> missingDeps;    // required deps not in the graph

    [[nodiscard]] bool IsSuccess() const noexcept {
        return conflicts.empty() && missingDeps.empty();
    }
};

struct PluginNode {
    std::string id;
    std::string version;
    bool        installed = false;
};

class PluginDependencyGraph {
public:
    PluginDependencyGraph()  = default;
    ~PluginDependencyGraph() = default;

    PluginDependencyGraph(const PluginDependencyGraph&)            = delete;
    PluginDependencyGraph& operator=(const PluginDependencyGraph&) = delete;
    PluginDependencyGraph(PluginDependencyGraph&&)                 = default;
    PluginDependencyGraph& operator=(PluginDependencyGraph&&)      = default;

    // Register a plugin node; no-op if already present.
    void AddPlugin(const PluginNode& node);

    // Add a directed dependency edge; returns false if either node is unknown.
    bool AddEdge(const DependencyEdge& edge);

    // Resolve the full dependency graph starting from a root plugin.
    [[nodiscard]] ResolutionResult Resolve(const std::string& rootPluginId) const;

    // Returns true if one or more dependency cycles exist.
    [[nodiscard]] bool DetectCycles() const;

    // Topological order for all registered plugins (Kahn's algorithm).
    [[nodiscard]] std::vector<std::string> GetInstallOrder() const;

    // Returns all conflict edges in the current graph.
    [[nodiscard]] std::vector<DependencyEdge> FindConflicts() const;

    // Remove a plugin and all edges that reference it.
    bool RemovePlugin(const std::string& pluginId);

    // Check whether a version string satisfies a constraint expression.
    [[nodiscard]] static bool SatisfiesConstraint(
        const std::string& version,
        const std::string& constraint) noexcept;

    [[nodiscard]] uint32_t NodeCount() const noexcept;
    [[nodiscard]] uint32_t EdgeCount() const noexcept;

    void Clear() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl{ nullptr };

    void EnsureImpl();

    // DFS helper used by both DetectCycles and GetInstallOrder.
    bool DFSVisit(
        const std::string& nodeId,
        std::vector<std::string>& visited,
        std::vector<std::string>& stack,
        std::vector<std::string>& order) const;
};

} // namespace ExplorerLens::Engine
