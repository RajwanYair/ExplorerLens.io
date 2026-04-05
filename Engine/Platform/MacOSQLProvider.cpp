// MacOSQLProvider.cpp — macOS Quick Look QLThumbnailRequest shell adapter
// Copyright (c) 2026 ExplorerLens Project
//
#include "MacOSQLProvider.h"
#include "PlatformShellProvider.h"
#include <cstddef>
#include "PlatformShellProvider.h"
#include <cstddef>

namespace ExplorerLens { namespace Engine {

MacOSQLProvider::MacOSQLProvider()  = default;
MacOSQLProvider::~MacOSQLProvider() = default;

MacOSQLProvider& MacOSQLProvider::Instance() noexcept
{
    static MacOSQLProvider instance;
    return instance;
}

bool MacOSQLProvider::RegisterProvider()
{
#ifdef __APPLE__
    m_registered = true;
    return true;
#else
    return false;
#endif
}

void MacOSQLProvider::UnregisterProvider()
{
    m_registered = false;
}

PlatformThumbnailResult MacOSQLProvider::GenerateThumbnail(const PlatformThumbnailRequest& req)
{
    PlatformThumbnailResult result{};
    if (req.filePath.empty())
    {
        result.errorMsg = "Empty file path";
        return result;
    }
#ifdef __APPLE__
    result.width   = req.width;
    result.height  = req.height;
    result.success = true;
    result.pixels.resize(static_cast<std::size_t>(req.width) * req.height * 4U, 0U);
#else
    result.errorMsg = "MacOSQLProvider requires macOS — stub on this platform";
#endif
    return result;
}

}} // namespace ExplorerLens::Engine
