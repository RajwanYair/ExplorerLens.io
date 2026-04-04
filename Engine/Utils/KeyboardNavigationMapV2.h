// KeyboardNavigationMapV2.h — Keyboard Navigation Map v2
// Copyright (c) 2026 ExplorerLens Project
//
// Defines and manages the keyboard navigation graph for the thumbnail grid and
// annotation panels, supporting arrow-key navigation, tab order, and custom shortcuts.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NavigationDirection {
    Up,
    Down,
    Left,
    Right,
    Next,
    Previous,
    First,
    Last
};
enum class KeyboardModifier {
    None = 0,
    Shift = 1,
    Ctrl = 2,
    Alt = 4
};

struct NavigationNode
{
    std::string id;
    std::string name;
    int tabIndex = 0;
    bool disabled = false;
    std::unordered_map<int, std::string> neighbors;  // direction → id
};

struct NavigationResult
{
    bool found = false;
    std::string targetNodeId;
    bool wrappedAround = false;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return found;
    }
};

class KeyboardNavigationMapV2
{
  public:
    explicit KeyboardNavigationMapV2() = default;

    void AddNode(NavigationNode node)
    {
        m_nodes[node.id] = std::move(node);
    }

    void AddEdge(const std::string& fromId, NavigationDirection dir, const std::string& toId)
    {
        m_nodes[fromId].neighbors[static_cast<int>(dir)] = toId;
    }

    NavigationResult Navigate(const std::string& currentId, NavigationDirection dir) const
    {
        auto it = m_nodes.find(currentId);
        if (it == m_nodes.end())
            return {false, {}, false, "Node not found: " + currentId};

        auto nit = it->second.neighbors.find(static_cast<int>(dir));
        if (nit == it->second.neighbors.end()) {
            // Wrap-around for List-type navigation
            if (dir == NavigationDirection::Next || dir == NavigationDirection::Previous) {
                auto wrapped = WrapNavigate(currentId, dir);
                if (!wrapped.empty())
                    return {true, wrapped, true, {}};
            }
            return {false, {}, false, "No neighbor in direction"};
        }
        const std::string& targetId = nit->second;
        auto tit = m_nodes.find(targetId);
        if (tit == m_nodes.end() || tit->second.disabled)
            return {false, {}, false, "Target disabled or missing"};
        return {true, targetId, false, {}};
    }

    std::vector<std::string> TabOrder() const
    {
        std::vector<std::pair<int, std::string>> order;
        for (const auto& [id, n] : m_nodes)
            if (!n.disabled)
                order.push_back({n.tabIndex, id});
        std::sort(order.begin(), order.end());
        std::vector<std::string> result;
        for (auto& [_, id] : order)
            result.push_back(id);
        return result;
    }

    size_t NodeCount() const noexcept
    {
        return m_nodes.size();
    }

  private:
    std::string WrapNavigate(const std::string& currentId, NavigationDirection dir) const
    {
        auto order = TabOrder();
        for (size_t i = 0; i < order.size(); ++i) {
            if (order[i] == currentId) {
                if (dir == NavigationDirection::Next)
                    return order[(i + 1) % order.size()];
                else
                    return order[(i + order.size() - 1) % order.size()];
            }
        }
        return {};
    }

    std::unordered_map<std::string, NavigationNode> m_nodes;
};

}  // namespace Engine
}  // namespace ExplorerLens
