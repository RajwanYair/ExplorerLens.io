// ============================================================================
// SpacebarPreviewShortcutContract.h -- S281 / ROADMAP v6.0 H3 QuickLook shortcut
//
// Phase 3 contract: Space-bar press in Explorer selects file → invoke
// IPreviewHandler with full-resolution preview, ESC closes, arrow keys
// advance.  Header-only.  Declares hotkey routing table, preview-window
// state machine, and close semantics.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class SpacebarPreviewTrigger : uint8_t
{
    SPACE_PRESSED      = 0,
    ALT_SPACE_PRESSED  = 1,   // future: side-by-side
    CONTEXT_MENU_VERB  = 2,
    COMMAND_LINE       = 3,
};

enum class SpacebarPreviewWindowState : uint8_t
{
    HIDDEN            = 0,
    OPENING           = 1,
    ANIMATING_IN      = 2,
    READY             = 3,
    ANIMATING_OUT     = 4,
    CLOSED            = 5,
};

enum class SpacebarPreviewCloseReason : uint8_t
{
    ESC_PRESSED            = 0,
    SPACE_PRESSED_AGAIN    = 1,
    FOCUS_LOST             = 2,
    SELECTION_CHANGED      = 3,
    EXPLORER_CLOSING       = 4,
    DECODER_FAILED         = 5,
    USER_CANCELLED         = 6,
};

struct SpacebarPreviewPolicy
{
    bool     enableGlobalHotkey    = true;
    bool     closeOnFocusLoss      = true;
    bool     dimBackground         = true;   // macOS QL-style
    bool     advanceOnArrowKeys    = true;
    uint32_t animationDurationMs   = 150;
    uint32_t readyBudgetMs         = 250;    // time until first paint
    uint32_t maxImageBytesDisplayed = 64u * 1024u * 1024u;
};

struct SpacebarPreviewProbe
{
    SpacebarPreviewWindowState state           = SpacebarPreviewWindowState::HIDDEN;
    SpacebarPreviewTrigger     lastTrigger     = SpacebarPreviewTrigger::SPACE_PRESSED;
    SpacebarPreviewCloseReason lastCloseReason = SpacebarPreviewCloseReason::ESC_PRESSED;
    uint32_t                   openToReadyMs   = 0;
    uint32_t                   framesRendered  = 0;
};

inline constexpr uint32_t kSpacebarPreviewDefaultBudgetMs = 250;
inline constexpr uint32_t kSpacebarPreviewHardBudgetMs    = 1000;
inline constexpr uint32_t kSpacebarPreviewAnimationMs     = 150;

static_assert(std::is_trivially_copyable_v<SpacebarPreviewPolicy>,
              "SpacebarPreviewPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<SpacebarPreviewProbe>,
              "SpacebarPreviewProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
