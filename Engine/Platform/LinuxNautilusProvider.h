// LinuxNautilusProvider.h — Linux Nautilus/Tumbler thumbnail extension adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Implements PlatformShellProvider for Linux using the Nautilus thumbnailer spec
// (GNOME Thumbnailer D-Bus) and KDE Dolphin ThumbCreator interface. Integrates with
// the XDG thumbnail cache spec (Tumbler). On Windows/macOS this compiles as a no-op
// stub to enable unified PAL compilation across all target platforms.
//
#pragma once

#include "PlatformShellProvider.h"

namespace ExplorerLens { namespace Engine {

class LinuxNautilusProvider final : public PlatformShellProvider
{
public:
    LinuxNautilusProvider();
    ~LinuxNautilusProvider() override;

    bool                    RegisterProvider()                                     override;
    void                    UnregisterProvider()                                   override;
    PlatformThumbnailResult GenerateThumbnail(const PlatformThumbnailRequest& req) override;
    PlatformKind            GetPlatform()    const noexcept override { return PlatformKind::LINUX; }
    const char*             GetPlatformName() const noexcept override { return "Linux/Nautilus+Tumbler"; }
    bool                    IsRegistered()   const noexcept override { return m_registered; }

    static LinuxNautilusProvider& Instance() noexcept;

private:
    bool m_registered = false;
};

}} // namespace ExplorerLens::Engine
