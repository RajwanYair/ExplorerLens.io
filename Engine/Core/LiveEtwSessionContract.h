// ============================================================================
// LiveEtwSessionContract.h -- S283 / ROADMAP v6.0 O5 LENSManager live ETW
//
// Phase 3 contract: LENSManager exposes a live ETW session pane that tails
// the `ExplorerLens-Engine` provider in real time.  Header-only.  Declares
// the event filter set, ring-buffer sizing, and UI-backpressure policy.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class LiveEtwSessionState : uint8_t
{
    STOPPED         = 0,
    STARTING        = 1,
    RUNNING         = 2,
    PAUSED          = 3,
    DRAINING        = 4,
    STOPPING        = 5,
    FAILED_STATE    = 6,   // renamed vs winerror.h FAILED() macro
};

enum class LiveEtwEventLevel : uint8_t
{
    CRITICAL    = 1,
    ERROR_LEVEL = 2,
    WARNING     = 3,
    INFO        = 4,
    VERBOSE     = 5,
};

enum class LiveEtwKeyword : uint8_t
{
    DECODE_PIPELINE  = 0,
    CACHE            = 1,
    GPU              = 2,
    PLUGIN           = 3,
    SECURITY         = 4,
    PERF_BUDGET      = 5,
    SHELL            = 6,
    TELEMETRY        = 7,
};

struct LiveEtwSessionPolicy
{
    LiveEtwEventLevel minLevel             = LiveEtwEventLevel::INFO;
    uint32_t          keywordMask          = 0xFF;    // all 8 keywords
    uint32_t          ringBufferEntries    = 4096;
    uint32_t          uiFlushIntervalMs    = 100;
    uint32_t          maxEventsPerSecond   = 2000;    // backpressure drop
    bool              dropOldestOnFull     = true;
    bool              pauseWhenWindowHidden = true;
    bool              writeRotatingEtlFile = false;
};

struct LiveEtwSessionProbe
{
    LiveEtwSessionState state              = LiveEtwSessionState::STOPPED;
    uint32_t            eventsEmitted      = 0;
    uint32_t            eventsDropped      = 0;
    uint32_t            eventsPerSecond    = 0;
    uint32_t            ringUtilizationPct = 0;
};

inline constexpr uint32_t kLiveEtwSessionMaxRingEntries = 65536;
inline constexpr uint32_t kLiveEtwSessionDefaultRingEntries = 4096;
inline constexpr uint32_t kLiveEtwSessionMaxEventsPerSecond = 10000;

static_assert(std::is_trivially_copyable_v<LiveEtwSessionPolicy>,
              "LiveEtwSessionPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LiveEtwSessionProbe>,
              "LiveEtwSessionProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
