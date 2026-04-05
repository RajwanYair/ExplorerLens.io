// MacOSQLProvider.h — macOS Quick Look QLThumbnailRequest shell adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Implements PlatformShellProvider for macOS using the Quick Look framework's
// QLThumbnailRequest API (macOS 10.15+). On Windows/Linux this class compiles
// as a no-op stub, enabling cross-platform PAL compilation without native SDKs.
//
#pragma once

#include "PlatformShellProvider.h"

namespace ExplorerLens { namespace Engine {

class MacOSQLProvider final : public PlatformShellProvider
{
public:
    MacOSQLProvider();
    ~MacOSQLProvider() override;

    bool                    RegisterProvider()                                     override;
    void                    UnregisterProvider()                                   override;
    PlatformThumbnailResult GenerateThumbnail(const PlatformThumbnailRequest& req) override;
    PlatformKind            GetPlatform()    const noexcept override { return PlatformKind::MACOS; }
    const char*             GetPlatformName() const noexcept override { return "macOS/QLThumbnailRequest"; }
    bool                    IsRegistered()   const noexcept override { return m_registered; }

    static MacOSQLProvider& Instance() noexcept;

private:
    bool m_registered = false;
};

}} // namespace ExplorerLens::Engine
