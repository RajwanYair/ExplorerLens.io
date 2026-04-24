//==============================================================================
// ExplorerLens Engine — CET / mitigation policy manifest (Sprint S246)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A13 / ADR — /CETCOMPAT linker flag (Phase 2).
//==============================================================================
//
// Source-of-truth for the process-mitigation flags LENSShell.dll and
// LENSManager.exe are built with. Keeping this in a header lets tests verify
// the advertised policy matches what the linker produced (PE header
// inspection). Actual flag application happens in CMake/MSBuild; this file
// documents and cross-checks.
//==============================================================================
#pragma once

#include <cstdint>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

/// <summary>Mitigation feature bitmask.</summary>
enum class MitigationBits : std::uint32_t
{
    NONE              = 0,
    DYNAMIC_BASE      = 1u << 0,   // /DYNAMICBASE ASLR
    HIGH_ENTROPY_VA   = 1u << 1,   // /HIGHENTROPYVA (x64 ASLR)
    NX_COMPAT         = 1u << 2,   // /NXCOMPAT DEP
    GUARD_CF          = 1u << 3,   // /guard:cf Control-Flow Guard
    GUARD_EH          = 1u << 4,   // /guard:ehcont EH continuation
    CET_COMPAT        = 1u << 5,   // /CETCOMPAT shadow-stack
    GS_COOKIE         = 1u << 6,   // /GS stack buffer overrun
    SDL               = 1u << 7,   // /sdl security-dev-lifecycle
    RETPOLINE         = 1u << 8,   // /Qspectre-load
    SAFE_SEH          = 1u << 9    // x86-only — unused in x64 builds
};

constexpr MitigationBits operator|(MitigationBits a, MitigationBits b) noexcept
{
    return static_cast<MitigationBits>(
        static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

constexpr bool HasBit(MitigationBits mask, MitigationBits bit) noexcept
{
    return (static_cast<std::uint32_t>(mask) & static_cast<std::uint32_t>(bit)) != 0;
}

/// <summary>Baseline mitigation set v39.2.0 ships with (Phase 1 — no CET yet).</summary>
inline constexpr MitigationBits kMitigationsPhase1 =
      MitigationBits::DYNAMIC_BASE
    | MitigationBits::HIGH_ENTROPY_VA
    | MitigationBits::NX_COMPAT
    | MitigationBits::GUARD_CF
    | MitigationBits::GS_COOKIE
    | MitigationBits::SDL;

/// <summary>Phase 2 target — adds CETCOMPAT + ehcont + retpoline.</summary>
inline constexpr MitigationBits kMitigationsPhase2 =
      kMitigationsPhase1
    | MitigationBits::CET_COMPAT
    | MitigationBits::GUARD_EH
    | MitigationBits::RETPOLINE;

/// <summary>Per-binary policy record — used by CI asserts.</summary>
struct BinaryMitigationPolicy
{
    std::string_view  binaryName;
    MitigationBits    requiredBits = MitigationBits::NONE;
    MitigationBits    desiredBits  = MitigationBits::NONE;
};

inline constexpr BinaryMitigationPolicy kLENSShellPolicy {
    "LENSShell.dll", kMitigationsPhase1, kMitigationsPhase2
};

inline constexpr BinaryMitigationPolicy kLENSManagerPolicy {
    "LENSManager.exe", kMitigationsPhase1, kMitigationsPhase2
};

} // namespace Engine
} // namespace ExplorerLens
