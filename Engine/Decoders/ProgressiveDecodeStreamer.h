// ProgressiveDecodeStreamer.h — Streams progressive decode updates to caller
// Copyright (c) 2026 ExplorerLens Project
//
// Delivers incremental decode progress for progressive JPEG/PNG formats,
// providing usable partial images before full decode completion.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ProgressiveDecodeStreamerConfig {
    bool enabled = true;
    uint32_t minPassForPreview = 2;
    std::string label = "ProgressiveDecodeStreamer";
};

class ProgressiveDecodeStreamer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ProgressiveDecodeStreamerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    void RecordPass(uint32_t passIndex, uint32_t linesDecoded) {
        m_currentPass = passIndex;
        m_totalLinesDecoded += linesDecoded;
    }

    bool IsPreviewReady() const {
        return m_currentPass >= m_config.minPassForPreview;
    }

    uint32_t GetCurrentPass() const { return m_currentPass; }
    uint32_t GetTotalLinesDecoded() const { return m_totalLinesDecoded; }

private:
    bool m_initialized = false;
    ProgressiveDecodeStreamerConfig m_config;
    uint32_t m_currentPass = 0;
    uint32_t m_totalLinesDecoded = 0;
};

}
} // namespace ExplorerLens::Engine
