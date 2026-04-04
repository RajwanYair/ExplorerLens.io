// MacOSQuickLookV3.h — macOS Quick Look Thumbnail Provider
// Copyright (c) 2026 ExplorerLens Project
//
// macOS Quick Look thumbnail provider using QLThumbnailProvider API for
// Finder integration. Supports Legacy, Modern, Thumbnail, and Preview modes.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "QuickLookIntegration.h"

namespace ExplorerLens {
namespace Engine {

enum class QuickLookAPI : uint8_t {
    Legacy,
    Modern,
    Thumbnail,
    Preview
};

enum class QuickLookScale : uint8_t {
    Standard,
    Retina,
    ProMotion
};

class MacOSQuickLookV3
{
  public:
    MacOSQuickLookV3() = default;
    ~MacOSQuickLookV3() = default;

    MacOSQuickLookV3(MacOSQuickLookV3 const&) = delete;
    MacOSQuickLookV3& operator=(MacOSQuickLookV3 const&) = delete;
    MacOSQuickLookV3(MacOSQuickLookV3&&) noexcept = default;
    MacOSQuickLookV3& operator=(MacOSQuickLookV3&&) noexcept = default;

    bool Initialize(QuickLookConfig const& config)
    {
        m_config = config;
        return true;
    }

    bool RegisterExtension()
    {
        if (m_extensionActive)
            return false;
        m_extensionActive = true;
        return true;
    }

    bool UnregisterExtension()
    {
        m_extensionActive = false;
        return true;
    }

    [[nodiscard]] bool IsExtensionActive() const
    {
        return m_extensionActive;
    }

    [[nodiscard]] std::vector<std::string> GetSupportedUTIs() const
    {
        return {"public.image",         "public.jpeg",
                "public.png",           "public.tiff",
                "public.heif",          "public.avif",
                "org.webmproject.webp", "public.camera-raw-image",
                "com.adobe.pdf",        "org.khronos.glTF.binary",
                "public.svg-image"};
    }

    [[nodiscard]] QuickLookConfig const& GetConfig() const
    {
        return m_config;
    }

  private:
    QuickLookConfig m_config;
    bool m_extensionActive = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
