// MetalGPUBackend.h — macOS Metal v2 GPU backend stub
// Copyright (c) 2026 ExplorerLens Project
//
// Stub implementation of the Metal v2 GPU rendering backend for macOS Quick Look.
// Compiles as a no-op on Windows (MSVC) — actual Metal API calls are guarded by
// __APPLE__. Satisfies the PlatformGPURouter contract on all platforms.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class MetalBackendState : uint8_t
{
    Uninitialized = 0,
    Ready         = 1,
    Fault         = 2,
};

class MetalGPUBackend
{
public:
    MetalGPUBackend();
    ~MetalGPUBackend();

    MetalGPUBackend(const MetalGPUBackend&)            = delete;
    MetalGPUBackend& operator=(const MetalGPUBackend&) = delete;

    bool              Initialize();
    void              Shutdown();
    MetalBackendState GetState()     const noexcept { return m_state; }
    const char*       BackendName()  const noexcept { return "Metal-v2-Stub"; }
    bool              IsAvailable()  const noexcept;
    uint64_t          DeviceMemoryMB() const noexcept { return 0; }
    std::string       DeviceDescription() const;

    static MetalGPUBackend& Instance() noexcept;

private:
    MetalBackendState    m_state = MetalBackendState::Uninitialized;
    static MetalGPUBackend s_instance;
};

}} // namespace ExplorerLens::Engine
