// =============================================================================
// ExplorerLens Engine — CodeSigningValidator.h
// Sprint S352 | ROADMAP v8.0 Phase 3 (Security)
// PE code signature verification for shell extension binaries.
// Validates Authenticode signatures on LENSShell.dll and LENSManager.exe
// using WinTrust (WinVerifyTrust) API.
//
// Phase 3 score table: "code signing" → Category A dimension "Code-signed
// binaries". This header provides the runtime check contract.
//
// Windows-only. Non-Windows builds compile to stubs returning NOT_WIN32.
// =============================================================================
#pragma once

#include <cstdint>
#include <string>

#ifndef EXPLORERLENS_ENGINE_CODESIGNINGVALIDATOR_H
#define EXPLORERLENS_ENGINE_CODESIGNINGVALIDATOR_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// PeSignatureStatus — result of a single WinVerifyTrust call
// ---------------------------------------------------------------------------
enum class PeSignatureStatus : uint8_t {
    VALID           = 0,  ///< Signature present, chain trusted, not expired
    NOT_SIGNED      = 1,  ///< No Authenticode signature embedded
    CHAIN_NOT_TRUSTED = 2, ///< Signature present but chain not trusted (self-signed)
    EXPIRED         = 3,  ///< Certificate has expired
    REVOKED         = 4,  ///< Certificate revoked
    HASH_MISMATCH   = 5,  ///< File modified after signing (hash failure)
    NOT_WIN32       = 6,  ///< Non-Windows build — check not available
    FILE_NOT_FOUND  = 7,  ///< Target binary path does not exist
    WINTRUST_ERROR  = 8,  ///< WinTrust returned an unexpected HRESULT
    NULL_PATH       = 9,  ///< Path argument is null or empty
};

// ---------------------------------------------------------------------------
// SigningCertInfo — certificate metadata extracted from a valid PE signature
// ---------------------------------------------------------------------------
struct SigningCertInfo final {
    std::wstring subjectName;       ///< e.g. "CN=RajwanYair, O=ExplorerLens"
    std::wstring issuerName;        ///< Issuing CA common name
    std::wstring thumbprintHex;     ///< SHA-1 thumbprint, 40 hex chars
    std::wstring serialNumber;      ///< Certificate serial number hex string
    int64_t      notBeforeUnixSec{0};  ///< Valid from (Unix timestamp)
    int64_t      notAfterUnixSec{0};   ///< Valid to   (Unix timestamp)
    bool         isEvCertificate{false}; ///< Extended Validation certificate
};

// ---------------------------------------------------------------------------
// CodeSigningValidationResult — combined outcome for one binary
// ---------------------------------------------------------------------------
struct CodeSigningValidationResult final {
    PeSignatureStatus status{PeSignatureStatus::NOT_SIGNED};
    std::wstring      binaryPath;
    SigningCertInfo   cert;           ///< Populated only when status == VALID
    uint32_t          winTrustHResult{0}; ///< Raw HRESULT from WinVerifyTrust

    [[nodiscard]] bool IsValid()       const noexcept { return status == PeSignatureStatus::VALID; }
    [[nodiscard]] bool IsSigned()      const noexcept {
        return status != PeSignatureStatus::NOT_SIGNED
            && status != PeSignatureStatus::FILE_NOT_FOUND
            && status != PeSignatureStatus::NULL_PATH
            && status != PeSignatureStatus::NOT_WIN32;
    }
};

// ---------------------------------------------------------------------------
// CodeSigningConfig — validation policy
// ---------------------------------------------------------------------------
struct CodeSigningConfig final {
    bool  requireTrustedChain{true};   ///< Fail if chain is not system-trusted
    bool  requireNotExpired{true};     ///< Fail if cert is expired
    bool  requireEv{false};            ///< Optionally require EV cert
    bool  useRevocationCheck{true};    ///< Enable online/CRL revocation check
    /// Expected subject CN prefix for positive verification
    std::wstring expectedSubjectPrefix{L""};

    /// Default config for ExplorerLens release binaries
    [[nodiscard]] static CodeSigningConfig ForReleaseBuild() noexcept {
        CodeSigningConfig c;
        c.requireTrustedChain   = true;
        c.requireNotExpired     = true;
        c.requireEv             = false;
        c.useRevocationCheck    = true;
        c.expectedSubjectPrefix = L"ExplorerLens";
        return c;
    }

