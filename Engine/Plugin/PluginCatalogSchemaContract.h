// PluginCatalogSchemaContract.h -- S299 / ROADMAP v6.0 Phase 7
// Third-party plugin catalog schema for lens-plugins.io discovery.
//
// Defines the compile-time catalog entry structure for the ExplorerLens
// plugin marketplace. Third-party decoder plugins are discovered via:
//   1. Local scan: %PROGRAMDATA%\ExplorerLens\Plugins\
//   2. Remote catalog: https://lens-plugins.io/catalog/v1/index.json
//      (Phase 7 — catalog server not yet operational)
//
// Each catalog entry carries enough metadata to:
//   - Validate plugin identity before download (hash-pinned)
//   - Enforce trust chain (PluginTrustChainValidatorV1, S268)
//   - Display in LENSManager plugin browser panel
//
// Rule: contract header only — no implementation, no Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>
#include <cstddef>

namespace ExplorerLens {
namespace Engine {

// ── Catalog Entry Status ──────────────────────────────────────────────────────

enum class PluginCatalogStatus : uint8_t {
    AVAILABLE    = 0,  // Available for download
    INSTALLED    = 1,  // Installed locally
    UPDATE_AVAIL = 2,  // Newer version in catalog
    DEPRECATED   = 3,  // Replaced by built-in or another plugin
    REVOKED      = 4,  // Trust chain revoked — do not load
};

// ── License Category ──────────────────────────────────────────────────────────

enum class PluginCatalogLicense : uint8_t {
    MIT          = 0,
    BSD_2        = 1,
    BSD_3        = 2,
    APACHE2      = 3,
    GPL2         = 4,
    LGPL3        = 5,
    COMMERCIAL   = 6,
    OTHER        = 7,
};

// ── Catalog Entry ─────────────────────────────────────────────────────────────

struct PluginCatalogEntry {
    const char*           pluginId;          // "com.vendor.MyDecoder"
    const char*           displayName;       // Human-readable
    const char*           version;           // SemVer string "1.2.3"
    const char*           sha256Hex;         // SHA-256 of the .lens bundle
    const char*           downloadUrl;       // HTTPS URL (Phase 7)
    PluginCatalogStatus   status;
    PluginCatalogLicense  license;
    uint32_t              installedVersion;  // Packed uint32 for comparison
    uint32_t              catalogVersion;
    bool                  requiresElevation; // COM registration needs admin
};

// ── Catalog Policy ────────────────────────────────────────────────────────────

struct PluginCatalogPolicy {
    bool     allowRemoteFetch   = false;  // Default: local scan only until Phase 7
    bool     requireSignedHash  = true;   // SHA-256 must match before install
    bool     enforceTrustChain  = true;   // Runs PluginTrustChainValidatorV1
    uint32_t fetchTimeoutMs     = 5000;   // Remote catalog fetch timeout
    uint32_t maxCatalogEntries  = 1024;
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint8_t  kPluginCatalogSchemaVersion    = 1;
static constexpr uint32_t kPluginCatalogMaxEntries       = 1024u;
static constexpr uint32_t kPluginCatalogFetchHardMs      = 15000u;
static constexpr const char* kPluginCatalogRemoteBaseUrl = "https://lens-plugins.io/catalog/v1/";

} // namespace Engine
} // namespace ExplorerLens
