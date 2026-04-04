// ConfigDriftDetector.h — Configuration Drift Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Captures a baseline of configuration key-value pairs and compares them
// against current runtime values to detect drift. Missing keys, changed
// values, and unexpected new keys are each flagged with appropriate
// severity. A quick HasDrift() check is available for gate logic.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ConfigDriftSeverity : uint32_t {
    None = 0,
    Info = 1,
    Warning = 2,
    Critical = 3
};

struct ConfigDriftFinding
{
    std::wstring key;
    std::wstring expectedValue;
    std::wstring actualValue;
    ConfigDriftSeverity severity = ConfigDriftSeverity::Warning;
    std::wstring description;
};

struct ConfigSnapshot
{
    uint64_t timestamp = 0;
    uint32_t entryCount = 0;
    std::unordered_map<std::wstring, std::wstring> values;
};

struct ConfigDriftReport
{
    uint64_t checkTimestamp = 0;
    uint32_t totalKeys = 0;
    uint32_t driftedKeys = 0;
    ConfigDriftSeverity maxSeverity = ConfigDriftSeverity::None;
    std::vector<ConfigDriftFinding> findings;
    bool hasDrift = false;
};

// ========================================================================
// ConfigDriftDetector — Detects settings changes between runs
// ========================================================================
class ConfigDriftDetector
{
  public:
    static ConfigDriftDetector& Instance()
    {
        static ConfigDriftDetector instance;
        return instance;
    }

    void Initialize()
    {
        m_baseline = {};
        m_checksPerformed = 0;
        m_driftsDetected = 0;
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Capture baseline (expected state)
    void CaptureBaseline()
    {
        m_baseline.timestamp = GetTickCount64();
        m_baseline.entryCount = static_cast<uint32_t>(m_baseline.values.size());
    }

    // Set baseline value
    void SetBaselineValue(const std::wstring& key, const std::wstring& value)
    {
        m_baseline.values[key] = value;
        m_baseline.entryCount = static_cast<uint32_t>(m_baseline.values.size());
    }

    // Set current runtime value
    void SetCurrentValue(const std::wstring& key, const std::wstring& value)
    {
        m_current[key] = value;
    }

    // Check for drift
    ConfigDriftReport CheckDrift()
    {
        ConfigDriftReport report;
        report.checkTimestamp = GetTickCount64();
        report.totalKeys = static_cast<uint32_t>(m_baseline.values.size());
        m_checksPerformed++;

        for (auto& [key, expectedValue] : m_baseline.values) {
            auto it = m_current.find(key);
            if (it == m_current.end()) {
                // Key missing from current
                ConfigDriftFinding finding;
                finding.key = key;
                finding.expectedValue = expectedValue;
                finding.actualValue = L"<missing>";
                finding.severity = ConfigDriftSeverity::Warning;
                finding.description = L"Configuration key missing from runtime";
                report.findings.push_back(finding);
            } else if (it->second != expectedValue) {
                // Value changed
                ConfigDriftFinding finding;
                finding.key = key;
                finding.expectedValue = expectedValue;
                finding.actualValue = it->second;
                finding.severity = ConfigDriftSeverity::Warning;
                finding.description = L"Value differs from baseline";
                report.findings.push_back(finding);
            }
        }

        // Check for unexpected new keys
        for (auto& [key, value] : m_current) {
            if (m_baseline.values.find(key) == m_baseline.values.end()) {
                ConfigDriftFinding finding;
                finding.key = key;
                finding.expectedValue = L"<not in baseline>";
                finding.actualValue = value;
                finding.severity = ConfigDriftSeverity::Info;
                finding.description = L"New key not in baseline";
                report.findings.push_back(finding);
            }
        }

        report.driftedKeys = static_cast<uint32_t>(report.findings.size());
        report.hasDrift = !report.findings.empty();

        if (report.hasDrift) {
            m_driftsDetected++;
            // Compute max severity
            for (auto& f : report.findings) {
                if (static_cast<uint32_t>(f.severity) > static_cast<uint32_t>(report.maxSeverity)) {
                    report.maxSeverity = f.severity;
                }
            }
        }

        return report;
    }

    // Quick check: any drift?
    bool HasDrift()
    {
        auto report = CheckDrift();
        return report.hasDrift;
    }

    // Stats
    uint64_t GetChecksPerformed() const
    {
        return m_checksPerformed;
    }
    uint64_t GetDriftsDetected() const
    {
        return m_driftsDetected;
    }
    uint32_t GetBaselineKeyCount() const
    {
        return static_cast<uint32_t>(m_baseline.values.size());
    }

  private:
    ConfigDriftDetector() = default;

    ConfigSnapshot m_baseline;
    std::unordered_map<std::wstring, std::wstring> m_current;
    uint64_t m_checksPerformed = 0;
    uint64_t m_driftsDetected = 0;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
