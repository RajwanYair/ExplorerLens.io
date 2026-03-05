// PluginVersionMigrator.h — Plugin API Version Migration
// Copyright (c) 2026 ExplorerLens Project
//
// Plugin API version migration. Converts plugin manifests between API versions,
// patches deprecated API calls, and provides compatibility shims.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <functional>

namespace ExplorerLens {
namespace Engine {

struct APIVersion {
    uint32_t major = 1;
    uint32_t minor = 0;

    inline bool operator<(const APIVersion& other) const {
        return major < other.major || (major == other.major && minor < other.minor);
    }

    inline bool operator==(const APIVersion& other) const {
        return major == other.major && minor == other.minor;
    }

    inline bool operator<=(const APIVersion& other) const {
        return *this < other || *this == other;
    }

    inline std::string ToString() const {
        return std::to_string(major) + "." + std::to_string(minor);
    }
};

struct DeprecatedAPI {
    std::string oldName;
    std::string newName;
    APIVersion deprecatedSince;
    APIVersion removedIn;
    std::string migrationHint;
};

struct PluginMigrationStep {
    APIVersion fromVersion;
    APIVersion toVersion;
    std::string description;
    uint32_t changesApplied = 0;
};

struct PluginMigrationResult {
    bool success = false;
    std::vector<PluginMigrationStep> stepsApplied;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
    APIVersion originalVersion;
    APIVersion targetVersion;
};

struct ManifestField {
    std::string key;
    std::string value;
};

struct PluginManifestV2 {
    std::string pluginId;
    APIVersion apiVersion;
    std::vector<ManifestField> fields;
    std::vector<std::string> permissions;
};

class PluginVersionMigrator {
public:
    static PluginVersionMigrator& Instance() {
        static PluginVersionMigrator instance;
        return instance;
    }

    inline void RegisterDeprecation(const DeprecatedAPI& deprecation) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_deprecations.push_back(deprecation);
    }

    inline PluginMigrationResult MigrateManifest(PluginManifestV2& manifest, const APIVersion& targetVersion) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginMigrationResult result;
        result.originalVersion = manifest.apiVersion;
        result.targetVersion = targetVersion;

        if (targetVersion < manifest.apiVersion) {
            result.errors.push_back("Cannot downgrade API version from " +
                manifest.apiVersion.ToString() + " to " + targetVersion.ToString());
            return result;
        }

        if (manifest.apiVersion == targetVersion) {
            result.success = true;
            return result;
        }

        APIVersion current = manifest.apiVersion;
        while (current < targetVersion) {
            APIVersion next = { current.major, current.minor + 1 };
            if (next.minor > 99) {
                next = { current.major + 1, 0 };
            }
            if (targetVersion < next) next = targetVersion;

            PluginMigrationStep step;
            step.fromVersion = current;
            step.toVersion = next;
            step.description = "Migrate from " + current.ToString() + " to " + next.ToString();

            for (const auto& dep : m_deprecations) {
                if (current < dep.deprecatedSince && !(next < dep.deprecatedSince)) {
                    for (auto& field : manifest.fields) {
                        if (field.key == dep.oldName) {
                            result.warnings.push_back("Deprecated field '" + dep.oldName +
                                "' renamed to '" + dep.newName + "'");
                            field.key = dep.newName;
                            step.changesApplied++;
                        }
                    }
                }
            }

            result.stepsApplied.push_back(step);
            current = next;
        }

        manifest.apiVersion = targetVersion;
        result.success = true;
        return result;
    }

    inline std::vector<DeprecatedAPI> GetDeprecationsForVersion(const APIVersion& version) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<DeprecatedAPI> applicable;
        for (const auto& dep : m_deprecations) {
            if (dep.deprecatedSince <= version && version < dep.removedIn) {
                applicable.push_back(dep);
            }
        }
        return applicable;
    }

    inline bool IsAPISupported(const APIVersion& version) const {
        return version.major >= 1 && version.major <= CURRENT_API_VERSION.major;
    }

    inline APIVersion GetCurrentAPIVersion() const { return CURRENT_API_VERSION; }

    inline std::string GenerateMigrationReport(const PluginMigrationResult& result) const {
        std::string report = "Migration Report: " + result.originalVersion.ToString() +
            " -> " + result.targetVersion.ToString() + "\n";
        report += "Status: " + std::string(result.success ? "SUCCESS" : "FAILED") + "\n";
        report += "Steps: " + std::to_string(result.stepsApplied.size()) + "\n";
        for (const auto& step : result.stepsApplied) {
            report += "  " + step.description + " (" + std::to_string(step.changesApplied) + " changes)\n";
        }
        for (const auto& w : result.warnings) report += "WARNING: " + w + "\n";
        for (const auto& e : result.errors) report += "ERROR: " + e + "\n";
        return report;
    }

private:
    PluginVersionMigrator() = default;

    static constexpr APIVersion CURRENT_API_VERSION = { 3, 0 };

    mutable std::mutex m_mutex;
    std::vector<DeprecatedAPI> m_deprecations;
};

}
} // namespace ExplorerLens::Engine
