// DbusThumbnailerContract.h -- S298 / ROADMAP v6.0 H18 Phase 6
// D-Bus thumbnailer protocol contract for Linux Nautilus / KIO integration.
//
// Adapted from the GNOME Tumbler D-Bus thumbnailer model (H18):
// the Engine operates as a COM surrogate on Windows but adapts to
// D-Bus activation on Linux.
//
// On Linux, ExplorerLens registers as:
//   org.freedesktop.thumbnails.Thumbnailer1 (Tumbler spec)
// Service name: org.ExplorerLens.Thumbnailer
//
// This contract defines the D-Bus interface, MIME type list, and
// URI scheme capabilities as compile-time constants.
//
// Rule: contract header only — no implementation, no platform headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>
#include <cstddef>

namespace ExplorerLens {
namespace Engine {

// ── D-Bus Protocol Mode ───────────────────────────────────────────────────────

enum class DbusThumbnailerMode : uint8_t {
    DISABLED        = 0,  // Windows-only build — D-Bus not available
    TUMBLER_V1      = 1,  // GNOME Tumbler org.freedesktop.thumbnails.Thumbnailer1
    KIO_THUMBNAIL   = 2,  // KDE KIO thumbnail plugin protocol
    MOCK_DBUS       = 3,  // Test harness mode (no D-Bus needed)
};

// ── URI Scheme ────────────────────────────────────────────────────────────────

enum class DbusThumbnailerUriScheme : uint8_t {
    FILE    = 0,  // file:// (local files — primary)
    SFTP    = 1,  // sftp:// (Phase 6 remote thumbnailing)
    TRASH   = 2,  // trash:// (Nautilus trash integration)
};

// ── Service Policy ────────────────────────────────────────────────────────────

struct DbusThumbnailerPolicy {
    DbusThumbnailerMode       mode               = DbusThumbnailerMode::DISABLED;
    DbusThumbnailerUriScheme  primaryScheme      = DbusThumbnailerUriScheme::FILE;
    uint32_t                  maxThumbnailPx     = 256;    // Standard Tumbler size
    uint32_t                  softTimeoutMs      = 500;
    uint32_t                  hardTimeoutMs      = 3000;
    uint8_t                   maxConcurrentJobs  = 4;
    bool                      emitReadySignal    = true;   // D-Bus Ready() signal
    bool                      emitErrorSignal    = true;   // D-Bus Error() signal
};

// ── Probe ─────────────────────────────────────────────────────────────────────

struct DbusThumbnailerProbe {
    DbusThumbnailerMode activeMode      = DbusThumbnailerMode::DISABLED;
    bool                dbusConnected   = false;
    uint32_t            requestsHandled = 0;
    uint32_t            requestsFailed  = 0;
    uint32_t            cacheHits       = 0;
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint32_t kDbusThumbnailerMaxSizePx      = 512u;  // Tumbler max
static constexpr uint32_t kDbusThumbnailerHardTimeoutMs  = 10000u;
static constexpr uint8_t  kDbusThumbnailerSchemaVersion  = 1;
static constexpr const char* kDbusThumbnailerServiceName = "org.ExplorerLens.Thumbnailer";
static constexpr const char* kDbusThumbnailerInterface   = "org.freedesktop.thumbnails.Thumbnailer1";

} // namespace Engine
} // namespace ExplorerLens
