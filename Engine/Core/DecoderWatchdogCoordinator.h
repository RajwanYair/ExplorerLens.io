// ============================================================================
// DecoderWatchdogCoordinator.h -- S275 / ROADMAP v6.0 H11 decode timeouts
//
// Phase 3 watchdog contract.  Oversees per-decoder timeout enforcement so a
// misbehaving decoder cannot stall the shell thumbnail pump.  Integrates
// with `CancellationToken` + per-decoder budget tables.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class DecoderWatchdogAction : uint8_t
{
    NONE                 = 0,
    SIGNAL_CANCEL        = 1,   // polite co-op cancel
    SOFT_ABORT           = 2,   // throw from decoder
    HARD_TERMINATE       = 3,   // tear down worker thread
    QUARANTINE_DECODER   = 4,   // disable for this session
};

enum class DecoderWatchdogPhase : uint8_t
{
    PARSE              = 0,
    DECODE             = 1,
    COLOR_CONVERT      = 2,
    RESIZE             = 3,
    ENCODE_PREVIEW     = 4,
    WRITE_CACHE        = 5,
};

enum class DecoderWatchdogVerdict : uint8_t
{
    OK                 = 0,
    WARNING_SLOW       = 1,   // > soft threshold
    TIMEOUT_CANCELLED  = 2,
    TIMEOUT_TERMINATED = 3,
    QUARANTINED        = 4,
};

struct DecoderWatchdogPolicy
{
    uint32_t softBudgetMs      = 100;   // log warning at this point
    uint32_t hardBudgetMs      = 500;   // cancel
    uint32_t terminateBudgetMs = 2000;  // hard-kill thread
    uint32_t quarantineAfterFails = 3;  // N hard-kills -> disable for session
    bool     emitEtwEvents     = true;
};

struct DecoderWatchdogReport
{
    DecoderWatchdogVerdict verdict      = DecoderWatchdogVerdict::OK;
    DecoderWatchdogAction  actionTaken  = DecoderWatchdogAction::NONE;
    DecoderWatchdogPhase   stalledPhase = DecoderWatchdogPhase::DECODE;
    uint32_t               elapsedMs    = 0;
    uint32_t               previousFails = 0;
};

inline constexpr uint32_t kDecoderWatchdogDefaultSoftBudgetMs = 100;
inline constexpr uint32_t kDecoderWatchdogDefaultHardBudgetMs = 500;
inline constexpr uint32_t kDecoderWatchdogMaxBudgetMs         = 5000;
inline constexpr uint32_t kDecoderWatchdogQuarantineThreshold = 3;

static_assert(std::is_trivially_copyable_v<DecoderWatchdogPolicy>,
              "DecoderWatchdogPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<DecoderWatchdogReport>,
              "DecoderWatchdogReport must be trivially copyable");
static_assert(kDecoderWatchdogDefaultSoftBudgetMs <
              kDecoderWatchdogDefaultHardBudgetMs,
              "watchdog soft < hard budget");

} // namespace ExplorerLens::Engine
