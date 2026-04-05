// CrossPlatformSyncFence.h — GPU sync fence abstraction across D3D12/Metal/Vulkan
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a cross-platform GPU fence/semaphore abstraction used to synchronize
// CPU/GPU work. Maps to ID3D12Fence on Windows, MTLFence on macOS, and VkFence
// on Linux. On unsupported platforms the fence uses a lightweight spinlock stub.
//
#pragma once

#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class FenceState : uint8_t
{
    Idle        = 0,
    Signaled    = 1,
    WaitTimeout = 2,
    Error       = 3,
};

class CrossPlatformSyncFence
{
public:
    CrossPlatformSyncFence();
    ~CrossPlatformSyncFence();

    CrossPlatformSyncFence(const CrossPlatformSyncFence&)            = delete;
    CrossPlatformSyncFence& operator=(const CrossPlatformSyncFence&) = delete;

    bool       Create();
    void       Destroy();
    bool       Signal(uint64_t value = 1);
    FenceState Wait(uint64_t value = 1, uint32_t timeoutMs = 1000);
    uint64_t   GetCompletedValue() const noexcept { return m_completedValue; }
    bool       IsCreated()         const noexcept { return m_created; }
    void       Reset()             noexcept;
    FenceState GetState()          const noexcept { return m_state; }

private:
    bool       m_created        = false;
    uint64_t   m_completedValue = 0;
    FenceState m_state          = FenceState::Idle;
};

}} // namespace ExplorerLens::Engine
