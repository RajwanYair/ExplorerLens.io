// PluginVersionNegotiator.h — SDK Version Compatibility Negotiation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides semantic version comparison, range-based matching, and
// migration advisory for plugin SDK compatibility. Ensures plugins
// built against older SDK versions can be safely loaded or rejected.
//
#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Current SDK version constant
constexpr uint32_t SDK_VERSION_MAJOR = 15;
constexpr uint32_t SDK_VERSION_MINOR = 0;
constexpr uint32_t SDK_VERSION_PATCH = 0;

// Semantic version triplet
struct SemanticVersion {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t patch = 0;

    constexpr SemanticVersion() = default;
    constexpr SemanticVersion(uint32_t maj, uint32_t min, uint32_t pat)
        : major(maj), minor(min), patch(pat) {}

    constexpr bool operator==(const SemanticVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    constexpr bool operator!=(const SemanticVersion& o) const { return !(*this == o); }

    constexpr bool operator<(const SemanticVersion& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    constexpr bool operator<=(const SemanticVersion& o) const { return !(o < *this); }
    constexpr bool operator>(const SemanticVersion& o) const { return o < *this; }
    constexpr bool operator>=(const SemanticVersion& o) const { return !(*this < o); }

    std::wstring ToString() const {
        return std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(patch);
    }

    // Pack into single 32-bit integer for fast comparison
    constexpr uint32_t Packed() const {
        return (major << 22) | (minor << 12) | patch;
    }

    static SemanticVersion Parse(const std::wstring& str) {
        SemanticVersion v;
        // Parse "major.minor.patch"
        size_t pos1 = str.find(L'.');
        if (pos1 == std::wstring::npos) {
            v.major = static_cast<uint32_t>(std::wcstoul(str.c_str(), nullptr, 10));
            return v;
        }
        v.major = static_cast<uint32_t>(std::wcstoul(str.substr(0, pos1).c_str(), nullptr, 10));
        size_t pos2 = str.find(L'.', pos1 + 1);
        if (pos2 == std::wstring::npos) {
            v.minor = static_cast<uint32_t>(std::wcstoul(str.substr(pos1 + 1).c_str(), nullptr, 10));
            return v;
        }
        v.minor = static_cast<uint32_t>(std::wcstoul(str.substr(pos1 + 1, pos2 - pos1 - 1).c_str(), nullptr, 10));
        v.patch = static_cast<uint32_t>(std::wcstoul(str.substr(pos2 + 1).c_str(), nullptr, 10));
        return v;
    }
};

// Current SDK version
constexpr SemanticVersion SDK_VERSION{SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_PATCH};

// Compatibility result of version comparison
enum class CompatibilityResult : uint32_t {
    Compatible      = 0,   // Exact or patch-level difference
    MinorMismatch   = 1,   // Different minor version, same major (backward compat likely)
    MajorBreaking   = 2,   // Different major version (breaking change)
    Unknown         = 3    // Cannot determine compatibility
};

inline const wchar_t* CompatibilityResultName(CompatibilityResult r) {
    switch (r) {
        case CompatibilityResult::Compatible:    return L"Compatible";
        case CompatibilityResult::MinorMismatch: return L"MinorMismatch";
        case CompatibilityResult::MajorBreaking: return L"MajorBreaking";
        case CompatibilityResult::Unknown:       return L"Unknown";
        default:                                 return L"Unknown";
    }
}

// Version range for matching (inclusive)
struct SDKVersionRange {
    SemanticVersion minVersion;
    SemanticVersion maxVersion;

    bool Contains(const SemanticVersion& v) const {
        return v >= minVersion && v <= maxVersion;
    }
};

// Migration step suggestion
struct MigrationStep {
    SemanticVersion fromVersion;
    SemanticVersion toVersion;
    std::wstring description;
    bool breaking = false;
};

// ========================================================================
// PluginVersionNegotiator — version check + migration advisory
// ========================================================================
class PluginVersionNegotiator {
public:
    static PluginVersionNegotiator& Instance() {
        static PluginVersionNegotiator instance;
        return instance;
    }

    void Initialize() {
        m_initialized = true;
        m_migrationSteps.clear();
        // Register known migration steps
        RegisterDefaultMigrations();
    }

    bool IsInitialized() const { return m_initialized; }

    // Get current SDK version
    SemanticVersion GetSDKVersion() const { return SDK_VERSION; }

    // Check compatibility between plugin version and SDK
    CompatibilityResult CheckCompatibility(const SemanticVersion& pluginVersion) const {
        return CheckCompatibility(pluginVersion, SDK_VERSION);
    }

    // Check compatibility between two arbitrary versions
    CompatibilityResult CheckCompatibility(const SemanticVersion& pluginVersion,
                                           const SemanticVersion& sdkVersion) const {
        if (pluginVersion.major == 0 && pluginVersion.minor == 0 && pluginVersion.patch == 0) {
            return CompatibilityResult::Unknown;
        }

        if (pluginVersion.major != sdkVersion.major) {
            return CompatibilityResult::MajorBreaking;
        }

        if (pluginVersion.minor != sdkVersion.minor) {
            return CompatibilityResult::MinorMismatch;
        }

        return CompatibilityResult::Compatible;
    }

    // Range-based version matching
    bool IsInRange(const SemanticVersion& version, const SDKVersionRange& range) const {
        return range.Contains(version);
    }

    // Register a migration step
    void RegisterMigration(const MigrationStep& step) {
        m_migrationSteps.push_back(step);
    }

    // Get migration path from one version to another
    std::vector<MigrationStep> GetMigrationPath(const SemanticVersion& fromVer,
                                                  const SemanticVersion& toVer) const {
        std::vector<MigrationStep> path;
        for (auto& step : m_migrationSteps) {
            if (step.fromVersion >= fromVer && step.toVersion <= toVer) {
                path.push_back(step);
            }
        }
        return path;
    }

    // Get upgrade suggestion string
    std::wstring GetUpgradeSuggestion(const SemanticVersion& pluginVersion) const {
        auto compat = CheckCompatibility(pluginVersion);
        switch (compat) {
            case CompatibilityResult::Compatible:
                return L"Plugin is compatible with current SDK.";
            case CompatibilityResult::MinorMismatch:
                return L"Minor version mismatch. Plugin may work but upgrade recommended to "
                       + SDK_VERSION.ToString() + L".";
            case CompatibilityResult::MajorBreaking:
                return L"Major version mismatch. Plugin must be recompiled for SDK "
                       + SDK_VERSION.ToString() + L".";
            case CompatibilityResult::Unknown:
            default:
                return L"Cannot determine compatibility. Verify plugin SDK version.";
        }
    }

    // Get all registered migration steps
    const std::vector<MigrationStep>& GetMigrationSteps() const { return m_migrationSteps; }

private:
    PluginVersionNegotiator() = default;

    void RegisterDefaultMigrations() {
        m_migrationSteps.push_back({
            SemanticVersion{13, 0, 0}, SemanticVersion{14, 0, 0},
            L"v13 to v14: Plugin API restructured, ICADDecoderPlugin added.", true
        });
        m_migrationSteps.push_back({
            SemanticVersion{14, 0, 0}, SemanticVersion{15, 0, 0},
            L"v14 to v15: Capability-based security model, resource limits enforced.", true
        });
    }

    bool m_initialized = false;
    std::vector<MigrationStep> m_migrationSteps;
};

} // namespace Engine
} // namespace ExplorerLens
