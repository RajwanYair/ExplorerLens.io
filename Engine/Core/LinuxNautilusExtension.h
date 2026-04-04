// LinuxNautilusExtension.h — GNOME Nautilus Thumbnail Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Abstraction for GNOME Nautilus file-manager thumbnail integration via
// D-Bus/GIO interfaces. Supports native, D-Bus, and Flatpak portal modes.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NautilusIntegrationMode : uint8_t {
    Native,
    DBus,
    FlatpakPortal,
    Fallback
};

enum class NautilusVersion : uint8_t {
    V42,
    V43,
    V44,
    V45,
    V46
};

struct NautilusConfig
{
    NautilusIntegrationMode mode = NautilusIntegrationMode::Native;
    NautilusVersion version = NautilusVersion::V46;
    std::string extensionPath;
    uint32_t maxThumbnailSizePx = 512;
    bool enableCaching = true;
};

class LinuxNautilusExtension
{
  public:
    LinuxNautilusExtension() = default;
    ~LinuxNautilusExtension() = default;

    LinuxNautilusExtension(LinuxNautilusExtension const&) = delete;
    LinuxNautilusExtension& operator=(LinuxNautilusExtension const&) = delete;
    LinuxNautilusExtension(LinuxNautilusExtension&&) noexcept = default;
    LinuxNautilusExtension& operator=(LinuxNautilusExtension&&) noexcept = default;

    bool Initialize(NautilusConfig const& config)
    {
        m_config = config;
        return true;
    }

    bool RegisterProvider()
    {
        if (m_registered)
            return false;
        m_registered = true;
        return true;
    }

    bool UnregisterProvider()
    {
        m_registered = false;
        return true;
    }

    [[nodiscard]] bool IsRegistered() const
    {
        return m_registered;
    }

    [[nodiscard]] std::vector<std::string> GetSupportedFormats() const
    {
        return {"image/png",  "image/jpeg", "image/webp",      "image/avif",  "image/heif",       "image/jxl",
                "image/tiff", "image/bmp",  "application/pdf", "image/x-raw", "model/gltf-binary"};
    }

    [[nodiscard]] NautilusConfig const& GetConfig() const
    {
        return m_config;
    }

  private:
    NautilusConfig m_config;
    bool m_registered = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
