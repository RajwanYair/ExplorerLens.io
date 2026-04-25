// ============================================================================
// ShellComHostInTestContract.h -- S284 / ROADMAP v6.0 T7 COM integration tests
//
// Phase 3 contract: register `LENSShell.dll` inside a sandboxed test harness
// that hosts the IThumbnailProvider CLSID, invokes the full COM path, and
// validates pixel output via SSIM.  Header-only — declares harness modes,
// register/unregister semantics, and bitmap-validation thresholds.
// Named `ShellComHostInTest*` to dodge existing `COMIntegrationTest` class
// in `Engine/Tests/Integration/` (Sprint 29 artefact).
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ShellComHostInTestMode : uint8_t
{
    IN_PROCESS_REGFREE   = 0,   // manifest-based, preferred for CI
    IN_PROCESS_REGSVR32  = 1,   // requires admin; local only
    OUT_OF_PROCESS_SURROGATE = 2, // dllhost.exe /ProcessID
    MOCK_COM_RUNTIME     = 3,   // unit-test fake
};

enum class ShellComHostInTestStatus : uint8_t
{
    OK                     = 0,
    REGISTRATION_FAILED    = 1,
    COCREATEINSTANCE_FAILED = 2,
    QUERYINTERFACE_FAILED  = 3,
    INITIALIZE_FAILED      = 4,
    GETTHUMBNAIL_FAILED    = 5,
    BITMAP_INVALID         = 6,
    SSIM_BELOW_THRESHOLD   = 7,
    TIMEOUT                = 8,
};

struct ShellComHostInTestPolicy
{
    ShellComHostInTestMode mode            = ShellComHostInTestMode::IN_PROCESS_REGFREE;
    uint32_t               invokeBudgetMs  = 1500;
    uint32_t               cleanupBudgetMs = 500;
    float                  minSsim         = 0.95f;   // vs reference bitmap
    bool                   validateAlphaChannel = true;
    bool                   requireStride4Alignment = true;
    bool                   failOnComLeakedRefs = true;
};

struct ShellComHostInTestProbe
{
    ShellComHostInTestStatus status          = ShellComHostInTestStatus::OK;
    uint32_t                 invokeMs        = 0;
    uint32_t                 bitmapWidth     = 0;
    uint32_t                 bitmapHeight    = 0;
    float                    measuredSsim    = 0.0f;
    uint32_t                 comRefLeakCount = 0;
};

inline constexpr uint32_t kShellComHostInTestHardBudgetMs = 5000;
inline constexpr float    kShellComHostInTestMinAcceptableSsim = 0.90f;
inline constexpr float    kShellComHostInTestDefaultMinSsim    = 0.95f;

static_assert(std::is_trivially_copyable_v<ShellComHostInTestPolicy>,
              "ShellComHostInTestPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ShellComHostInTestProbe>,
              "ShellComHostInTestProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
