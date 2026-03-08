// MultiFrameDecodeRouter.h — Routes multi-frame formats to appropriate handler
// Copyright (c) 2026 ExplorerLens Project
//
// Handles formats with multiple frames (GIF, APNG, multi-page TIFF, ICO)
// by selecting the best frame for thumbnail (largest, first, or key frame).
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct MultiFrameDecodeRouterConfig {
    bool enabled = true;
    uint32_t maxFrameCount = 1000;
    std::string label = "MultiFrameDecodeRouter";
};

class MultiFrameDecodeRouter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    MultiFrameDecodeRouterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class FrameSelectionPolicy : uint8_t { First, Largest, KeyFrame, Middle };

    struct FrameInfo {
        uint32_t index = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        bool isKeyFrame = false;
    };

    uint32_t SelectBestFrame(const FrameInfo* frames, uint32_t count, FrameSelectionPolicy policy) const {
        if (!frames || count == 0) return 0;
        if (policy == FrameSelectionPolicy::First) return 0;
        if (policy == FrameSelectionPolicy::Middle) return count / 2;
        if (policy == FrameSelectionPolicy::Largest) {
            uint32_t best = 0;
            uint64_t bestArea = 0;
            for (uint32_t i = 0; i < count; ++i) {
                uint64_t area = static_cast<uint64_t>(frames[i].width) * frames[i].height;
                if (area > bestArea) { bestArea = area; best = i; }
            }
            return best;
        }
        return 0;
    }

private:
    bool m_initialized = false;
    MultiFrameDecodeRouterConfig m_config;
};

}
} // namespace ExplorerLens::Engine
