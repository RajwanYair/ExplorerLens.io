// ThumbnailStreamMultiplexer.h — Routes thumbnail requests across decode streams
// Copyright (c) 2026 ExplorerLens Project
//
// Multiplexes incoming thumbnail requests across multiple concurrent decode
// streams, load-balancing by format type and estimated decode cost.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

struct ThumbnailStreamMultiplexerConfig {
    bool enabled = true;
    uint32_t maxStreams = 8;
    uint32_t queueDepthPerStream = 32;
    std::string label = "ThumbnailStreamMultiplexer";
};

class ThumbnailStreamMultiplexer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_streamLoads.resize(m_config.maxStreams, 0);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ThumbnailStreamMultiplexerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    uint32_t SelectStream(uint32_t formatCost) {
        uint32_t best = 0;
        uint32_t bestLoad = m_streamLoads.empty() ? 0 : m_streamLoads[0];
        for (uint32_t i = 1; i < m_streamLoads.size(); ++i) {
            if (m_streamLoads[i] < bestLoad) {
                bestLoad = m_streamLoads[i];
                best = i;
            }
        }
        if (best < m_streamLoads.size())
            m_streamLoads[best] += formatCost;
        return best;
    }

    void ReleaseStream(uint32_t streamIdx, uint32_t cost) {
        if (streamIdx < m_streamLoads.size() && m_streamLoads[streamIdx] >= cost)
            m_streamLoads[streamIdx] -= cost;
    }

    uint32_t GetStreamCount() const { return m_config.maxStreams; }
    uint32_t GetStreamLoad(uint32_t idx) const {
        return idx < m_streamLoads.size() ? m_streamLoads[idx] : 0;
    }

private:
    bool m_initialized = false;
    ThumbnailStreamMultiplexerConfig m_config;
    std::vector<uint32_t> m_streamLoads;
};

}
} // namespace ExplorerLens::Engine
