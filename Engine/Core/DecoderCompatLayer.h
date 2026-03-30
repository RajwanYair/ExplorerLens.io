// DecoderCompatLayer.h — Cross-Version Decoder Compatibility Shim
// Copyright (c) 2026 ExplorerLens Project
//
// Provides version-bridging shims that let older decoders work with the current
// engine API, generating compatibility reports and automatic migration suggestions.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class CompatMode : uint8_t {
    Strict = 0, Relaxed = 1, Legacy = 2, AutoDetect = 3
};

enum class CompatSeverity : uint8_t {
    Info = 0, Warning = 1, Error = 2, Breaking = 3
};

struct CompatIssue {
    std::string decoderName;
    std::string sourceVersion;
    std::string targetVersion;
    CompatSeverity severity = CompatSeverity::Warning;
    std::string workaround;
    std::string affectedApi;
    bool autoFixable = false;

    bool IsBreaking() const { return severity == CompatSeverity::Breaking; }
};

struct CompatShim {
    std::string name;
    std::string fromVersion;
    std::string toVersion;
    std::string description;
    std::function<bool()> applyFn;
    bool applied = false;
};

struct CompatReport {
    bool isCompatible = true;
    std::vector<CompatIssue> issues;
    uint32_t migrationsRequired = 0;
    uint32_t autoFixableCount = 0;
    std::string summary;

    size_t GetBreakingCount() const {
        size_t count = 0;
        for (const auto& i : issues)
            if (i.IsBreaking()) ++count;
        return count;
    }
};

class DecoderCompatLayer {
public:
    DecoderCompatLayer()
        : m_mode(CompatMode::AutoDetect), m_shimCount(0) {}

    ~DecoderCompatLayer() = default;

    CompatReport CheckCompat(const std::string& decoderName,
                             const std::string& sourceVersion,
                             const std::string& targetVersion) const {
        CompatReport report;
        report.isCompatible = true;
        for (const auto& known : m_knownIssues) {
            if (known.decoderName == decoderName &&
                known.sourceVersion == sourceVersion &&
                known.targetVersion == targetVersion) {
                report.issues.push_back(known);
                if (known.IsBreaking() && m_mode == CompatMode::Strict)
                    report.isCompatible = false;
                if (known.autoFixable)
                    ++report.autoFixableCount;
                else
                    ++report.migrationsRequired;
            }
        }
        report.summary = decoderName + " " + sourceVersion + " -> " + targetVersion +
            ": " + std::to_string(report.issues.size()) + " issue(s)";
        return report;
    }

    bool ApplyShim(const std::string& shimName) {
        for (auto& shim : m_shims) {
            if (shim.name == shimName && !shim.applied) {
                if (shim.applyFn && shim.applyFn()) {
                    shim.applied = true;
                    ++m_shimCount;
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    CompatReport GenerateReport() const {
        CompatReport report;
        report.issues = m_knownIssues;
        report.isCompatible = true;
        for (const auto& issue : m_knownIssues) {
            if (issue.IsBreaking()) { report.isCompatible = false; break; }
        }
        report.migrationsRequired = 0;
        for (const auto& i : m_knownIssues)
            if (!i.autoFixable) ++report.migrationsRequired;
        report.summary = std::to_string(m_knownIssues.size()) + " known issues, " +
                         std::to_string(m_shims.size()) + " shims available";
        return report;
    }

    void RegisterIssue(const CompatIssue& issue) { m_knownIssues.push_back(issue); }

    void RegisterShim(CompatShim shim) { m_shims.push_back(std::move(shim)); }

    void SetMode(CompatMode mode) { m_mode = mode; }
    CompatMode GetMode() const { return m_mode; }
    size_t GetShimCount() const { return m_shimCount; }
    size_t GetKnownIssueCount() const { return m_knownIssues.size(); }

private:
    CompatMode m_mode;
    size_t m_shimCount;
    std::vector<CompatIssue> m_knownIssues;
    std::vector<CompatShim> m_shims;
};

}} // namespace ExplorerLens::Engine
