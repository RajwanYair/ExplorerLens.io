// CrashTelemetryConsentContract.h -- S293 / ROADMAP v6.0 H2 Phase 4
// Opt-in crash telemetry consent policy for ExplorerLens.
// Inspired by PowerToys centralized crash telemetry with user opt-in.
//
// Design rules:
//   - Default mode is DISABLED — no crash data sent without explicit consent.
//   - No PII allowed: paths are stripped (TelemetryPrivacyRedactor, S276).
//   - Crash dumps upload to private Azure Blob (opt-in only).
//   - Consent persists in HKCU\Software\ExplorerLens\Telemetry.
//
// Rule: contract header only — no implementation, no Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Consent Level ────────────────────────────────────────────────────────────

enum class CrashTelemetryConsentMode : uint8_t {
    DISABLED        = 0,  // Default: no telemetry sent
    CRASH_ONLY      = 1,  // Send mini-dump on crash only
    CRASH_AND_PERF  = 2,  // Send mini-dump + anonymised perf counters
    FULL            = 3,  // All of above + usage heuristics (enterprise)
};

// ── PII Filtering Level ───────────────────────────────────────────────────────

enum class CrashTelemetryPiiFilter : uint8_t {
    STRICT_MODE = 0,  // Strip paths, hostnames, usernames, EXIF PII (default)
    MODERATE    = 1,  // Strip obvious PII only
    NONE        = 2,  // No filtering (enterprise network-isolated deployments only)
};

// ── Upload Target ─────────────────────────────────────────────────────────────

enum class CrashTelemetryUploadTarget : uint8_t {
    AZURE_BLOB      = 0,  // Private Azure Blob (default)
    LOCAL_ONLY      = 1,  // Write dump to %LOCALAPPDATA%\ExplorerLens\Crashes only
    CUSTOM_ENDPOINT = 2,  // Enterprise custom URL (via Group Policy)
};

// ── Consent Policy ────────────────────────────────────────────────────────────

struct CrashTelemetryConsentPolicy {
    CrashTelemetryConsentMode   mode           = CrashTelemetryConsentMode::DISABLED;
    CrashTelemetryPiiFilter     piiFilter       = CrashTelemetryPiiFilter::STRICT_MODE;
    CrashTelemetryUploadTarget  uploadTarget    = CrashTelemetryUploadTarget::LOCAL_ONLY;
    uint32_t                    maxDumpKb       = 8192;    // 8 MiB default (hard cap via WerCrashReporter)
    uint32_t                    rateLimitPerDay = 3;       // Max crash reports per day
    bool                        requireWifi     = true;    // Do not upload on metered connections
};

// ── Consent Probe (for LENSManager diagnostics) ───────────────────────────────

struct CrashTelemetryConsentProbe {
    CrashTelemetryConsentMode activeMode = CrashTelemetryConsentMode::DISABLED;
    uint32_t crashesRecordedToday        = 0;
    uint32_t crashesUploadedToday        = 0;
    bool     consentDialogShownToUser    = false;
};

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr uint32_t kCrashTelemetryMaxDumpKb         = 32768u;   // 32 MiB hard cap
static constexpr uint32_t kCrashTelemetryHardRateLimitPerDay = 10u;    // Never > 10/day
static constexpr uint8_t  kCrashTelemetrySchemaVersion      = 1;

} // namespace Engine
} // namespace ExplorerLens
