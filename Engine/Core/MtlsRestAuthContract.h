// MtlsRestAuthContract.h -- S295 / ROADMAP v6.0 S12 Phase 4
// Mutual TLS (mTLS) authentication policy for the ExplorerLens REST API.
//
// mTLS enforces that both server and client present X.509 certificates
// signed by a trusted CA. Used in enterprise network-isolated deployments.
// Default mode is DISABLED — requires explicit opt-in via Group Policy.
//
// Integrates with LensRestApiEndpointContract (S291) when
// LensRestApiAuthMode::MTLS is selected.
//
// Rule: contract header only — no implementation, no Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── mTLS Mode ────────────────────────────────────────────────────────────────

enum class MtlsRestAuthMode : uint8_t {
    DISABLED             = 0,  // Default: Bearer token only
    SERVER_CERT_ONLY     = 1,  // TLS server cert, no client cert required
    MUTUAL_TLS           = 2,  // Both server + client cert required
    MUTUAL_TLS_WITH_OCSP = 3,  // mTLS + real-time OCSP revocation check
};

// ── Certificate Validation Level ─────────────────────────────────────────────

enum class MtlsRestCertValidation : uint8_t {
    STRICT_MODE = 0,  // Chain + hostname + expiry + revocation (default)
    STANDARD    = 1,  // Chain + expiry (no revocation check)
    PINNED      = 2,  // SHA-256 public-key pin only
};

// ── Auth Policy ───────────────────────────────────────────────────────────────

struct MtlsRestAuthPolicy {
    MtlsRestAuthMode       mode               = MtlsRestAuthMode::DISABLED;
    MtlsRestCertValidation clientValidation   = MtlsRestCertValidation::STRICT_MODE;
    uint32_t               handshakeTimeoutMs = 5000;   // TLS handshake budget
    uint32_t               sessionLifetimeMs  = 3600000; // 1 hour session reuse
    bool                   requireSan         = true;    // Require Subject Alt Name
    bool                   allowSelfSigned    = false;   // Never in production
};

// ── Probe ────────────────────────────────────────────────────────────────────

struct MtlsRestAuthProbe {
    MtlsRestAuthMode activeMode          = MtlsRestAuthMode::DISABLED;
    uint32_t         handshakesCompleted = 0;
    uint32_t         handshakesFailed    = 0;
    uint32_t         certRejectionsTotal = 0;
    bool             ocspResponderUp     = false;
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint32_t kMtlsRestAuthHandshakeHardMs  = 15000u;  // 15 s hard cap
static constexpr uint32_t kMtlsRestAuthSessionHardMs    = 86400000u; // 24 h max session
static constexpr uint8_t  kMtlsRestAuthSchemaVersion    = 1;

} // namespace Engine
} // namespace ExplorerLens
