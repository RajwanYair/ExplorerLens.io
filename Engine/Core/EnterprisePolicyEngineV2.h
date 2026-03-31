// EnterprisePolicyEngineV2.h — Enterprise Policy Engine V2 (GPO/Intune/ConfigMgr)
// Copyright (c) 2026 ExplorerLens Project
//
// Hierarchical policy resolution engine for enterprise deployments.
// Policy source priority (highest → lowest):
//   1. Group Policy Objects (GPO) via HKLM\Software\Policies\ExplorerLens
//   2. Microsoft Intune (MDM) via ./Vendor/MSFT/Policy CSP bridge
//   3. Microsoft Configuration Manager (SCCM) via WMI root\ccm\policy namespace
//   4. Manual/admin overrides via HKLM\Software\ExplorerLens\Admin
//   5. Per-user preferences via HKCU\Software\ExplorerLens
//
// All policy values are strongly-typed via PolicyValue variant and validated
// against a registry of valid settings before being applied.
//
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Policy source tier — used for audit logging and override precedence.
enum class PolicySource : uint8_t {
    GroupPolicy    = 0,  // HKLM\Software\Policies\ExplorerLens (highest)
    Intune         = 1,  // MDM/CSP
    ConfigMgr      = 2,  // SCCM WMI
    AdminManual    = 3,  // HKLM\Software\ExplorerLens\Admin
    UserPreference = 4,  // HKCU (lowest)
    Default        = 5,  // Built-in default (no policy set)
};

// Extended enterprise policy source (includes COUNT sentinel for static helpers)
enum class EnterprisePolicySource : uint8_t {
    GroupPolicy    = 0,
    Intune         = 1,
    ConfigMgr      = 2,
    AdminManual    = 3,
    UserPreference = 4,
    Default        = 5,
    COUNT          = 6
};

enum class PolicyComplianceStatus : uint8_t {
    Compliant    = 0,
    Partial      = 1,
    NonCompliant = 2,
    Unenforced   = 3,
    COUNT        = 4
};

enum class PolicyScope : uint8_t {
    Machine = 0,
    User    = 1,
    COUNT   = 2
};

struct EnterprisePolicyReport {
    uint32_t totalPolicies   = 0;
    uint32_t compliant       = 0;
    uint32_t nonCompliant    = 0;
    float    complianceScore = 0.0f;
};

// Typed policy value variant.
using PolicyValue = std::variant<
    bool,
    int32_t,
    uint32_t,
    std::string,
    std::vector<std::string>  // Allow-list / deny-list
>;

// A single resolved policy entry with provenance.
struct EnterprisePolicyEntry {
    std::string  key;             // e.g. "GPU.AllowedBackend"
    PolicyValue  value;
    PolicySource source;
    bool         enforced { false };  // True if GPO mandates this value
};

// Callback fired when a policy is applied or changed at runtime.
using PolicyChangeCallback =
    std::function<void(const EnterprisePolicyEntry& newValue,
                       const EnterprisePolicyEntry& oldValue)>;

// EnterprisePolicyEngineV2 — Hierarchical policy resolver.
//
// Call Load() at startup and whenever a WM_SETTINGCHANGE for policy is received.
// All engine components query this singleton to check their allowed configuration.
class EnterprisePolicyEngineV2 {
public:
    EnterprisePolicyEngineV2() noexcept {}
    ~EnterprisePolicyEngineV2() noexcept {}

    EnterprisePolicyEngineV2(const EnterprisePolicyEngineV2&)            = delete;
    EnterprisePolicyEngineV2& operator=(const EnterprisePolicyEngineV2&) = delete;

    // Load / refresh all policy sources.  Call at startup and on SETTINGCHANGE.
    void Load() noexcept;

    // Resolve a typed policy value.
    // Returns Default-sourced default if no policy is set.
    template<typename T>
    T Get(const std::string& key, const T& defaultValue) const noexcept;

    // Get raw EnterprisePolicyEntry with provenance info (for diagnostics page).
    bool TryGet(const std::string& key, EnterprisePolicyEntry& out) const noexcept;

    // Check if a feature is policy-disabled.
    bool IsFeatureDisabled(const std::string& featureKey) const noexcept;

    // Get all currently active policies (for audit export).
    std::vector<EnterprisePolicyEntry> GetAll() const noexcept;

    // Subscribe to policy changes (fired on next Load() if value changed).
    void OnPolicyChange(PolicyChangeCallback cb) noexcept;

    // Export current policy set as JSON for support/audit.
    std::string ExportJson() const noexcept;

    // Static helpers for UI / diagnostics
    static const wchar_t* SourceName(EnterprisePolicySource src) noexcept {
        switch (src) {
        case EnterprisePolicySource::GroupPolicy:    return L"Group Policy";
        case EnterprisePolicySource::Intune:         return L"Microsoft Intune";
        case EnterprisePolicySource::ConfigMgr:      return L"Configuration Manager";
        case EnterprisePolicySource::AdminManual:    return L"Admin Manual";
        case EnterprisePolicySource::UserPreference: return L"User Preference";
        case EnterprisePolicySource::Default:        return L"Default";
        default:                                     return L"Unknown";
        }
    }
    static const wchar_t* ComplianceStatusName(PolicyComplianceStatus s) noexcept {
        switch (s) {
        case PolicyComplianceStatus::Compliant:    return L"Compliant";
        case PolicyComplianceStatus::Partial:      return L"Partial";
        case PolicyComplianceStatus::NonCompliant: return L"Non-Compliant";
        case PolicyComplianceStatus::Unenforced:   return L"Unenforced";
        default:                                   return L"Unknown";
        }
    }
    static const wchar_t* ScopeName(PolicyScope scope) noexcept {
        return scope == PolicyScope::Machine ? L"Machine" : L"User";
    }
    static constexpr size_t SourceCount() noexcept {
        return static_cast<size_t>(EnterprisePolicySource::COUNT);
    }
    static constexpr size_t ComplianceStatusCount() noexcept {
        return static_cast<size_t>(PolicyComplianceStatus::COUNT);
    }
    static bool IsFullyCompliant(const EnterprisePolicyReport& r) noexcept {
        return r.nonCompliant == 0 && r.complianceScore >= 100.0f;
    }

    // Singleton.
    static EnterprisePolicyEngineV2& Instance() noexcept {
        static EnterprisePolicyEngineV2 s_instance;
        return s_instance;
    }

private:
    struct Impl;
    Impl* m_impl { nullptr };
};

}} // namespace ExplorerLens::Engine
