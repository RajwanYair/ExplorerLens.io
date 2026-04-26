// EngineDllAbiContract.h -- S296 / ROADMAP v6.0 B1 Phase 4
// Engine.lib → Engine.dll C-ABI export policy.
//
// Phase 4 goal: migrate ExplorerLensEngine from a static library (.lib)
// to a versioned DLL with a stable C-ABI. This contract defines the
// export policy, versioning scheme, and backward-compatibility rules.
//
// C-ABI rules (enforced by static_assert at DLL boundary):
//   - No STL containers across DLL boundary (ADR-021)
//   - No C++ exceptions across boundary (ADR-003)
//   - All public functions use __cdecl calling convention
//   - Version negotiation via LensEngineQueryVersion()
//
// Rule: contract header only — no implementation, no Win32 headers.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── ABI Version ──────────────────────────────────────────────────────────────
// Increment MAJOR on any breaking change. MINOR for additions.

static constexpr uint16_t kEngineDllAbiMajorVersion = 1;
static constexpr uint16_t kEngineDllAbiMinorVersion = 0;
static constexpr uint32_t kEngineDllAbiCurrentVersion =
    (static_cast<uint32_t>(kEngineDllAbiMajorVersion) << 16) |
     static_cast<uint32_t>(kEngineDllAbiMinorVersion);

// ── Export Stability ──────────────────────────────────────────────────────────

enum class EngineDllAbiStability : uint8_t {
    STABLE      = 0,  // Frozen — guaranteed across major versions
    PROVISIONAL = 1,  // May change within Phase 4 iteration
    INTERNAL    = 2,  // Not exported — internal linkage only
};

// ── Export Entry ─────────────────────────────────────────────────────────────

struct EngineDllAbiExportEntry {
    const char*          symbolName;
    EngineDllAbiStability stability;
    uint32_t             introducedVersion;  // kEngineDllAbiCurrentVersion format
    const char*          signature;          // Informational C declaration
};

// ── Core Export Table (Phase 4 baseline) ─────────────────────────────────────
// Exported symbols for the Engine.dll C-ABI.

static constexpr size_t kEngineDllAbiExportCount = 8;

static constexpr EngineDllAbiExportEntry kEngineDllAbiExports[kEngineDllAbiExportCount] = {
    { "LensEngineQueryVersion",     EngineDllAbiStability::STABLE,      0x00010000, "uint32_t __cdecl LensEngineQueryVersion(void)" },
    { "LensEngineInitialize",       EngineDllAbiStability::STABLE,      0x00010000, "int __cdecl LensEngineInitialize(const LensEngineConfig*)" },
    { "LensEngineShutdown",         EngineDllAbiStability::STABLE,      0x00010000, "void __cdecl LensEngineShutdown(void)" },
    { "LensEngineDecode",           EngineDllAbiStability::STABLE,      0x00010000, "int __cdecl LensEngineDecode(const LensDecodeRequest*, LensDecodeResult*)" },
    { "LensEngineDecodeFree",       EngineDllAbiStability::STABLE,      0x00010000, "void __cdecl LensEngineDecodeFree(LensDecodeResult*)" },
    { "LensEngineCacheStats",       EngineDllAbiStability::PROVISIONAL, 0x00010000, "int __cdecl LensEngineCacheStats(LensCacheStats*)" },
    { "LensEngineCachePurge",       EngineDllAbiStability::PROVISIONAL, 0x00010000, "int __cdecl LensEngineCachePurge(const char* formatGlob)" },
    { "LensEngineFormatList",       EngineDllAbiStability::PROVISIONAL, 0x00010000, "int __cdecl LensEngineFormatList(LensFormatEntry* buf, size_t bufLen)" },
};

static_assert(kEngineDllAbiExportCount == 8,
    "kEngineDllAbiExportCount must equal 8 — update when adding exports");

// ── Compat Policy ─────────────────────────────────────────────────────────────

struct EngineDllAbiPolicy {
    uint32_t minClientVersion = kEngineDllAbiCurrentVersion;
    bool     enforceStableOnly = true;   // Refuse PROVISIONAL exports to unknown clients
    bool     allowDowngrade    = false;  // Never load older Engine.dll than required
};

} // namespace Engine
} // namespace ExplorerLens
