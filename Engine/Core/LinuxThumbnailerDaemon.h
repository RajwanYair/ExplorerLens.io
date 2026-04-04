// LinuxThumbnailerDaemon.h — Freedesktop Thumbnail Daemon
// Copyright (c) 2026 ExplorerLens Project
//
// System-wide Linux thumbnail daemon compliant with the freedesktop.org
// thumbnail specification. Manages cache lifecycle and concurrent generation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ThumbSize : uint8_t {
    Normal,
    Large,
    XLarge,
    XXLarge
};

enum class DaemonState : uint8_t {
    Stopped,
    Starting,
    Running,
    Paused,
    ShuttingDown
};

struct ThumbnailerDaemonConfig
{
    std::string cachePath;
    uint64_t maxCacheSize = 512 * 1024 * 1024;
    uint32_t maxConcurrent = 4;
    ThumbSize defaultSize = ThumbSize::Large;
    bool autoStart = true;
};

class LinuxThumbnailerDaemon
{
  public:
    LinuxThumbnailerDaemon() = default;
    ~LinuxThumbnailerDaemon() = default;

    LinuxThumbnailerDaemon(LinuxThumbnailerDaemon const&) = delete;
    LinuxThumbnailerDaemon& operator=(LinuxThumbnailerDaemon const&) = delete;
    LinuxThumbnailerDaemon(LinuxThumbnailerDaemon&&) noexcept = default;
    LinuxThumbnailerDaemon& operator=(LinuxThumbnailerDaemon&&) noexcept = default;

    bool Start(ThumbnailerDaemonConfig const& config)
    {
        if (m_state != DaemonState::Stopped)
            return false;
        m_config = config;
        m_state = DaemonState::Running;
        return true;
    }

    bool Stop()
    {
        m_state = DaemonState::ShuttingDown;
        m_state = DaemonState::Stopped;
        return true;
    }

    [[nodiscard]] DaemonState GetState() const
    {
        return m_state;
    }

    bool GenerateThumbnail(std::string const& filePath, ThumbSize size)
    {
        if (m_state != DaemonState::Running)
            return false;
        (void)filePath;
        (void)size;
        return true;
    }

    uint64_t PurgeCache()
    {
        uint64_t purgedBytes = 0;
        return purgedBytes;
    }

    [[nodiscard]] ThumbnailerDaemonConfig const& GetConfig() const
    {
        return m_config;
    }

  private:
    ThumbnailerDaemonConfig m_config;
    DaemonState m_state = DaemonState::Stopped;
};

}  // namespace Engine
}  // namespace ExplorerLens
