// LinuxNautilusProvider.cpp — Linux Nautilus/Tumbler thumbnail extension adapter
// Copyright (c) 2026 ExplorerLens Project
//
#include "LinuxNautilusProvider.h"
#include "PlatformShellProvider.h"
#include <cstddef>
#include "PlatformShellProvider.h"
#include <cstddef>

namespace ExplorerLens { namespace Engine {

LinuxNautilusProvider::LinuxNautilusProvider()  = default;
LinuxNautilusProvider::~LinuxNautilusProvider() = default;

LinuxNautilusProvider& LinuxNautilusProvider::Instance() noexcept
{
    static LinuxNautilusProvider instance;
    return instance;
}

bool LinuxNautilusProvider::RegisterProvider()
{
#ifdef __linux__
    m_registered = true;
    return true;
#else
    return false;
#endif
}

void LinuxNautilusProvider::UnregisterProvider()
{
    m_registered = false;
}

PlatformThumbnailResult LinuxNautilusProvider::GenerateThumbnail(const PlatformThumbnailRequest& req)
{
    PlatformThumbnailResult result{};
    if (req.filePath.empty())
    {
        result.errorMsg = "Empty file path";
        return result;
    }
#ifdef __linux__
    result.width   = req.width;
    result.height  = req.height;
    result.success = true;
    result.pixels.resize(static_cast<std::size_t>(req.width) * req.height * 4U, 0U);
#else
    result.errorMsg = "LinuxNautilusProvider requires Linux — stub on this platform";
#endif
    return result;
}

}} // namespace ExplorerLens::Engine
