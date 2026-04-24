// ============================================================================
// PluginTrustChainValidatorV1.h -- S268 / ROADMAP v6.0 S7 plugin trust
//
// Phase 3 validator contract for verifying third-party decoder plugins.
// Complements the pre-existing PluginTrustLevelV3 in PluginMarketplaceUnified
// by locking the *file-on-disk* validation sequence executed before the DLL
// is handed to LoadLibraryEx.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class PluginTrustChainV1Stage : uint8_t
{
    NONE                    = 0,
    FILE_HASH_SHA256        = 1,  // match published manifest hash
    AUTHENTICODE_CHECK      = 2,  // WinVerifyTrust
    PUBLISHER_ALLOWLIST     = 3,  // CN/OU/O vs allowlisted publishers
    CERT_PINNING            = 4,  // intermediate CA pin
    ABI_VERSION_CHECK       = 5,  // plugin_api.h ABI magic
    CAPABILITY_MANIFEST     = 6,  // requested caps vs decoder needs
    SANDBOX_COMPAT_PROBE    = 7,  // can run under AppContainer/LPAC
    REVOCATION_LIST_CHECK   = 8,  // offline CRL embedded in LENSManager
};

enum class PluginTrustChainV1Verdict : uint8_t
{
    TRUSTED                 = 0,
    TRUSTED_WITH_WARNINGS   = 1,
    UNSIGNED                = 2,
    PUBLISHER_UNKNOWN       = 3,
    SIGNATURE_INVALID       = 4,
    HASH_MISMATCH           = 5,
    ABI_INCOMPATIBLE        = 6,
    CAPABILITY_DENIED       = 7,
    SANDBOX_INCOMPATIBLE    = 8,
    REVOKED                 = 9,
    STAGE_TIMEOUT           = 10,
};

struct PluginTrustChainV1Policy
{
    bool     requireAuthenticode      = true;
    bool     requirePublisherAllowlist = true;
    bool     requireCertPinning        = false;
    bool     requireSandboxCompat      = true;
    bool     allowUnsignedDevBuilds    = false;  // only set in -DDEV_MODE builds
    uint32_t stageBudgetMs             = 500;    // per-stage timeout
    uint32_t totalBudgetMs             = 3000;
};

struct PluginTrustChainV1Report
{
    PluginTrustChainV1Verdict verdict = PluginTrustChainV1Verdict::TRUSTED;
    PluginTrustChainV1Stage   failingStage = PluginTrustChainV1Stage::NONE;
    uint32_t                  stagesPassed = 0;
    uint32_t                  elapsedMs    = 0;
    bool                      sawWarnings  = false;
};

inline constexpr uint32_t kPluginTrustChainV1StageCount = 8;
inline constexpr uint32_t kPluginTrustChainV1DefaultBudgetMs = 3000;
inline constexpr uint32_t kPluginTrustChainV1HardBudgetMs    = 10000;

static_assert(std::is_trivially_copyable_v<PluginTrustChainV1Policy>,
              "PluginTrustChainV1Policy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PluginTrustChainV1Report>,
              "PluginTrustChainV1Report must be trivially copyable");
static_assert(kPluginTrustChainV1DefaultBudgetMs < kPluginTrustChainV1HardBudgetMs,
              "plugin trust default budget < hard budget");

} // namespace ExplorerLens::Engine
