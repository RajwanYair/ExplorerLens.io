// CacheVersionMigrator.h — Cache Format Migration Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Handles versioned cache format upgrades and downgrades, ensuring cache
// data survives application updates without full invalidation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

struct CacheVersion {
    uint16_t major = 1;
    uint16_t minor = 0;

    bool operator==(const CacheVersion& o) const { return major == o.major && minor == o.minor; }
    bool operator<(const CacheVersion& o) const {
        return major < o.major || (major == o.major && minor < o.minor);
    }
    bool operator>(const CacheVersion& o) const { return o < *this; }
    std::string ToString() const { return std::to_string(major) + "." + std::to_string(minor); }
};

struct CacheVersionStep {
    CacheVersion from;
    CacheVersion to;
    std::string description;
    std::function<bool()> migrate;
};

struct CacheVersionResult {
    bool success = false;
    CacheVersion fromVersion;
    CacheVersion toVersion;
    uint32_t stepsExecuted = 0;
    uint32_t entriesMigrated = 0;
    uint32_t entriesDropped = 0;
    double durationMs = 0.0;
    std::string errorMessage;
};

class CacheVersionMigrator {
public:
    void RegisterStep(CacheVersionStep step) {
        m_steps.push_back(std::move(step));
    }

    bool CanMigrate(CacheVersion from, CacheVersion to) const {
        if (from == to) return true;
        auto current = from;
        for (const auto& step : m_steps) {
            if (step.from == current) {
                current = step.to;
                if (current == to) return true;
            }
        }
        return false;
    }

    std::vector<CacheVersionStep> GetPath(CacheVersion from, CacheVersion to) const {
        std::vector<CacheVersionStep> path;
        auto current = from;
        for (const auto& step : m_steps) {
            if (step.from == current) {
                path.push_back(step);
                current = step.to;
                if (current == to) break;
            }
        }
        return path;
    }

    bool NeedsMigration(CacheVersion existing, CacheVersion current) const {
        return !(existing == current);
    }

    static CacheVersion CurrentVersion() { return { 2, 0 }; }

private:
    std::vector<CacheVersionStep> m_steps;
};

} // namespace Engine
} // namespace ExplorerLens
