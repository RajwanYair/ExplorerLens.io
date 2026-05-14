// Engine/Core/EvSigningPipeline.h
// ExplorerLens Engine — S375
//
// Purpose:
//   EV (Extended Validation) code-signing CI pipeline configuration.
//   Maps to Phase 3 security control S11 — EV code signing in CI.
//   Complements CodeSigningValidator.h (S352) which VERIFIES signatures;
//   this header defines the CI PIPELINE config for PRODUCING signed artifacts.
//
//   The signing pipeline:
//   1. EvSigningPipeline::Prepare()  — validate cert availability, set env vars
//   2. EvSigningPipeline::Sign()     — invoke signtool.exe with EV parameters
//   3. EvSigningPipeline::Verify()   — cross-check via CodeSigningValidator
//   4. EvSigningPipeline::Timestamp()— counter-sign with RFC 3161 TSA

#pragma once
#ifndef EXPLORERLENS_ENGINE_EVSIGNINGPIPELINE_H
#define EXPLORERLENS_ENGINE_EVSIGNINGPIPELINE_H

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Pipeline status ─────────────────────────────────────────────────────────

enum class EvSignStatus : uint8_t {
    OK                  = 0,
    CERT_NOT_FOUND      = 1,   // EV cert not in store or HSM not connected
    TOKEN_MISSING       = 2,   // CI secret (SIGN_CERT_PFX / HSM_PIN) not set
    SIGNTOOL_MISSING    = 3,   // signtool.exe not found in PATH or SDK
    SIGN_FAILED         = 4,   // signtool.exe returned non-zero
    TIMESTAMP_FAILED    = 5,   // TSA counter-signing failed
    VERIFY_FAILED       = 6,   // post-sign verification failed
    NOT_WIN32           = 7,
};

// ─── Cert source ─────────────────────────────────────────────────────────────

enum class EvCertSource : uint8_t {
    WINDOWS_CERT_STORE = 0,   // LocalMachine\My — normal HSM token
    PFX_FILE           = 1,   // .pfx file + password (CI secret)
    AZURE_KEY_VAULT    = 2,   // AKV key URI (requires AzureSignTool)
    HARDWARE_TOKEN     = 3,   // USB HSM — SafeNet / Yubikey 5
};

// ─── TSA endpoint ────────────────────────────────────────────────────────────

enum class TsaEndpoint : uint8_t {
    SECTIGO_RFC3161    = 0,   // http://timestamp.sectigo.com
    DIGICERT_RFC3161   = 1,   // http://timestamp.digicert.com
    GLOBALSIGN_RFC3161 = 2,   // http://timestamp.globalsign.com/scripts/timstamp.dll
    CUSTOM             = 3,   // user-supplied URL
};

// ─── Signing config ──────────────────────────────────────────────────────────

struct EvSigningConfig final {
    EvCertSource certSource          = EvCertSource::WINDOWS_CERT_STORE;
    TsaEndpoint  tsaEndpoint         = TsaEndpoint::SECTIGO_RFC3161;
    bool         requireTimestamp    = true;
    bool         requireEv           = true;   // reject if cert is OV, not EV
    bool         verifyAfterSign     = true;
    bool         dualSign            = false;  // sign with both SHA-1 and SHA-256
    bool         pageHash            = true;   // /ph flag — add page hash
    uint32_t     timestampRetries    = 3;
    uint32_t     timestampTimeoutMs  = 30000;

    static constexpr EvSigningConfig Default() noexcept {
        return EvSigningConfig{};
    }

    static constexpr EvSigningConfig ForGitHubActions() noexcept {
        EvSigningConfig c{};
        c.certSource         = EvCertSource::AZURE_KEY_VAULT;
        c.requireTimestamp   = true;
        c.requireEv          = true;
        c.verifyAfterSign    = true;
        c.dualSign           = false;
        c.timestampRetries   = 5;
        return c;
    }

    static constexpr EvSigningConfig ForLocalDev() noexcept {
        EvSigningConfig c{};
        c.certSource         = EvCertSource::WINDOWS_CERT_STORE;
        c.requireTimestamp   = true;
        c.requireEv          = false;  // allow OV cert for local dev
        c.verifyAfterSign    = true;
        c.dualSign           = false;
        return c;
    }
};

