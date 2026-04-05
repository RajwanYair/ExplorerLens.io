// VulkanEGLBackend.cpp — Linux Vulkan + EGL GPU backend stub
// Copyright (c) 2026 ExplorerLens Project
//
#include "VulkanEGLBackend.h"

namespace ExplorerLens { namespace Engine {

VulkanEGLBackend VulkanEGLBackend::s_instance;

VulkanEGLBackend::VulkanEGLBackend()  = default;
VulkanEGLBackend::~VulkanEGLBackend() { Shutdown(); }

VulkanEGLBackend& VulkanEGLBackend::Instance() noexcept { return s_instance; }

bool VulkanEGLBackend::Initialize()
{
#ifdef __linux__
    m_state = VulkanEGLState::Ready;
    return true;
#else
    m_state = VulkanEGLState::NoDevice;
    return false;
#endif
}

void VulkanEGLBackend::Shutdown()
{
    m_state = VulkanEGLState::Uninitialized;
}

bool VulkanEGLBackend::IsAvailable() const noexcept
{
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

std::string VulkanEGLBackend::DriverVersion() const
{
#ifdef __linux__
    return "Vulkan-1.3-EGL-Stub";
#else
    return "Vulkan-EGL-Stub (non-Linux platform)";
#endif
}

}} // namespace ExplorerLens::Engine
