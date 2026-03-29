// ARPassthroughVideoEngine.h — AR Passthrough Video Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages real-time camera passthrough video feed and composites
// ExplorerLens thumbnail overlays onto the live video stream.
//
#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct PassthroughFrame {
    uint32_t             width   = 0;
    uint32_t             height  = 0;
    std::vector<uint8_t> rgbaData;
    uint64_t             timestampUs = 0;
    float                exposureMs  = 0.0f;
};

struct OverlayEntry {
    uint64_t             anchorId;
    std::vector<uint8_t> thumbnailRGBA;
    float                x = 0.0f, y = 0.0f;
    float                widthNDC = 0.2f, heightNDC = 0.2f;
    float                opacity  = 1.0f;
};

class ARPassthroughVideoEngine {
public:
    ARPassthroughVideoEngine() = default;

    bool Start(uint32_t width, uint32_t height, uint32_t fps = 60) {
        m_width   = width;
        m_height  = height;
        m_fps     = fps;
        m_running = true;
        return true;
    }

    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running; }

    bool AddOverlay(const OverlayEntry& overlay) {
        m_overlays.push_back(overlay);
        return true;
    }

    bool RemoveOverlay(uint64_t anchorId) {
        auto it = std::find_if(m_overlays.begin(), m_overlays.end(),
            [anchorId](const OverlayEntry& o){ return o.anchorId == anchorId; });
        if (it == m_overlays.end()) return false;
        m_overlays.erase(it);
        return true;
    }

    PassthroughFrame CompositeFrame(const PassthroughFrame& camera) const {
        PassthroughFrame out = camera;
        // Overlay thumbnails onto passthrough frame (stub)
        (void)m_overlays;
        return out;
    }

    uint32_t GetWidth()  const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetFPS()    const { return m_fps;    }
    uint32_t GetOverlayCount() const { return static_cast<uint32_t>(m_overlays.size()); }

private:
    uint32_t                 m_width   = 1920;
    uint32_t                 m_height  = 1080;
    uint32_t                 m_fps     = 60;
    bool                     m_running = false;
    std::vector<OverlayEntry> m_overlays;
};

}} // namespace ExplorerLens::Engine
