// SmartFileTypeRouter.h — Intelligent File Type Routing Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Routes files to optimal decoders based on magic bytes, extension mapping,
// and historical decode performance. Maintains a routing table that adapts
// based on decoder success rates. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RouteConfidence : uint8_t {
    Unknown,
    ExtensionOnly,
    MagicBytes,
    FullHeader,
    Verified
};

struct FileTypeRoute
{
    std::wstring extension;
    std::wstring primaryDecoder;
    std::wstring fallbackDecoder;
    RouteConfidence confidence = RouteConfidence::Unknown;
    float avgDecodeTimeMs = 0.0f;
    uint32_t successCount = 0;
    uint32_t failureCount = 0;
};

struct FileTypeRoutingResult
{
    std::wstring selectedDecoder;
    RouteConfidence confidence = RouteConfidence::Unknown;
    float estimatedDecodeMs = 0.0f;
    bool hasFallback = false;
    std::wstring fallbackDecoder;
};

struct FileTypeRouterStats
{
    uint64_t totalRoutingDecisions = 0;
    uint64_t extensionMatches = 0;
    uint64_t fallbacksUsed = 0;
    uint64_t unknownFormats = 0;
    uint32_t registeredRoutes = 0;
    bool initialized = false;
};

class SmartFileTypeRouter
{
  public:
    static SmartFileTypeRouter& Instance()
    {
        static SmartFileTypeRouter instance;
        return instance;
    }

    void Initialize()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_routes.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    void RegisterRoute(const std::wstring& extension, const std::wstring& primaryDecoder,
                       const std::wstring& fallbackDecoder = L"")
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        FileTypeRoute route;
        route.extension = extension;
        route.primaryDecoder = primaryDecoder;
        route.fallbackDecoder = fallbackDecoder;
        route.confidence = RouteConfidence::ExtensionOnly;
        m_routes[extension] = route;
        m_stats.registeredRoutes = static_cast<uint32_t>(m_routes.size());
    }

    FileTypeRoutingResult Route(const std::wstring& extension)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalRoutingDecisions++;

        FileTypeRoutingResult decision;
        auto it = m_routes.find(extension);
        if (it != m_routes.end()) {
            decision.selectedDecoder = it->second.primaryDecoder;
            decision.confidence = it->second.confidence;
            decision.estimatedDecodeMs = it->second.avgDecodeTimeMs;
            decision.hasFallback = !it->second.fallbackDecoder.empty();
            decision.fallbackDecoder = it->second.fallbackDecoder;
            m_stats.extensionMatches++;
        } else {
            decision.selectedDecoder = L"GenericDecoder";
            decision.confidence = RouteConfidence::Unknown;
            m_stats.unknownFormats++;
        }

        return decision;
    }

    void RecordResult(const std::wstring& extension, bool success, float decodeMs)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_routes.find(extension);
        if (it != m_routes.end()) {
            if (success) {
                it->second.successCount++;
                float n = static_cast<float>(it->second.successCount);
                it->second.avgDecodeTimeMs = it->second.avgDecodeTimeMs * ((n - 1.0f) / n) + decodeMs / n;
            } else {
                it->second.failureCount++;
                m_stats.fallbacksUsed++;
            }
        }
    }

    uint32_t GetRegisteredRouteCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.registeredRoutes;
    }

    bool IsInitialized() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    FileTypeRouterStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_routes.clear();
    }

  private:
    SmartFileTypeRouter() = default;
    ~SmartFileTypeRouter() = default;
    SmartFileTypeRouter(const SmartFileTypeRouter&) = delete;
    SmartFileTypeRouter& operator=(const SmartFileTypeRouter&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, FileTypeRoute> m_routes;
    FileTypeRouterStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
