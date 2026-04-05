// PlatformDetector.cpp — Runtime platform detection and provider factory
// Copyright (c) 2026 ExplorerLens Project
//
#include "PlatformDetector.h"
#include "Win32ShellProvider.h"
#include "MacOSQLProvider.h"
#include "LinuxNautilusProvider.h"
#include <string>

namespace ExplorerLens { namespace Engine {

PlatformKind PlatformDetector::Detect() noexcept
{
#ifdef _WIN32
    return PlatformKind::WINDOWS;
#elif defined(__APPLE__)
    return PlatformKind::MACOS;
#elif defined(__linux__)
    return PlatformKind::LINUX;
#else
    return PlatformKind::UNKNOWN;
#endif
}

const char* PlatformDetector::PlatformName() noexcept
{
#ifdef _WIN32
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

std::string PlatformDetector::PlatformDescString()
{
    return std::string("ExplorerLens PAL / ") + PlatformName();
}

std::unique_ptr<PlatformShellProvider> PlatformDetector::MakeProvider(PlatformKind kind)
{
    switch (kind)
    {
    case PlatformKind::WINDOWS: return std::make_unique<Win32ShellProvider>();
    case PlatformKind::MACOS:   return std::make_unique<MacOSQLProvider>();
    case PlatformKind::LINUX:   return std::make_unique<LinuxNautilusProvider>();
    default:                    return std::make_unique<Win32ShellProvider>();
    }
}

std::unique_ptr<PlatformShellProvider> PlatformDetector::MakeCurrentPlatformProvider()
{
    return MakeProvider(Detect());
}

}} // namespace ExplorerLens::Engine
