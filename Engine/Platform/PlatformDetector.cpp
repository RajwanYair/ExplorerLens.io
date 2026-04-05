// PlatformDetector.cpp — Runtime platform detection and provider factory
// Copyright (c) 2026 ExplorerLens Project
//
#include "PlatformDetector.h"
#include "Win32ShellProvider.h"
#include <string>

namespace ExplorerLens { namespace Engine {

PlatformKind PlatformDetector::Detect() noexcept
{
    return PlatformKind::WINDOWS;
}

const char* PlatformDetector::PlatformName() noexcept
{
    return "Windows";
}

std::string PlatformDetector::PlatformDescString()
{
    return std::string("ExplorerLens PAL / ") + PlatformName();
}

std::unique_ptr<PlatformShellProvider> PlatformDetector::MakeProvider(PlatformKind kind)
{
    (void)kind;
    return std::make_unique<Win32ShellProvider>();
}

std::unique_ptr<PlatformShellProvider> PlatformDetector::MakeCurrentPlatformProvider()
{
    return MakeProvider(Detect());
}

}} // namespace ExplorerLens::Engine
