// AnonymousTelemetry.h — Opt-in Anonymous Telemetry Module
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an opt-in, anonymous instrumentation layer for ExplorerLens.
// Events are batched locally and uploaded via HTTPS only when the user has
// explicitly granted consent.  No PII is collected at any time.
//
// Privacy contract:
//   - Consent flag stored in HKCU\Software\ExplorerLens\Telemetry\Enabled (DWORD)
//   - No file paths, user names, or machine-identifiable data in any payload
//   - Session ID is a random GUID generated at process start (not persisted)
//   - Batch is discarded if upload fails — no retry queue persisted to disk
//
#pragma once

#include <string>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Telemetry event categories.
enum class AnonTelemetryEventType : uint8_t {
    FormatDecoded     = 1,  // A specific format was successfully decoded
    FormatFailed      = 2,  // Decode failed (no PII — format name only)
    GpuBackendUsed    = 3,  // Which GPU backend handled the request
    CacheHit          = 4,  // Cache hit event (no paths)
    CacheMiss         = 5,  // Cache miss event
    CrashReport       = 6,  // Structured crash summary (no stack PIIs)
    SessionStart      = 7,  // Engine session started
    SessionEnd        = 8,  // Engine session ended (includes duration)
    SettingChanged    = 9,  // A setting key was changed (key name only, no value)
    PluginLoaded      = 10, // A plugin was loaded (plugin name + version)
};

// A single anonymous telemetry event payload.
struct AnonTelemetryEvent {
    AnonTelemetryEventType type;
    std::string        key;    // e.g. format name, backend name, setting key
    std::string        value;  // numeric or enum string — never user content
    uint64_t           tsMs;   // millisecond timestamp since epoch
};

// AnonymousTelemetry — Batched opt-in event collector.
//
// Thread-safe.  All methods silently no-op when telemetry is disabled.
class AnonymousTelemetry {
public:
    AnonymousTelemetry() noexcept;
    ~AnonymousTelemetry() noexcept;

    AnonymousTelemetry(const AnonymousTelemetry&)            = delete;
    AnonymousTelemetry& operator=(const AnonymousTelemetry&) = delete;

    // Initialise from registry consent flag.  Must be called before Track().
    void Initialize() noexcept;

    // Explicit consent toggle (also writes HKCU registry flag).
    void SetConsent(bool granted) noexcept;
    bool HasConsent() const noexcept { return m_consent; }

    // Queue an event.  No-ops if !HasConsent().
    void Track(AnonTelemetryEventType type,
               const std::string& key,
               const std::string& value = {}) noexcept;

    // Flush the event batch.  Calls uploadFn with JSON payload.
    // uploadFn receives: (const std::string& jsonPayload) -> bool (true = success)
    void Flush(std::function<bool(const std::string&)> uploadFn) noexcept;

    // Discard all queued events without uploading.
    void Discard() noexcept;

    // Global singleton accessor.  Not initialised until first call.
    static AnonymousTelemetry& Instance() noexcept;

private:
    bool        m_consent { false };
    std::string m_sessionId;

    struct Impl;
    Impl*       m_impl { nullptr };
};

// Convenience macros for zero-overhead tracking when telemetry is disabled.
#define LENS_TELEMETRY_TRACK(type, key, value) \
    ExplorerLens::Engine::AnonymousTelemetry::Instance().Track(type, key, value)

}} // namespace ExplorerLens::Engine
