// Engine/Core/ProcessIsolationConfig.h
// ExplorerLens Engine — S382 (Phase 4, Sprint 2)
//
// Purpose:
//   Out-of-process decoder isolation configuration (ROADMAP H36).
//   Phase 4 exit criterion: "Process isolation option for decoders (H36)"
//
//   When enabled, decode operations run in a separate low-integrity process
//   (lens-decode-host.exe) via COM out-of-proc activation. This:
//   - Prevents decoder crashes from crashing Explorer
//   - Allows per-format sandboxing (JPEG goes to process A, PDF to process B)
//   - Enables crash recovery without shell restart
//
//   IPC options:
//   1. COM out-of-proc (default) — leverages existing COM infrastructure
//   2. Shared memory (H45) — SharedMemoryChannel handles the bitmap transfer
//   3. Named pipe — for older OS versions
//
//   ProcessIsolationBroker.h (existing, S396) handles the runtime brokering.
//   This header defines the static configuration that feeds the broker.

#pragma once
#ifndef EXPLORERLENS_ENGINE_PROCESSISOLATIONCONFIG_H
#define EXPLORERLENS_ENGINE_PROCESSISOLATIONCONFIG_H

#include <cstdint>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── IPC transport ───────────────────────────────────────────────────────────

enum class IsolationIpcTransport : uint8_t {
    COM_OUT_OF_PROC  = 0,   // preferred: COM surrogate process
    SHARED_MEMORY    = 1,   // H45: CreateFileMapping bitmap handoff
    NAMED_PIPE       = 2,   // fallback for older OS
    IN_PROCESS       = 3,   // disabled: no isolation (default when feature off)
};

// ─── Isolation scope ─────────────────────────────────────────────────────────

enum class IsolationScope : uint8_t {
    NONE             = 0,   // all decoders in-process
    HIGH_RISK_ONLY   = 1,   // PDF, archive, video out-of-proc; images in-proc
    ALL_DECODERS     = 2,   // every decode out-of-proc
    PER_FORMAT       = 3,   // configurable per format family
};

// ─── Process integrity level ─────────────────────────────────────────────────

enum class IsolationIntegrity : uint8_t {
    MEDIUM       = 0,   // same as Explorer
    LOW          = 1,   // recommended: isolated with low-integrity token
    APP_CONTAINER= 2,   // Phase 5: AppContainer sandbox
};

// ─── Per-format isolation override ───────────────────────────────────────────

struct FormatIsolationOverride final {
    const char*       extension       = nullptr;   // e.g. "pdf", "zip"
    bool              isolate         = true;
    IsolationIntegrity integrity      = IsolationIntegrity::LOW;

    bool IsValid() const noexcept { return extension != nullptr && extension[0] != '\0'; }
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct ProcessIsolationConfig final {
    bool                  enabled           = false;  // opt-in via registry/settings
    IsolationScope        scope             = IsolationScope::HIGH_RISK_ONLY;
    IsolationIpcTransport transport         = IsolationIpcTransport::COM_OUT_OF_PROC;
    IsolationIntegrity    integrity         = IsolationIntegrity::LOW;
    uint32_t              hostTimeoutMs     = 5000u;  // kill host if decode > 5s
    uint32_t              maxHostProcesses  = 4u;     // cap parallel host procs
    bool                  restartOnCrash    = true;

    static constexpr ProcessIsolationConfig Disabled() noexcept {
        return ProcessIsolationConfig{};  // enabled=false
    }

    static constexpr ProcessIsolationConfig Phase4Default() noexcept {
        ProcessIsolationConfig c{};
        c.enabled       = true;
        c.scope         = IsolationScope::HIGH_RISK_ONLY;
        c.transport     = IsolationIpcTransport::COM_OUT_OF_PROC;
        c.integrity     = IsolationIntegrity::LOW;
        c.hostTimeoutMs = 5000u;
        return c;
    }

    static constexpr ProcessIsolationConfig AllDecoders() noexcept {
        ProcessIsolationConfig c{};
        c.enabled    = true;
        c.scope      = IsolationScope::ALL_DECODERS;
        c.transport  = IsolationIpcTransport::SHARED_MEMORY;
        c.integrity  = IsolationIntegrity::LOW;
        return c;
    }
};

// ─── Runtime status ──────────────────────────────────────────────────────────

struct ProcessIsolationStatus final {
    bool     enabled           = false;
    uint32_t activeHostCount   = 0;
    uint32_t crashesTotal      = 0;
    uint32_t restartsTotal     = 0;

    bool IsHealthy() const noexcept { return enabled && crashesTotal == 0; }
};

// ─── Controller ──────────────────────────────────────────────────────────────

class ProcessIsolationController final {
public:
    ProcessIsolationController() = default;
    ~ProcessIsolationController() = default;

    ProcessIsolationController(const ProcessIsolationController&) = delete;
    ProcessIsolationController& operator=(const ProcessIsolationController&) = delete;

    static ProcessIsolationController& Global() noexcept {
        static ProcessIsolationController s_instance;
        return s_instance;
    }

    void Configure(const ProcessIsolationConfig& config) noexcept { m_config = config; }

    // Check if a given extension should be decoded out-of-process
    bool ShouldIsolate(const char* extension) const noexcept;

    // Get isolation scope for a format family index
    bool IsFormatIsolated(uint8_t familyIdx) const noexcept;

    ProcessIsolationStatus QueryStatus() const noexcept;

    bool IsEnabled() const noexcept { return m_config.enabled; }
    const ProcessIsolationConfig& Config() const noexcept { return m_config; }

private:
    ProcessIsolationConfig m_config{};
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline bool ProcessIsolationController::ShouldIsolate(const char* extension) const noexcept {
    if (!m_config.enabled) return false;
    if (!extension)        return false;
    if (m_config.scope == IsolationScope::ALL_DECODERS) return true;
    if (m_config.scope == IsolationScope::NONE) return false;
    // HIGH_RISK_ONLY: PDF, archives, video
    std::string_view ext{extension};
    return ext == "pdf" || ext == "zip" || ext == "7z" || ext == "rar" ||
           ext == "mp4" || ext == "mkv" || ext == "mov";
}

inline bool ProcessIsolationController::IsFormatIsolated(uint8_t familyIdx) const noexcept {
    if (!m_config.enabled) return false;
    if (m_config.scope == IsolationScope::ALL_DECODERS) return true;
    // familyIdx 3=DOCUMENT, 4=MEDIA are high-risk
    return m_config.scope == IsolationScope::HIGH_RISK_ONLY && (familyIdx == 3 || familyIdx == 4);
}

inline ProcessIsolationStatus ProcessIsolationController::QueryStatus() const noexcept {
    ProcessIsolationStatus s{};
    s.enabled = m_config.enabled;
    return s;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kIsolationDefaultTimeoutMs  = 5000u;
static constexpr uint32_t kIsolationMaxHostProcesses  = 8u;
static constexpr uint32_t kIsolationRestartMaxCount   = 3u;   // give up after 3 restarts

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_PROCESSISOLATIONCONFIG_H
