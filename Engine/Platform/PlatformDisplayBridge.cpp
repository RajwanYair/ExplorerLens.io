// PlatformDisplayBridge.cpp — Platform-neutral display surface abstraction
// Copyright (c) 2026 ExplorerLens Project
//
#include "PlatformDisplayBridge.h"

namespace ExplorerLens { namespace Engine {

PlatformDisplayBridge PlatformDisplayBridge::s_instance;

PlatformDisplayBridge::PlatformDisplayBridge()  = default;
PlatformDisplayBridge::~PlatformDisplayBridge() { Detach(); }

PlatformDisplayBridge& PlatformDisplayBridge::Instance() noexcept
{
    return s_instance;
}

bool PlatformDisplayBridge::Attach()
{
    m_attached  = true;
    m_pushCount = 0;
    return true;
}

void PlatformDisplayBridge::Detach()
{
    m_attached = false;
}

DisplaySurfaceInfo PlatformDisplayBridge::QuerySurface() const noexcept
{
    DisplaySurfaceInfo info;
#if defined(_WIN32)
    info.dpi          = 96;
    info.hdrSupported = false;
    info.colorBitDepth = 8;
#endif
    return info;
}

bool PlatformDisplayBridge::PushPixels(const uint8_t* pixels, uint32_t width,
                                        uint32_t height, uint32_t /*stride*/)
{
    if (!m_attached || pixels == nullptr || width == 0 || height == 0)
        return false;
    ++m_pushCount;
    return true;
}

PlatformKind PlatformDisplayBridge::GetPlatform() const noexcept
{
#if defined(_WIN32)
    return PlatformKind::WINDOWS;
#elif defined(__APPLE__)
    return PlatformKind::MACOS;
#elif defined(__linux__)
    return PlatformKind::LINUX;
#else
    return PlatformKind::UNKNOWN;
#endif
}

}} // namespace ExplorerLens::Engine
