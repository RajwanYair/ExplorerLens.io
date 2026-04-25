// ============================================================================
// EvCodeSigningPipelineContract.h -- S287 / ROADMAP v6.0 S5 EV signing
//
// Phase 4 contract: codify the EV (Extended Validation) code-signing pipeline
// for `LENSShell.dll`, `LENSManager.exe`, and MSI/MSIX artifacts.  Header-only.
// Declares signer order, timestamp-server rotation, and verification gates.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class EvSigningAlgorithm : uint8_t
{
    SHA256_RSA     = 0,
    SHA384_ECDSA   = 1,
    SHA512_RSA_PSS = 2,
};

enum class EvSigningArtifactKind : uint8_t
{
    WIN_DLL            = 0,
    WIN_EXE            = 1,
    MSI_INSTALLER      = 2,
    MSIX_PACKAGE       = 3,
    APPX_BUNDLE        = 4,
    PE_DRIVER          = 5,   // future
    CATALOG_CAT        = 6,
};

struct EvSigningTimestampUrl
{
    const char* url;
    bool        rfc3161;       // true = RFC3161, false = Authenticode v1
    uint8_t     priority;      // lower = preferred
};

inline constexpr EvSigningTimestampUrl kEvSigningTimestampUrls[] = {
    { "http://timestamp.digicert.com",               true,  1 },
    { "http://timestamp.sectigo.com",                true,  2 },
    { "http://ts.ssl.com",                           true,  3 },
    { "http://timestamp.globalsign.com/tsa/r6advanced1", true, 4 },
};
inline constexpr size_t kEvSigningTimestampUrlCount =
    sizeof(kEvSigningTimestampUrls) / sizeof(kEvSigningTimestampUrls[0]);

enum class EvSigningStatus : uint8_t
{
    OK                        = 0,
    HSM_UNREACHABLE           = 1,
    CERT_EXPIRED              = 2,
    CERT_REVOKED              = 3,
    TIMESTAMP_FAILED_ALL_TSA  = 4,
    DUAL_SIGN_INSERT_FAILED   = 5,
    VERIFY_FAILED             = 6,
    BUDGET_EXCEEDED           = 7,
};

struct EvSigningPolicy
{
    EvSigningAlgorithm primaryAlgorithm    = EvSigningAlgorithm::SHA256_RSA;
    EvSigningAlgorithm secondaryAlgorithm  = EvSigningAlgorithm::SHA384_ECDSA;
    bool               requireDualSignature = true;
    bool               requireRfc3161Timestamp = true;
    bool               requireSmartScreenReputationCheck = true;
    uint32_t           signBudgetMsPerArtifact = 15000;
    uint32_t           verifyBudgetMsPerArtifact = 5000;
};

inline constexpr uint32_t kEvSigningHardBudgetMsPerArtifact = 60000;

static_assert(kEvSigningTimestampUrlCount == 4,
              "EV signing timestamp rotation must have 4 URLs");
static_assert(std::is_trivially_copyable_v<EvSigningPolicy>,
              "EvSigningPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<EvSigningTimestampUrl>,
              "EvSigningTimestampUrl must be trivially copyable");

} // namespace ExplorerLens::Engine
