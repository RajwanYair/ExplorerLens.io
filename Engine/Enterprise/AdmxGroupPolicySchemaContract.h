// ============================================================================
// AdmxGroupPolicySchemaContract.h -- S289 / ROADMAP v6.0 H1 ADMX templates
//
// Phase 4 contract: expose ExplorerLens enterprise settings via Group Policy
// (ADMX + ADML).  Header-only.  Declares the canonical policy-key table
// (15 keys) that the ADMX generator and the Engine settings loader must
// agree on.  Registry hive is HKLM\Software\Policies\ExplorerLens.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class AdmxPolicyValueKind : uint8_t
{
    DWORD          = 0,
    QWORD          = 1,
    SZ             = 2,
    MULTI_SZ       = 3,
    BOOLEAN        = 4,   // DWORD 0/1
    ENUM_DWORD     = 5,
};

enum class AdmxPolicyCategory : uint8_t
{
    PERFORMANCE   = 0,
    SECURITY      = 1,
    PRIVACY       = 2,
    DIAGNOSTICS   = 3,
    UPDATES       = 4,
    PLUGINS       = 5,
    EXPLORER      = 6,
};

struct AdmxPolicyKey
{
    const char*         registryValueName;   // e.g. "MaxCacheMB"
    AdmxPolicyValueKind kind;
    AdmxPolicyCategory  category;
    bool                requiresRestart;
};

inline constexpr AdmxPolicyKey kAdmxPolicyKeys[] = {
    // Performance
    { "MaxCacheMB",              AdmxPolicyValueKind::DWORD,       AdmxPolicyCategory::PERFORMANCE, false },
    { "DecodeBudgetMs",          AdmxPolicyValueKind::DWORD,       AdmxPolicyCategory::PERFORMANCE, false },
    { "EnableGpuAcceleration",   AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::PERFORMANCE, true  },
    // Security
    { "AllowPlugins",            AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::SECURITY,    true  },
    { "PluginAllowlist",         AdmxPolicyValueKind::MULTI_SZ,    AdmxPolicyCategory::SECURITY,    true  },
    { "RequireSignedDecoders",   AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::SECURITY,    true  },
    // Privacy
    { "TelemetryLevel",          AdmxPolicyValueKind::ENUM_DWORD,  AdmxPolicyCategory::PRIVACY,     false },
    { "DisableWerSubmission",    AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::PRIVACY,     false },
    // Diagnostics
    { "EnableEtw",               AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::DIAGNOSTICS, false },
    { "LogLevel",                AdmxPolicyValueKind::ENUM_DWORD,  AdmxPolicyCategory::DIAGNOSTICS, false },
    // Updates
    { "DisableAutoUpdate",       AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::UPDATES,     false },
    { "UpdateChannel",           AdmxPolicyValueKind::ENUM_DWORD,  AdmxPolicyCategory::UPDATES,     false },
    // Plugins
    { "PluginSandboxLevel",      AdmxPolicyValueKind::ENUM_DWORD,  AdmxPolicyCategory::PLUGINS,     true  },
    // Explorer surface
    { "DisabledFormats",         AdmxPolicyValueKind::MULTI_SZ,    AdmxPolicyCategory::EXPLORER,    true  },
    { "EnableSpacebarPreview",   AdmxPolicyValueKind::BOOLEAN,     AdmxPolicyCategory::EXPLORER,    false },
};
inline constexpr size_t kAdmxPolicyKeyCount =
    sizeof(kAdmxPolicyKeys) / sizeof(kAdmxPolicyKeys[0]);

inline constexpr const char* kAdmxPolicyRegistryRoot =
    "HKLM\\Software\\Policies\\ExplorerLens";
inline constexpr const char* kAdmxPolicyNamespace =
    "ExplorerLens.Policies";

static_assert(kAdmxPolicyKeyCount == 15, "ADMX policy schema must have 15 keys");
static_assert(std::is_trivially_copyable_v<AdmxPolicyKey>,
              "AdmxPolicyKey must be trivially copyable");

} // namespace ExplorerLens::Engine
