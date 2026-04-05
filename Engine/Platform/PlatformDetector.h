// PlatformDetector.h — Runtime platform detection and provider factory
// Copyright (c) 2026 ExplorerLens Project
//
// Detects the current host platform at runtime and creates the appropriate
// PlatformShellProvider concrete implementation. Acts as the PAL entry point:
// callers use PlatformDetector to obtain a provider without coupling to a
// specific platform implementation.
//
#pragma once

#include "PlatformShellProvider.h"
#include <memory>
#include <string>

namespace ExplorerLens { namespace Engine {

class PlatformDetector
{
public:
    static PlatformKind  Detect()            noexcept;
    static const char*   PlatformName()      noexcept;
    static std::string   PlatformDescString();

    static std::unique_ptr<PlatformShellProvider> MakeProvider(PlatformKind kind);
    static std::unique_ptr<PlatformShellProvider> MakeCurrentPlatformProvider();
};

}} // namespace ExplorerLens::Engine
