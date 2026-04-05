// MetalGPUBackend.cpp — macOS Metal v2 GPU backend stub
// Copyright (c) 2026 ExplorerLens Project
//
#include "MetalGPUBackend.h"

namespace ExplorerLens { namespace Engine {

MetalGPUBackend MetalGPUBackend::s_instance;

MetalGPUBackend::MetalGPUBackend()  = default;
MetalGPUBackend::~MetalGPUBackend() { Shutdown(); }

MetalGPUBackend& MetalGPUBackend::Instance() noexcept { return s_instance; }

bool MetalGPUBackend::Initialize()
{
#ifdef __APPLE__
    // Actual Metal device enumeration would happen here.
    m_state = MetalBackendState::Ready;
    return true;
#else
    m_state = MetalBackendState::Fault;
    return false;
#endif
}

void MetalGPUBackend::Shutdown()
{
    m_state = MetalBackendState::Uninitialized;
}

bool MetalGPUBackend::IsAvailable() const noexcept
{
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

std::string MetalGPUBackend::DeviceDescription() const
{
#ifdef __APPLE__
    return "Apple Metal v2 (GPU)";
#else
    return "Metal-v2-Stub (non-Apple platform)";
#endif
}

}} // namespace ExplorerLens::Engine