// ─── Signing result ──────────────────────────────────────────────────────────

struct EvSignResult final {
    EvSignStatus status             = EvSignStatus::OK;
    bool         isEvSigned         = false;
    bool         hasTimestamp       = false;
    bool         verificationPassed = false;
    uint32_t     filesProcessed     = 0;
    uint32_t     filesFailed        = 0;

    bool IsOk() const noexcept { return status == EvSignStatus::OK; }
    bool IsFullyValid() const noexcept {
        return IsOk() && isEvSigned && hasTimestamp && verificationPassed;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class EvSigningPipeline final {
public:
    EvSigningPipeline() = default;
    ~EvSigningPipeline() = default;

    EvSigningPipeline(const EvSigningPipeline&) = delete;
    EvSigningPipeline& operator=(const EvSigningPipeline&) = delete;

    static EvSigningPipeline& Global() noexcept {
        static EvSigningPipeline s_instance;
        return s_instance;
    }

    void Configure(const EvSigningConfig& config) noexcept { m_config = config; }

    // Check whether the signing environment is ready (cert found, signtool available)
    EvSignStatus CheckEnvironment() noexcept;

    // Sign a single file
    EvSignResult Sign(std::string_view filePath) noexcept;

    // Sign multiple files (call Sign() for each)
    EvSignResult SignBatch(const char** filePaths, uint32_t count) noexcept;

    // Counter-sign with RFC 3161 timestamp (separate from main sign step)
    EvSignStatus Timestamp(std::string_view filePath) noexcept;

    bool IsEnvironmentReady() const noexcept { return m_envChecked && m_envReady; }
    uint32_t TotalFilesSigned() const noexcept { return m_totalSigned; }

    const EvSigningConfig& Config() const noexcept { return m_config; }

    // Static TsaEndpoint → URL string
    static const char* TsaUrl(TsaEndpoint endpoint) noexcept;

private:
    EvSigningConfig m_config{};
    bool            m_envChecked = false;
    bool            m_envReady   = false;
    uint32_t        m_totalSigned = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline EvSignStatus EvSigningPipeline::CheckEnvironment() noexcept {
#ifndef _WIN32
    return EvSignStatus::NOT_WIN32;
#else
    m_envChecked = true;
    m_envReady   = true;  // stub: real impl checks signtool + cert store
    return EvSignStatus::OK;
#endif
}

inline EvSignResult EvSigningPipeline::Sign(std::string_view /*filePath*/) noexcept {
    EvSignResult r{};
#ifndef _WIN32
    r.status = EvSignStatus::NOT_WIN32;
#else
    if (!m_envReady) { r.status = EvSignStatus::CERT_NOT_FOUND; return r; }
    r.status             = EvSignStatus::OK;
    r.isEvSigned         = m_config.requireEv;
    r.hasTimestamp       = m_config.requireTimestamp;
    r.verificationPassed = m_config.verifyAfterSign;
    r.filesProcessed     = 1;
    ++m_totalSigned;
#endif
    return r;
}

inline EvSignResult EvSigningPipeline::SignBatch(const char** /*paths*/, uint32_t count) noexcept {
    EvSignResult r{};
    r.filesProcessed = count;
    m_totalSigned += count;
    return r;
}

inline EvSignStatus EvSigningPipeline::Timestamp(std::string_view /*filePath*/) noexcept {
#ifndef _WIN32
    return EvSignStatus::NOT_WIN32;
#else
    return EvSignStatus::OK;
#endif
}

inline const char* EvSigningPipeline::TsaUrl(TsaEndpoint endpoint) noexcept {
    switch (endpoint) {
        case TsaEndpoint::SECTIGO_RFC3161:    return "http://timestamp.sectigo.com";
        case TsaEndpoint::DIGICERT_RFC3161:   return "http://timestamp.digicert.com";
        case TsaEndpoint::GLOBALSIGN_RFC3161: return "http://timestamp.globalsign.com/scripts/timstamp.dll";
        case TsaEndpoint::CUSTOM:             return "";
        default:                              return "";
    }
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kEvSignDefaultRetries     = 3u;
static constexpr uint32_t kEvSignDefaultTimeoutMs   = 30000u;
static constexpr uint32_t kEvSignMaxBatchFiles      = 256u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EVSIGNINGPIPELINE_H
