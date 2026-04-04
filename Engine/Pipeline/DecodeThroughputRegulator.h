// DecodeThroughputRegulator.h — Throughput-based decode rate control
// Copyright (c) 2026 ExplorerLens Project
//
// Regulates decode throughput using a token-bucket algorithm, preventing
// resource exhaustion during burst thumbnail requests.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct DecodeThroughputRegulatorConfig
{
    bool enabled = true;
    uint32_t tokensPerSecond = 100;
    uint32_t maxBurst = 32;
    std::string label = "DecodeThroughputRegulator";
};

class DecodeThroughputRegulator
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_availableTokens = m_config.maxBurst;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    DecodeThroughputRegulatorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    bool TryConsume(uint32_t tokens = 1)
    {
        if (m_availableTokens >= tokens) {
            m_availableTokens -= tokens;
            m_totalConsumed += tokens;
            return true;
        }
        return false;
    }

    void Refill(uint32_t elapsedMs)
    {
        uint32_t newTokens = (m_config.tokensPerSecond * elapsedMs) / 1000;
        m_availableTokens =
            (m_availableTokens + newTokens > m_config.maxBurst) ? m_config.maxBurst : m_availableTokens + newTokens;
    }

    uint32_t GetAvailableTokens() const
    {
        return m_availableTokens;
    }
    uint64_t GetTotalConsumed() const
    {
        return m_totalConsumed;
    }

  private:
    bool m_initialized = false;
    DecodeThroughputRegulatorConfig m_config;
    uint32_t m_availableTokens = 0;
    uint64_t m_totalConsumed = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
