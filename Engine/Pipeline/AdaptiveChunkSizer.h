// AdaptiveChunkSizer.h — Dynamically sizes decode chunks based on throughput
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors decode throughput and adjusts the chunk size fed to decoders
// to optimize cache line utilization and minimize pipeline stalls.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct AdaptiveChunkSizerConfig
{
    bool enabled = true;
    uint32_t minChunkKB = 4;
    uint32_t maxChunkKB = 256;
    std::string label = "AdaptiveChunkSizer";
};

class AdaptiveChunkSizer
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_currentChunkKB = m_config.minChunkKB;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    AdaptiveChunkSizerConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    void RecordThroughput(double mbPerSec)
    {
        if (mbPerSec > m_lastThroughput && m_currentChunkKB < m_config.maxChunkKB)
            m_currentChunkKB *= 2;
        else if (mbPerSec < m_lastThroughput * 0.8 && m_currentChunkKB > m_config.minChunkKB)
            m_currentChunkKB /= 2;
        m_lastThroughput = mbPerSec;
    }

    uint32_t GetCurrentChunkKB() const
    {
        return m_currentChunkKB;
    }

  private:
    bool m_initialized = false;
    AdaptiveChunkSizerConfig m_config;
    uint32_t m_currentChunkKB = 16;
    double m_lastThroughput = 0.0;
};

}  // namespace Engine
}  // namespace ExplorerLens
