// ============================================================================
// ShellContextMenuContract.h -- S263 / ROADMAP v6.0 F6 IContextMenu
//
// Phase 3 IContextMenu contract.  Header-only.  Declares the verbs that
// LENSShell registers on right-click for its file types, so LENSManager,
// telemetry, and localisation all agree on the command set.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ShellMenuVerbId : uint16_t
{
    NONE                 = 0,
    OPEN_IN_LENSMANAGER  = 1,
    COPY_THUMBNAIL       = 2,
    COPY_METADATA        = 3,
    COPY_SHA256          = 4,
    SHOW_DECODER_INFO    = 5,
    REGENERATE_THUMBNAIL = 6,
    EXCLUDE_FROM_CACHE   = 7,
    REPORT_DECODER_BUG   = 8,
};

enum class ShellMenuScope : uint8_t
{
    FILE_ONLY            = 0,
    FOLDER_ONLY          = 1,
    FILE_OR_FOLDER       = 2,
};

struct ShellMenuCommand
{
    ShellMenuVerbId verb          = ShellMenuVerbId::NONE;
    ShellMenuScope  scope         = ShellMenuScope::FILE_ONLY;
    const char*     canonicalName = nullptr;   // "explorerlens.copyThumbnail"
    const char*     displayNameEn = nullptr;   // "Copy thumbnail image"
    bool            requiresAdmin = false;
};

inline constexpr ShellMenuCommand kShellMenuCommands[] = {
    { ShellMenuVerbId::OPEN_IN_LENSMANAGER,  ShellMenuScope::FILE_OR_FOLDER, "explorerlens.openInManager",    "Open in LENSManager",      false },
    { ShellMenuVerbId::COPY_THUMBNAIL,       ShellMenuScope::FILE_ONLY,      "explorerlens.copyThumbnail",    "Copy thumbnail image",     false },
    { ShellMenuVerbId::COPY_METADATA,        ShellMenuScope::FILE_ONLY,      "explorerlens.copyMetadata",     "Copy metadata as JSON",    false },
    { ShellMenuVerbId::COPY_SHA256,          ShellMenuScope::FILE_ONLY,      "explorerlens.copySha256",       "Copy SHA-256 hash",        false },
    { ShellMenuVerbId::SHOW_DECODER_INFO,    ShellMenuScope::FILE_ONLY,      "explorerlens.showDecoderInfo",  "Show decoder info",        false },
    { ShellMenuVerbId::REGENERATE_THUMBNAIL, ShellMenuScope::FILE_OR_FOLDER, "explorerlens.regenThumbnail",   "Regenerate thumbnail",     false },
    { ShellMenuVerbId::EXCLUDE_FROM_CACHE,   ShellMenuScope::FILE_OR_FOLDER, "explorerlens.excludeCache",     "Exclude from thumbnail cache", true },
    { ShellMenuVerbId::REPORT_DECODER_BUG,   ShellMenuScope::FILE_ONLY,      "explorerlens.reportBug",        "Report decoder bug...",    false },
};

inline constexpr size_t kShellMenuCommandsCount =
    sizeof(kShellMenuCommands) / sizeof(kShellMenuCommands[0]);

inline constexpr uint32_t kShellMenuMaxSelection = 1024;   // files per context-menu invoke
inline constexpr uint32_t kShellMenuInvokeBudgetMs = 16;   // IContextMenu::InvokeCommand budget

static_assert(kShellMenuCommandsCount == 8,
              "ShellMenuCommand table must declare 8 verbs");
static_assert(std::is_trivially_copyable_v<ShellMenuCommand>,
              "ShellMenuCommand must be trivially copyable");

} // namespace ExplorerLens::Engine
