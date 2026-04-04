// BatchAbortController.h — Coordinated batch decode cancellation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides cooperative cancellation for batch thumbnail operations.
// Workers check the abort token between decode stages for fast teardown.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct BatchAbortControllerConfig
{
    bool enabled = true;
    uint32_t gracePeriodMs = 500;
    std::string label = "BatchAbortController";
};

class BatchAbortController
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_aborted.store(false, std::memory_order_relaxed);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    BatchAbortControllerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    void Abort()
    {
        m_aborted.store(true, std::memory_order_release);
    }
    bool IsAborted() const
    {
        return m_aborted.load(std::memory_order_acquire);
    }
    void Reset()
    {
        m_aborted.store(false, std::memory_order_release);
    }

  private:
    bool m_initialized = false;
    BatchAbortControllerConfig m_config;
    std::atomic<bool> m_aborted{false};
};

}  // namespace Engine
}  // namespace ExplorerLens