    /// Relaxed config for CI/dev builds (self-signed acceptable)
    [[nodiscard]] static CodeSigningConfig ForDevBuild() noexcept {
        CodeSigningConfig c;
        c.requireTrustedChain   = false;
        c.requireNotExpired     = false;
        c.requireEv             = false;
        c.useRevocationCheck    = false;
        return c;
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kWinTrustSubjectFileAction  = 0x00AAC056u; ///< WINTRUST_ACTION_GENERIC_VERIFY_V2 guid flag
static constexpr uint32_t kWinTrustUiNone             = 2u;          ///< WTD_UI_NONE
static constexpr uint32_t kWinTrustRevocationCheckNone = 0u;
static constexpr uint32_t kWinTrustRevocationCheckChain = 4u;        ///< WTD_REVOKE_WHOLECHAIN
static constexpr uint32_t kExpectedThumbprintHexLength  = 40u;       ///< SHA-1 = 20 bytes = 40 hex chars

// ---------------------------------------------------------------------------
// CodeSigningValidator — static verification class (Windows-only)
// ---------------------------------------------------------------------------
class CodeSigningValidator final {
public:
    CodeSigningValidator() = delete;

    /// Verify Authenticode signature on a PE binary.
    /// @param binaryPath  Full path to .dll or .exe
    /// @param cfg         Validation policy
    /// @return            Populated CodeSigningValidationResult
    [[nodiscard]] static CodeSigningValidationResult Verify(
        const std::wstring& binaryPath,
        const CodeSigningConfig& cfg = CodeSigningConfig{}) noexcept;

    /// Verify LENSShell.dll residing next to the running process image.
    [[nodiscard]] static CodeSigningValidationResult VerifyShellDll(
        const CodeSigningConfig& cfg = CodeSigningConfig{}) noexcept;

    /// Verify LENSManager.exe residing next to the running process image.
    [[nodiscard]] static CodeSigningValidationResult VerifyManagerExe(
        const CodeSigningConfig& cfg = CodeSigningConfig{}) noexcept;

    /// Extract certificate metadata without enforcing policy.
    /// Returns default-constructed SigningCertInfo on failure.
    [[nodiscard]] static SigningCertInfo ExtractCertInfo(
        const std::wstring& binaryPath) noexcept;

    /// Quick boolean — returns true only when Verify() would return VALID.
    [[nodiscard]] static bool IsTrustedSigned(
        const std::wstring& binaryPath,
        const CodeSigningConfig& cfg = CodeSigningConfig{}) noexcept;

private:
#ifdef _WIN32
    /// Internal: call WinVerifyTrust and map HRESULT to PeSignatureStatus.
    [[nodiscard]] static PeSignatureStatus RunWinTrust(
        const std::wstring& path,
        bool revocationCheck,
        uint32_t& outHResult) noexcept;
#endif // _WIN32
};

// ---------------------------------------------------------------------------
// Inline stub bodies (non-Windows / no WinTrust linkage)
// ---------------------------------------------------------------------------
#ifndef _WIN32

inline CodeSigningValidationResult CodeSigningValidator::Verify(
    const std::wstring& binaryPath,
    const CodeSigningConfig& /*cfg*/) noexcept
{
    CodeSigningValidationResult r;
    r.binaryPath = binaryPath;
    r.status     = PeSignatureStatus::NOT_WIN32;
    return r;
}

inline CodeSigningValidationResult CodeSigningValidator::VerifyShellDll(
    const CodeSigningConfig& cfg) noexcept
{
    return Verify(L"LENSShell.dll", cfg);
}

inline CodeSigningValidationResult CodeSigningValidator::VerifyManagerExe(
    const CodeSigningConfig& cfg) noexcept
{
    return Verify(L"LENSManager.exe", cfg);
}

inline SigningCertInfo CodeSigningValidator::ExtractCertInfo(
    const std::wstring& /*binaryPath*/) noexcept
{
    return SigningCertInfo{};
}

inline bool CodeSigningValidator::IsTrustedSigned(
    const std::wstring& binaryPath,
    const CodeSigningConfig& cfg) noexcept
{
    return Verify(binaryPath, cfg).IsValid();
}

#endif // !_WIN32

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CODESIGNINGVALIDATOR_H
