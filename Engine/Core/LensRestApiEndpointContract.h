// LensRestApiEndpointContract.h -- S291 / ROADMAP v6.0 H11/H22 Phase 4
// REST API server endpoint manifest for ExplorerLens (cpp-httplib, no Boost).
//
// Stateless decode service: POST /v1/decode -> thumbnail + metadata JSON.
// Bearer token auth; mTLS enforced via MtlsRestAuthContract (S295).
// 7 canonical endpoints registered in compile-time table.
//
// Rule: this is a contract header — no implementation, no #include of Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>
#include <cstddef>

namespace ExplorerLens {
namespace Engine {

// ── HTTP Method ──────────────────────────────────────────────────────────────

enum class LensRestApiMethod : uint8_t {
    GET         = 0,
    POST        = 1,
    HTTP_DELETE = 2,
};

// ── Auth Mode ────────────────────────────────────────────────────────────────

enum class LensRestApiAuthMode : uint8_t {
    NONE        = 0,  // Public endpoint (health, version, formats)
    BEARER      = 1,  // Authorization: Bearer <sha256-hmac-token>
    MTLS        = 2,  // mTLS client certificate (Phase 4 enterprise)
};

// ── Endpoint Entry ───────────────────────────────────────────────────────────

struct LensRestApiEndpoint {
    const char*         path;             // URI path, e.g. "/v1/decode"
    LensRestApiMethod   method;
    LensRestApiAuthMode auth;
    uint32_t            softTimeoutMs;    // Target response time
    uint32_t            hardTimeoutMs;    // Abort if exceeded
    const char*         description;
};

// ── Endpoint Table ───────────────────────────────────────────────────────────
// 7 canonical REST endpoints defined in ROADMAP §8.3.

static constexpr size_t kLensRestApiEndpointCount = 7;

static constexpr LensRestApiEndpoint kLensRestApiEndpoints[kLensRestApiEndpointCount] = {
    // path                  method                          auth                        soft   hard
    { "/v1/decode",          LensRestApiMethod::POST,        LensRestApiAuthMode::BEARER,  200, 5000, "Decode file -> thumbnail (multipart)" },
    { "/v1/decode/batch",    LensRestApiMethod::POST,        LensRestApiAuthMode::BEARER, 2000,30000, "Batch decode list of paths" },
    { "/v1/formats",         LensRestApiMethod::GET,         LensRestApiAuthMode::NONE,     50,  500, "List supported formats as JSON" },
    { "/v1/cache/stats",     LensRestApiMethod::GET,         LensRestApiAuthMode::BEARER,   20,  200, "Cache hit/miss/size/eviction counts" },
    { "/v1/cache/purge",     LensRestApiMethod::HTTP_DELETE,      LensRestApiAuthMode::BEARER,  100, 1000, "Purge cache by format glob or all" },
    { "/v1/health",          LensRestApiMethod::GET,         LensRestApiAuthMode::NONE,      5,   50, "Liveness probe (Kubernetes-compatible)" },
    { "/v1/version",         LensRestApiMethod::GET,         LensRestApiAuthMode::NONE,      5,   50, "Version + build info JSON" },
};

static_assert(kLensRestApiEndpointCount == 7,
    "kLensRestApiEndpointCount must equal 7 — update when adding endpoints");

// ── Server Policy ────────────────────────────────────────────────────────────

struct LensRestApiServerPolicy {
    uint16_t listenPort            = 47382;   // Default port (not well-known)
    uint8_t  maxConcurrentRequests = 16;
    uint32_t requestBodyLimitBytes = 32u * 1024u * 1024u;  // 32 MiB
    bool     requireLocalhost      = true;    // Bind 127.0.0.1 by default
    bool     enableMtls            = false;   // Set true for enterprise mTLS
};

// ── Liveness Probe ───────────────────────────────────────────────────────────

struct LensRestApiProbe {
    bool     engineReady     = false;
    bool     cacheReady      = false;
    uint32_t uptimeSeconds   = 0;
    uint32_t requestsHandled = 0;
};

// ── Constants ────────────────────────────────────────────────────────────────

static constexpr uint32_t kLensRestApiDefaultPort         = 47382;
static constexpr uint32_t kLensRestApiHardBodyLimitBytes  = 64u * 1024u * 1024u;  // 64 MiB
static constexpr uint8_t  kLensRestApiSchemaVersion       = 1;

} // namespace Engine
} // namespace ExplorerLens
