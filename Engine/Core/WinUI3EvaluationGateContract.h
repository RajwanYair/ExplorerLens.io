// ============================================================================
// WinUI3EvaluationGateContract.h -- S285 / ROADMAP v6.0 H20 WinUI 3 gate
//
// Phase 3 gate: evaluate whether LENSManager's WTL UI should be migrated to
// WinUI 3.  Header-only.  Declares the criteria matrix, pass/fail thresholds,
// and decision enum.  This is a *planning* contract, not runtime code.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class WinUI3GateCriterion : uint8_t
{
    COLD_START_MS               = 0,   // ≤ 1500 ms
    WARM_START_MS               = 1,   // ≤ 400 ms
    INSTALLED_SIZE_MB           = 2,   // ≤ 15 MB deltas vs WTL
    WINDOWS_APP_SDK_RUNTIME_OK  = 3,   // bootstrap acceptable?
    MICA_ACRYLIC_SUPPORT        = 4,
    HIGH_DPI_OK                 = 5,
    COM_BRIDGE_COMPATIBLE       = 6,   // WinUI 3 can host IPreviewHandler UI?
    ACCESSIBILITY_NVDA_OK       = 7,
    PACKAGING_MSIX_ONLY         = 8,   // unpackaged app allowed?
    XAML_HOT_RELOAD_DEV_OK      = 9,
};

enum class WinUI3GateDecision : uint8_t
{
    PENDING              = 0,
    MIGRATE              = 1,
    KEEP_WTL             = 2,
    HYBRID_MICA_ONLY     = 3,   // WTL + DwmExtendFrameIntoClientArea
    REVISIT_NEXT_PHASE   = 4,
};

struct WinUI3GateResultRow
{
    WinUI3GateCriterion criterion;
    bool                passed;
    uint32_t            measuredValue;   // context-dependent
};

struct WinUI3GatePolicy
{
    uint32_t coldStartBudgetMs = 1500;
    uint32_t warmStartBudgetMs = 400;
    uint32_t maxInstalledSizeDeltaMb = 15;
    bool     requireUnpackagedSupport = true;
    bool     requireComBridge = true;
    bool     requireAccessibilityNvda = true;
};

inline constexpr size_t kWinUI3GateCriterionCount = 10;

static_assert(std::is_trivially_copyable_v<WinUI3GatePolicy>,
              "WinUI3GatePolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<WinUI3GateResultRow>,
              "WinUI3GateResultRow must be trivially copyable");

} // namespace ExplorerLens::Engine
