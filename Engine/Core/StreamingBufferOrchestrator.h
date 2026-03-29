// StreamingBufferOrchestrator.h — Streaming Buffer Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Manages elastic ring buffers for live streaming decode with adaptive backpressure.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <deque>

namespace ExplorerLens { namespace Engine {

enum class SBOBufferState { Idle, Filling, Full, Draining };

struct SBOConfig {
    uint32_t targetBufferMs  = 2000;
    uint32_t maxBufferMs     = 8000;
    uint32_t chunkSizeBytes  = 65536;
};

struct SBOStatus {
    SBOBufferState state             = SBOBufferState::Idle;
    uint32_t       bufferedMs        = 0;
    float          fillRatio         = 0.0f;
};

class StreamingBufferOrchestrator {
public:
    explicit StreamingBufferOrchestrator(const SBOConfig& config) : m_config(config) {}

    bool PushChunk(const std::vector<uint8_t>& chunk) {
        if (m_bufferedBytes + chunk.size() > m_config.maxBufferMs * 250) return false; // backpressure
        m_chunks.push_back(chunk);
        m_bufferedBytes += static_cast<uint32_t>(chunk.size());
        return true;
    }
    std::vector<uint8_t> PopChunk() {
        if (m_chunks.empty()) return {};
        auto c = m_chunks.front();
        m_chunks.pop_front();
        m_bufferedBytes -= static_cast<uint32_t>(c.size());
        return c;
    }
    SBOStatus GetStatus() const {
        SBOStatus s;
        s.bufferedMs = m_bufferedBytes / 250;
        s.fillRatio  = static_cast<float>(s.bufferedMs) / static_cast<float>(m_config.targetBufferMs);
        s.state      = m_chunks.empty() ? SBOBufferState::Idle :
                       (s.bufferedMs >= m_config.targetBufferMs ? SBOBufferState::Full : SBOBufferState::Filling);
        return s;
    }

private:
    SBOConfig                           m_config;
    std::deque<std::vector<uint8_t>>    m_chunks;
    uint32_t                            m_bufferedBytes = 0;
};

}} // namespace ExplorerLens::Engine
