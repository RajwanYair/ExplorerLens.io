// Win32ShellProvider.h — Windows IThumbnailProvider COM shell adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Implements PlatformShellProvider for the Windows platform. Wraps the
// IThumbnailProvider COM interface, routing thumbnail requests through
// ExplorerLensEngine decoder pipeline. On non-Windows platforms this
// class compiles as a no-op stub to satisfy the PAL contract.
//
#pragma once

#include "PlatformShellProvider.h"

namespace ExplorerLens { namespace Engine {

class Win32ShellProvider final : public PlatformShellProvider
{
public:
    Win32ShellProvider();
    ~Win32ShellProvider() override;

    bool                    RegisterProvider()                                     override;
    void                    UnregisterProvider()                                   override;
    PlatformThumbnailResult GenerateThumbnail(const PlatformThumbnailRequest& req) override;
    PlatformKind            GetPlatform()    const noexcept override { return PlatformKind::WINDOWS; }
    const char*             GetPlatformName() const noexcept override { return "Windows/IThumbnailProvider"; }
    bool                    IsRegistered()   const noexcept override { return m_registered; }

    static Win32ShellProvider& Instance() noexcept;

private:
    bool m_registered = false;
};

}} // namespace ExplorerLens::Engine
