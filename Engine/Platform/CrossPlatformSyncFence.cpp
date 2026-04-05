// CrossPlatformSyncFence.cpp — GPU sync fence abstraction
// Copyright (c) 2026 ExplorerLens Project
//
#include "CrossPlatformSyncFence.h"

namespace ExplorerLens { namespace Engine {

CrossPlatformSyncFence::CrossPlatformSyncFence()  = default;
CrossPlatformSyncFence::~CrossPlatformSyncFence() { Destroy(); }

bool CrossPlatformSyncFence::Create()
{
    if (m_created)
        return true;
    m_completedValue = 0;
    m_state          = FenceState::Idle;
    m_created        = true;
    return true;
}

void CrossPlatformSyncFence::Destroy()
{
    m_created        = false;
    m_completedValue = 0;
    m_state          = FenceState::Idle;
}

bool CrossPlatformSyncFence::Signal(uint64_t value)
{
    if (!m_created)
        return false;
    m_completedValue = value;
    m_state          = FenceState::Signaled;
    return true;
}

FenceState CrossPlatformSyncFence::Wait(uint64_t value, uint32_t /*timeoutMs*/)
{
    if (!m_created)
        return FenceState::Error;
    if (m_completedValue >= value)
        return FenceState::Signaled;
    return FenceState::WaitTimeout;
}

void CrossPlatformSyncFence::Reset() noexcept
{
    m_completedValue = 0;
    m_state          = FenceState::Idle;
}

}} // namespace ExplorerLens::Engine
