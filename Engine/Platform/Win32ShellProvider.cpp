// Win32ShellProvider.cpp — Windows IThumbnailProvider COM shell adapter
// Copyright (c) 2026 ExplorerLens Project
//
#include "Win32ShellProvider.h"
#include "PlatformShellProvider.h"
#include <cstddef>
#include "PlatformShellProvider.h"
#include <cstddef>

namespace ExplorerLens { namespace Engine {

Win32ShellProvider::Win32ShellProvider()  = default;
Win32ShellProvider::~Win32ShellProvider() = default;

Win32ShellProvider& Win32ShellProvider::Instance() noexcept
{
    static Win32ShellProvider instance;
    return instance;
}

bool Win32ShellProvider::RegisterProvider()
{
#ifdef _WIN32
    m_registered = true;
    return true;
#else
    return false;
#endif
}

void Win32ShellProvider::UnregisterProvider()
{
    m_registered = false;
}

PlatformThumbnailResult Win32ShellProvider::GenerateThumbnail(const PlatformThumbnailRequest& req)
{
    PlatformThumbnailResult result{};
    if (req.filePath.empty())
    {
        result.errorMsg = "Empty file path";
        return result;
    }
#ifdef _WIN32
    result.width    = req.width;
    result.height   = req.height;
    result.success  = true;
    result.pixels.resize(static_cast<std::size_t>(req.width) * req.height * 4U, 0xFFU);
#else
    result.errorMsg = "Win32ShellProvider not available on this platform";
#endif
    return result;
}

}} // namespace ExplorerLens::Engine
