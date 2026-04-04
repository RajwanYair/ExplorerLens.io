// KDEDolphinExtension.h — KDE Dolphin Thumbnail Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Integration layer for KDE Dolphin file manager thumbnails via KIO/Solid
// frameworks. Supports ThumbCreator, KIOSlave, and PreviewPlugin types.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DolphinPluginType : uint8_t {
    ThumbCreator,
    KIOSlave,
    PreviewPlugin
};

enum class DolphinPriority : uint8_t {
    Low,
    Normal,
    High,
    Realtime
};

struct DolphinPluginConfig
{
    DolphinPluginType pluginType = DolphinPluginType::ThumbCreator;
    DolphinPriority priority = DolphinPriority::Normal;
    std::string serviceName;
    std::vector<std::string> mimeTypes;
    uint32_t maxConcurrentJobs = 4;
};

class KDEDolphinExtension
{
  public:
    KDEDolphinExtension() = default;
    ~KDEDolphinExtension() = default;

    KDEDolphinExtension(KDEDolphinExtension const&) = delete;
    KDEDolphinExtension& operator=(KDEDolphinExtension const&) = delete;
    KDEDolphinExtension(KDEDolphinExtension&&) noexcept = default;
    KDEDolphinExtension& operator=(KDEDolphinExtension&&) noexcept = default;

    bool Initialize(DolphinPluginConfig const& config)
    {
        m_config = config;
        return true;
    }

    bool RegisterThumbCreator()
    {
        if (m_active)
            return false;
        m_active = true;
        return true;
    }

    bool UnregisterThumbCreator()
    {
        m_active = false;
        return true;
    }

    [[nodiscard]] bool IsActive() const
    {
        return m_active;
    }

    bool SetPriority(DolphinPriority priority)
    {
        m_config.priority = priority;
        return true;
    }

    [[nodiscard]] DolphinPluginConfig const& GetConfig() const
    {
        return m_config;
    }

  private:
    DolphinPluginConfig m_config;
    bool m_active = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
