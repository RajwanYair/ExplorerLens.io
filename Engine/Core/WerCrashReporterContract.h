// ============================================================================
// WerCrashReporterContract.h -- S288 / ROADMAP v6.0 O3 WER integration
//
// Phase 4 contract: register a custom Windows Error Reporting handler so
// `LENSShell.dll` crashes under `explorer.exe` produce actionable minidumps
// and ETW breadcrumbs.  Header-only — declares submission modes, redaction
// scope, and opt-in policy.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class WerSubmissionMode : uint8_t
{
    DISABLED               = 0,   // default until user opts in
    LOCAL_ONLY             = 1,   // dump to %LOCALAPPDATA%
    MICROSOFT_ONLY         = 2,   // Win error reporting servers
    VENDOR_ENDPOINT        = 3,   // explorerlens.io/wer
    VENDOR_PLUS_MICROSOFT  = 4,
};

enum class WerMinidumpTier : uint8_t
{
    NONE           = 0,
    MINI_TRIAGE    = 1,   // 64 KiB, stacks only
    MINI_WITH_HEAP = 2,
    FULL_HEAP      = 3,   // developer-only
};

enum class WerCrashCategory : uint8_t
{
    UNKNOWN               = 0,
    DECODER_ACCESS_VIOLATION = 1,
    DECODER_STACK_OVERFLOW   = 2,
    COM_FAILURE              = 3,
    GPU_DEVICE_LOST          = 4,
    OUT_OF_MEMORY            = 5,
    PLUGIN_CRASH             = 6,
    WATCHDOG_TERMINATE       = 7,
};

struct WerCrashReporterPolicy
{
    WerSubmissionMode submissionMode      = WerSubmissionMode::DISABLED;
    WerMinidumpTier   defaultTier         = WerMinidumpTier::MINI_TRIAGE;
    bool              redactFilePaths     = true;
    bool              redactDecoderPayload = true;
    bool              requireExplicitOptIn = true;
    bool              stripUsernameFromPaths = true;
    uint32_t          maxDumpBytes        = 4u * 1024u * 1024u;   // 4 MiB
    uint32_t          submitBudgetMs      = 5000;
    uint32_t          rateLimitPerHour    = 6;
};

struct WerCrashReporterProbe
{
    WerCrashCategory  category        = WerCrashCategory::UNKNOWN;
    WerMinidumpTier   tierUsed        = WerMinidumpTier::NONE;
    uint32_t          dumpBytes       = 0;
    bool              submitted       = false;
    bool              userOptedIn     = false;
};

inline constexpr uint32_t kWerCrashReporterHardMaxDumpBytes = 32u * 1024u * 1024u;
inline constexpr uint32_t kWerCrashReporterHardRateLimitPerHour = 60;

static_assert(std::is_trivially_copyable_v<WerCrashReporterPolicy>,
              "WerCrashReporterPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<WerCrashReporterProbe>,
              "WerCrashReporterProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
