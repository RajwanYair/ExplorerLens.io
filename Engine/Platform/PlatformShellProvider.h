// PlatformShellProvider.h — Abstract shell provider base for cross-platform PAL
// Copyright (c) 2026 ExplorerLens Project
//
// Pure interface abstracting Windows IThumbnailProvider COM, macOS QLThumbnailRequest,
// and Linux Nautilus/Tumbler shell extension registration and thumbnail dispatch.
// Concrete platform providers inherit from this class to implement the PAL contract.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class PlatformKind : uint8_t
{
    WINDOWS = 0,
    MACOS   = 1,
    LINUX   = 2,
    UNKNOWN = 255,
};

struct PlatformThumbnailRequest
{
    std::wstring filePath;
    uint32_t     width  = 256;
    uint32_t     height = 256;
};

struct PlatformThumbnailResult
{
    bool                 success = false;
    std::vector<uint8_t> pixels;
    uint32_t             width  = 0;
    uint32_t             height = 0;
    std::string          errorMsg;
};

class PlatformShellProvider
{
public:
    virtual ~PlatformShellProvider() = default;

    PlatformShellProvider(const PlatformShellProvider&)            = delete;
    PlatformShellProvider& operator=(const PlatformShellProvider&) = delete;

    virtual bool                    RegisterProvider()                                        = 0;
    virtual void                    UnregisterProvider()                                      = 0;
    virtual PlatformThumbnailResult GenerateThumbnail(const PlatformThumbnailRequest& req)    = 0;
    virtual PlatformKind            GetPlatform()    const noexcept                           = 0;
    virtual const char*             GetPlatformName() const noexcept                          = 0;
    virtual bool                    IsRegistered()   const noexcept                           = 0;

protected:
    PlatformShellProvider() = default;
};

}} // namespace ExplorerLens::Engine
