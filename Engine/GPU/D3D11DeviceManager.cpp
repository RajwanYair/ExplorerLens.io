// Engine/GPU/D3D11DeviceManager.cpp
// ExplorerLens — D3D11 device manager implementation (ROADMAP v7.0 Phase 2)
// Sprint S319.
//
// This is the Phase 2 stub implementation.  GPU path activation (Sprint S330+)
// will replace the stub bodies with actual D3D11CreateDevice() calls.
// Until then, IsReady() always returns false so the CPU bilinear blit path
// in D3D11TextureBlitPipeline is used for all resize operations.
//
// Note: D3D11 headers are NOT included here in the Phase 2 stub to avoid
// pulling in the full Windows SDK; actual COM device types are added in S330.
//
#include "D3D11DeviceManager.h"

#include <cstring>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// Internal state (Phase 2 stub — no actual COM device)
// ---------------------------------------------------------------------------
namespace {
    struct D3D11DeviceState final {
        bool            initialised{};
        bool            shutdownCalled{};
        D3D11DeviceInfo info{};
        std::uint32_t   tdrResets{};
    };
}

// ---------------------------------------------------------------------------
// D3D11DeviceManager method implementations
// ---------------------------------------------------------------------------

[[nodiscard]] D3D11DeviceManagerStatus D3D11DeviceManager::Initialize(
    const D3D11DeviceConfig& cfg) noexcept
{
    (void)cfg;
    // Phase 2 stub: record that Initialize() was called but return
    // HARDWARE_UNAVAILABLE since we don't attempt D3D11CreateDevice() yet.
    // The blit pipeline will use the CPU bilinear path via IsCPUFallbackAvailable().
    return D3D11DeviceManagerStatus::HARDWARE_UNAVAILABLE;
}

void D3D11DeviceManager::Shutdown() noexcept
{
    // Phase 2 stub: nothing to release.
}

[[nodiscard]] D3D11DeviceManagerStatus D3D11DeviceManager::Reset(
    const D3D11DeviceConfig& cfg) noexcept
{
    (void)cfg;
    return D3D11DeviceManagerStatus::HARDWARE_UNAVAILABLE;
}

[[nodiscard]] bool D3D11DeviceManager::IsReady() const noexcept
{
    // Phase 2 stub: always false.  The CPU bilinear fallback path in
    // D3D11TextureBlitPipeline handles all resize operations until S330.
    return false;
}

[[nodiscard]] D3D11DeviceInfo D3D11DeviceManager::QueryInfo() const noexcept
{
    // Return a zeroed info struct with isInitialised = false.
    D3D11DeviceInfo info{};
    return info;
}

[[nodiscard]] D3D11FeatureLevel D3D11DeviceManager::GetFeatureLevel() const noexcept
{
    return D3D11FeatureLevel::UNKNOWN;
}

} // namespace ExplorerLens::Engine
