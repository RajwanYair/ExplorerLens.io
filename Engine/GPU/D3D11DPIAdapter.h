// D3D11DPIAdapter.h — DirectX 11 Swap Chain DPI Adaptation
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps ID3D11Device swap chain creation with per-monitor DPI awareness,
// ensuring render targets match the physical pixel density of each display.
//
#pragma once

#include <cstdint>
#include <string>
#include <memory>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain1;
struct IDXGIFactory2;

namespace ExplorerLens {
namespace Engine {

// ---- Swap Chain DPI Configuration -------------------------------------------

struct D3D11SwapChainDPIConfig {
    uint32_t logicalWidth     = 256;   // Logical (shell-requested) width
    uint32_t logicalHeight    = 256;
    uint32_t monitorDPI       = 96;
    float    scaleFactor      = 1.0f;
    bool     enableHDR        = false; // Use DXGI_COLOR_SPACE_RGB_FULL_G2084 for HDR
    bool     enableMSAA       = false;
    uint32_t msaaSamples      = 4;
};

struct D3D11DPISwapChain {
    IDXGISwapChain1*    swapChain     = nullptr;
    uint32_t            physicalWidth  = 0;
    uint32_t            physicalHeight = 0;
    float               scaleFactor    = 1.0f;
    bool                hdrEnabled     = false;
};

// ---- Adapter ----------------------------------------------------------------

class D3D11DPIAdapter {
public:
    D3D11DPIAdapter();
    ~D3D11DPIAdapter();

    // Initialize with an existing D3D11 device.
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* ctx);
    void Shutdown();
    bool IsInitialized() const;

    // Create a DPI-aware swap chain for the given HWND and DPI config.
    D3D11DPISwapChain CreateSwapChain(
        void*                            hwnd,
        const D3D11SwapChainDPIConfig&   cfg) const;

    // Update an existing swap chain's physical buffer size on DPI change.
    bool ResizeSwapChain(
        IDXGISwapChain1*   swapChain,
        uint32_t           newPhysicalWidth,
        uint32_t           newPhysicalHeight) const;

    // Blit a staging texture (BGRA) to the swap chain with DPI scaling.
    bool BlitToSwapChain(
        IDXGISwapChain1*   swapChain,
        const uint8_t*     bgraPixels,
        uint32_t           srcWidth,
        uint32_t           srcHeight) const;

    ID3D11Device*        Device()  const;
    ID3D11DeviceContext* Context() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
