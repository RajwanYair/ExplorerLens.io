// VulkanEGLBackend.h — Linux Vulkan + EGL GPU backend stub
// Copyright (c) 2026 ExplorerLens Project
//
// Stub implementation of the Vulkan/EGL rendering backend for Linux Nautilus/
// Dolphin/Wayland thumbnail providers. Compiles as a no-op on Windows — actual
// Vulkan + EGL surface calls are guarded by __linux__. Satisfies the router contract.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class VulkanEGLState : uint8_t
{
    Uninitialized = 0,
    Ready         = 1,
    NoDevice      = 2,
    Error         = 3,
};

class VulkanEGLBackend
{
public:
    VulkanEGLBackend();
    ~VulkanEGLBackend();

    VulkanEGLBackend(const VulkanEGLBackend&)            = delete;
    VulkanEGLBackend& operator=(const VulkanEGLBackend&) = delete;

    bool           Initialize();
    void           Shutdown();
    VulkanEGLState GetState()       const noexcept { return m_state; }
    const char*    BackendName()    const noexcept { return "Vulkan-EGL-Stub"; }
    bool           IsAvailable()    const noexcept;
    uint32_t       MaxTextureSize() const noexcept { return 0; }
    std::string    DriverVersion()  const;

    static VulkanEGLBackend& Instance() noexcept;

private:
    VulkanEGLState   m_state = VulkanEGLState::Uninitialized;
    static VulkanEGLBackend s_instance;
};

}} // namespace ExplorerLens::Engine
