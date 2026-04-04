// LiveThumbnailPoller.h — Live Thumbnail Poller
// Copyright (c) 2026 ExplorerLens Project
//
// Periodically refreshes thumbnails for live sources with configurable polling interval.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LTPConfig
{
    uint32_t pollIntervalMs = 10000;
    uint32_t maxConcurrent = 4;
    bool autoRetry = true;
};

using LTPUpdateCallback = std::function<void(const std::string& sourceId, const std::vector<uint8_t>& rgba)>;

struct LTPSourceInfo
{
    std::string url;
    std::string sourceId;
    uint64_t lastPollMs = 0;
    bool active = false;
};

class LiveThumbnailPoller
{
  public:
    explicit LiveThumbnailPoller(const LTPConfig& config) : m_config(config) {}

    bool AddSource(const LTPSourceInfo& info)
    {
        if (m_sources.size() >= m_config.maxConcurrent)
            return false;
        m_sources[info.sourceId] = info;
        return true;
    }
    bool RemoveSource(const std::string& sourceId)
    {
        return m_sources.erase(sourceId) > 0;
    }
    void SetCallback(LTPUpdateCallback cb)
    {
        m_callback = std::move(cb);
    }
    uint32_t ActiveSourceCount() const
    {
        uint32_t n = 0;
        for (const auto& [k, s] : m_sources)
            if (s.active)
                ++n;
        return n;
    }

  private:
    LTPConfig m_config;
    std::unordered_map<std::string, LTPSourceInfo> m_sources;
    LTPUpdateCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
