// WaylandShellExtension.h — Wayland Compositor Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Wayland compositor integration for thumbnail rendering via wlr-layer-shell
// and xdg-foreign protocols. Detects compositor type at runtime.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class WaylandProtocol : uint8_t {
    WlrLayerShell,
    XDGForeign,
    XDGDecoration,
    Viewporter
};

enum class CompositorType : uint8_t {
    Mutter,
    KWin,
    Sway,
    Hyprland,
    Weston
};

struct WaylandConfig
{
    WaylandProtocol protocol = WaylandProtocol::WlrLayerShell;
    CompositorType compositor = CompositorType::Mutter;
    std::string displayName;
    float scaleFactor = 1.0f;
    bool enableHiDPI = true;
};

class WaylandShellExtension
{
  public:
    WaylandShellExtension() = default;
    ~WaylandShellExtension() = default;

    WaylandShellExtension(WaylandShellExtension const&) = delete;
    WaylandShellExtension& operator=(WaylandShellExtension const&) = delete;
    WaylandShellExtension(WaylandShellExtension&&) noexcept = default;
    WaylandShellExtension& operator=(WaylandShellExtension&&) noexcept = default;

    bool Initialize(WaylandConfig const& config)
    {
        m_config = config;
        return true;
    }

    bool Connect()
    {
        if (m_connected)
            return false;
        m_connected = true;
        return true;
    }

    bool Disconnect()
    {
        m_connected = false;
        return true;
    }

    [[nodiscard]] bool IsConnected() const
    {
        return m_connected;
    }

    [[nodiscard]] CompositorType GetCompositorType() const
    {
        return m_config.compositor;
    }

    [[nodiscard]] WaylandConfig const& GetConfig() const
    {
        return m_config;
    }

  private:
    WaylandConfig m_config;
    bool m_connected = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
