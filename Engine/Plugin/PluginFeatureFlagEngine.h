// PluginFeatureFlagEngine.h — Plugin Feature Flag Evaluator
// Copyright (c) 2026 ExplorerLens Project
//
// Evaluates feature flags for plugins via remote config — supports percentage rollouts, killswitches, and overrides.
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

struct FeatureFlag { std::string key; bool enabled; double rolloutPct; std::string variant; };
class PluginFeatureFlagEngine {
public:
    void   SetFlag(FeatureFlag flag)                   { m_flags[flag.key] = std::move(flag); }
    bool   IsEnabled(const std::string& key, uint64_t userId = 0) const {
        auto it = m_flags.find(key);
        if (it == m_flags.end()) return false;
        return it->second.enabled && (userId % 100) < static_cast<uint64_t>(it->second.rolloutPct * 100);
    }
    std::string Variant(const std::string& key) const {
        auto it = m_flags.find(key);
        return it != m_flags.end() ? it->second.variant : "";
    }
    size_t FlagCount() const { return m_flags.size(); }
private:
    std::unordered_map<std::string, FeatureFlag> m_flags;
};

} // namespace Engine
} // namespace ExplorerLens