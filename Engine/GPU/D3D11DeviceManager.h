// Engine/GPU/D3D11DeviceManager.h
// ExplorerLens — D3D11 device initialisation contract (ROADMAP v7.0 Phase 2)
// Sprint S311.
//
// Purpose:
//   Centralises Direct3D 11 device creation and lifetime management for the
//   ExplorerLens GPU decode/blit pipeline.  All GPU headers that need a
//   device obtain it from D3D11DeviceManager::GetDevice() — never create
//   their own device.
//
//   Phase 2 implementation plan:
//     1. Call D3D11CreateDevice() with feature levels FL_11_0 → FL_9_1.
//     2. On first success, cache ID3D11Device + ID3D11DeviceContext.
//     3. On TDR / device loss (DXGI_ERROR_DEVICE_REMOVED), call Reset().
//     4. Expose IsReady() so the blit pipeline (S317) can gate GPU operations.
//
// Contract rule: header-only, no platform headers, no COM pointers.
// Actual COM types live in D3D11DeviceManager.cpp (Phase 2 implementation file).
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_D3D11_DEVICE_MANAGER_H
#define EXPLORERLENS_ENGINE_D3D11_DEVICE_MANAGER_H

#include <cstdint>
#include <cstring>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// D3D11FeatureLevel — subset of D3D_FEATURE_LEVEL values we care about
// ---------------------------------------------------------------------------
enum class D3D11FeatureLevel : std::uint32_t {
    FL_11_1  = 0xb100,   // D3D_FEATURE_LEVEL_11_1 — compute shaders tier 2
    FL_11_0  = 0xb000,   // D3D_FEATURE_LEVEL_11_0 — compute shaders (min for GPU resize)
    FL_10_1  = 0xa100,   // D3D_FEATURE_LEVEL_10_1 — DXVA2 min level
    FL_10_0  = 0xa000,   // D3D_FEATURE_LEVEL_10_0
    FL_9_3   = 0x9300,   // D3D_FEATURE_LEVEL_9_3  — fallback BGRA blit only
    UNKNOWN  = 0,
};

// ---------------------------------------------------------------------------
// D3D11DeviceConfig
// ---------------------------------------------------------------------------
struct D3D11DeviceConfig final {
    // Minimum acceptable feature level (request fails if GPU is below this)
    D3D11FeatureLevel   minFeatureLevel  = D3D11FeatureLevel::FL_10_1;

    // Prefer hardware adapter; false = allow WARP software rasterizer
    bool                requireHardware  = false;

    // Enable D3D debug layer (forces debug SDK; never in release builds)
    bool                enableDebugLayer = false;

    // Adapter index (0 = default adapter, system will pick best)
    std::uint32_t       adapterIndex     = 0u;

    // Allow the blit pipeline to use BGRA render targets (requires DXGI 1.1+)
    bool                allowBGRATarget  = true;
};

// ---------------------------------------------------------------------------
// D3D11DeviceInfo — populated by D3D11DeviceManager::QueryInfo()
// ---------------------------------------------------------------------------
struct D3D11DeviceInfo final {
    wchar_t             adapterDescription[128]{};  // DXGI_ADAPTER_DESC.Description
    std::uint64_t       dedicatedVideoMemoryBytes{};
    std::uint64_t       sharedSystemMemoryBytes{};
    D3D11FeatureLevel   featureLevel{ D3D11FeatureLevel::UNKNOWN };
    bool                isWarpAdapter{};
    bool                isInitialised{};
};

// ---------------------------------------------------------------------------
// D3D11DeviceManagerStatus — result codes for Initialize / Reset
// ---------------------------------------------------------------------------
enum class D3D11DeviceManagerStatus : std::uint8_t {
    OK                  = 0,
    ALREADY_INITIALISED = 1,
    HARDWARE_UNAVAILABLE= 2,   // No adapter meets minFeatureLevel
    DEBUG_LAYER_MISSING = 3,   // D3D11SDKLayers.dll not present
    DEVICE_LOST         = 4,   // DXGI_ERROR_DEVICE_REMOVED during operation
    NOT_INITIALISED     = 5,   // Caller must call Initialize() first
};

// ---------------------------------------------------------------------------
// D3D11DeviceManager — contract interface
// ---------------------------------------------------------------------------
// Concrete implementation lives in D3D11DeviceManager.cpp (Phase 2).
// This header defines the stable API surface used by blit/decode stages.
//
class D3D11DeviceManager final {
public:
    // ── Lifecycle ─────────────────────────────────────────────────────────────

    D3D11DeviceManager() noexcept  = default;
    ~D3D11DeviceManager() noexcept = default;

    D3D11DeviceManager(const D3D11DeviceManager&)            = delete;
    D3D11DeviceManager& operator=(const D3D11DeviceManager&) = delete;

    // Initialise D3D11 device with the provided config.
    // Call once at LENSShell DllMain(DLL_PROCESS_ATTACH).
    [[nodiscard]] D3D11DeviceManagerStatus Initialize(
        const D3D11DeviceConfig& cfg = D3D11DeviceConfig{}) noexcept;

    // Release device and context; safe to call multiple times.
    void Shutdown() noexcept;

    // Attempt device recovery after TDR / device loss.
    [[nodiscard]] D3D11DeviceManagerStatus Reset(
        const D3D11DeviceConfig& cfg = D3D11DeviceConfig{}) noexcept;

    // ── State queries ──────────────────────────────────────────────────────────

    [[nodiscard]] bool IsReady() const noexcept;

    [[nodiscard]] D3D11DeviceInfo QueryInfo() const noexcept;

    [[nodiscard]] D3D11FeatureLevel GetFeatureLevel() const noexcept;

    // ── Constants ──────────────────────────────────────────────────────────────

    // Max adapters enumerated during initialize (prevents unbounded loop)
    static constexpr std::uint32_t kMaxAdaptersToTry = 8u;

    // Max consecutive TDR resets before the pipeline falls back to GDI+
    static constexpr std::uint32_t kMaxTDRResets = 3u;

    // Feature level required for compute-shader-based resize (S317)
    static constexpr D3D11FeatureLevel kMinComputeFeatureLevel = D3D11FeatureLevel::FL_11_0;

    // Feature level required for DXVA2 JPEG decode (S312)
    static constexpr D3D11FeatureLevel kMinDXVA2FeatureLevel = D3D11FeatureLevel::FL_10_1;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_D3D11_DEVICE_MANAGER_H
