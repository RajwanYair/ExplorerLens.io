// ============================================================================
// ShellPreviewHandlerContract.h -- S261 / ROADMAP v6.0 F6
//
// Phase 3 IPreviewHandler contract.  Header-only.  Locks the shape of the
// in-pane full-resolution preview (Space-bar in Explorer) before the COM
// class `CLENSPreviewHandler` is authored in LENSShell.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ShellPreviewStatus : uint8_t
{
    OK                   = 0,
    INIT_FAILED          = 1,
    FILE_NOT_FOUND       = 2,
    FORMAT_UNSUPPORTED   = 3,
    BUDGET_EXCEEDED      = 4,
    CANCELLED            = 5,
    HOST_CLOSED_PANE     = 6,
    OUT_OF_MEMORY        = 7,
};

enum class ShellPreviewRenderMode : uint8_t
{
    NATIVE_BITMAP        = 0,   // GDI HBITMAP blit
    DIRECT2D_SURFACE     = 1,   // D2D render target
    DCOMPOSITION_VISUAL  = 2,   // DComp visual (Win10+ panes)
};

struct ShellPreviewRequest
{
    uint32_t  paneWidth           = 0;
    uint32_t  paneHeight           = 0;
    uint32_t  dpi                  = 96;
    ShellPreviewRenderMode mode    = ShellPreviewRenderMode::NATIVE_BITMAP;
    bool      allowAnimation       = false;   // animated WebP/GIF/APNG
    bool      fitToPane            = true;
};

struct ShellPreviewResult
{
    ShellPreviewStatus status        = ShellPreviewStatus::OK;
    uint32_t           emittedWidth  = 0;
    uint32_t           emittedHeight = 0;
    uint32_t           latencyMs     = 0;
    bool               isAnimated    = false;
};

// Explorer gives preview handlers a larger budget than thumbnails (500 ms).
inline constexpr uint32_t kShellPreviewDefaultBudgetMs = 250;
inline constexpr uint32_t kShellPreviewMaxBudgetMs     = 500;
inline constexpr uint32_t kShellPreviewMaxPaneSide     = 8192;

static_assert(std::is_trivially_copyable_v<ShellPreviewRequest>,
              "ShellPreviewRequest must be trivially copyable");
static_assert(std::is_trivially_copyable_v<ShellPreviewResult>,
              "ShellPreviewResult must be trivially copyable");
static_assert(kShellPreviewDefaultBudgetMs < kShellPreviewMaxBudgetMs,
              "preview default budget < hard max");

} // namespace ExplorerLens::Engine
