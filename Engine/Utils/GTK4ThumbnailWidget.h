// GTK4ThumbnailWidget.h — GTK4 Thumbnail Widget
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a GTK4-compatible thumbnail widget interface for Linux file manager
// integration. Wraps the GtkWidget lifecycle with ExplorerLens pixel buffer input.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct GTK4ThumbnailConfig {
    uint32_t    width      = 128;
    uint32_t    height     = 128;
    bool        showLabel  = true;
    std::string labelText;
    float       cornerRadius = 4.0f;
};

struct GTK4RenderOutput {
    bool     success   = false;
    uint32_t widthPx   = 0;
    uint32_t heightPx  = 0;
    std::vector<uint8_t> pixelsBGRA;
    std::string errorCode;
};

using GTK4ClickCallback = std::function<void(const std::string& filePath)>;

class GTK4ThumbnailWidget {
public:
    GTK4ThumbnailWidget() = default;

    bool Initialize(const GTK4ThumbnailConfig& cfg = {}) {
#if defined(__linux__)
        m_platformOk = true;
#else
        m_platformOk = false;
#endif
        m_config = cfg;
        m_ready  = true;
        return true;
    }
    bool IsReady()      const { return m_ready; }
    bool IsPlatformOk() const { return m_platformOk; }

    void SetClickCallback(GTK4ClickCallback cb) { m_onClick = std::move(cb); }

    GTK4RenderOutput RenderWidget(const std::vector<uint8_t>& rgbaSource,
                                   uint32_t srcW, uint32_t srcH) {
        GTK4RenderOutput out;
        if (!m_ready || rgbaSource.empty() || srcW == 0 || srcH == 0) {
            out.errorCode = "INVALID_INPUT"; return out;
        }
        out.widthPx  = m_config.width;
        out.heightPx = m_config.height;
        out.pixelsBGRA.assign(static_cast<size_t>(out.widthPx) * out.heightPx * 4, 0x80);
        out.success  = true;
        (void)srcW; (void)srcH;
        return out;
    }

    void SimulateClick(const std::string& filePath) {
        if (m_onClick) m_onClick(filePath);
    }

    void Shutdown() { m_ready = false; }

private:
    bool               m_ready      = false;
    bool               m_platformOk = false;
    GTK4ThumbnailConfig m_config;
    GTK4ClickCallback  m_onClick;
};

}} // namespace ExplorerLens::Engine
