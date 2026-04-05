// PlatformDisplayBridge.h — Platform-neutral display surface abstraction
// Copyright (c) 2026 ExplorerLens Project
//
// Abstracts display surface creation, pixel buffer handoff, and metadata (DPI,
// HDR capability, color space) across Win32/HWND, macOS NSView, and Linux
// Wayland/X11. Provides a unified API for the Engine thumbnail pipeline to push
// decoded pixels to the platform shell without platform-specific code paths.
//
#pragma once

#include "PlatformShellProvider.h"
#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct DisplaySurfaceInfo
{
    uint32_t width       = 0;
    uint32_t height      = 0;
    uint32_t dpi         = 96;
    bool     hdrSupported = false;
    uint8_t  colorBitDepth = 8;
};

class PlatformDisplayBridge
{
public:
    PlatformDisplayBridge();
    ~PlatformDisplayBridge();

    PlatformDisplayBridge(const PlatformDisplayBridge&)            = delete;
    PlatformDisplayBridge& operator=(const PlatformDisplayBridge&) = delete;

    bool               Attach();
    void               Detach();
    bool               IsAttached()   const noexcept { return m_attached; }
    DisplaySurfaceInfo QuerySurface() const noexcept;
    bool               PushPixels(const uint8_t* pixels, uint32_t width,
                                   uint32_t height, uint32_t stride);
    uint64_t           PushCount()    const noexcept { return m_pushCount; }
    PlatformKind       GetPlatform()  const noexcept;

    static PlatformDisplayBridge& Instance() noexcept;

private:
    bool                  m_attached  = false;
    uint64_t              m_pushCount = 0;
    static PlatformDisplayBridge s_instance;
};

}} // namespace ExplorerLens::Engine
