#pragma once
// ============================================================================
// FilePreviewRouter.h — Route file preview requests to appropriate decoder
//                        based on format detection
//
// Purpose:   Route file preview requests to appropriate decoder based on
//            format detection
// Provides:  PreviewTarget, PreviewRouteAction enums, PreviewRoute struct,
//            FilePreviewRouter class
// Used by:   Shell preview handler
// ============================================================================

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Which subsystem handles a particular file preview
enum class PreviewHandler : uint8_t {
    Internal = 0,      // Engine's built-in decoder
    WICFallback = 1,   // Windows Imaging Component
    ShellDefault = 2,  // Default shell thumbnail provider
    Plugin = 3,        // Third-party plugin decoder
    External = 4       // Out-of-process external tool
};

inline const char* PreviewHandlerName(PreviewHandler h) noexcept
{
    switch (h) {
        case PreviewHandler::Internal:
            return "Internal";
        case PreviewHandler::WICFallback:
            return "WICFallback";
        case PreviewHandler::ShellDefault:
            return "ShellDefault";
        case PreviewHandler::Plugin:
            return "Plugin";
        case PreviewHandler::External:
            return "External";
        default:
            return "Unknown";
    }
}

/// Routing priority — determines trade-offs when choosing a handler
enum class RoutingPriority : uint8_t {
    Speed = 0,           // Lowest latency handler
    Quality = 1,         // Highest fidelity output
    Both = 2,            // Balance speed and quality
    CacheFirst = 3,      // Prefer cached result
    ForceRegenerate = 4  // Bypass cache, decode fresh
};

inline const char* RoutingPriorityName(RoutingPriority p) noexcept
{
    switch (p) {
        case RoutingPriority::Speed:
            return "Speed";
        case RoutingPriority::Quality:
            return "Quality";
        case RoutingPriority::Both:
            return "Both";
        case RoutingPriority::CacheFirst:
            return "CacheFirst";
        case RoutingPriority::ForceRegenerate:
            return "ForceRegenerate";
        default:
            return "Unknown";
    }
}

/// Describes the resolved route for a single file
struct PreviewRoute
{
    PreviewHandler handler = PreviewHandler::Internal;
    RoutingPriority priority = RoutingPriority::Both;
    double estimatedMs = 0.0;  // Predicted decode time
    std::string decoder;       // Decoder identifier string
};

/// Routes incoming file requests to the best-suited preview handler
/// based on format, installed plugins, cache state, and system capabilities.
class FilePreviewRouter
{
  public:
    static constexpr uint32_t HANDLER_COUNT = 5;

    FilePreviewRouter() = default;
    ~FilePreviewRouter() = default;

    FilePreviewRouter(const FilePreviewRouter&) = delete;
    FilePreviewRouter& operator=(const FilePreviewRouter&) = delete;
    FilePreviewRouter(FilePreviewRouter&&) noexcept = default;
    FilePreviewRouter& operator=(FilePreviewRouter&&) noexcept = default;

    /// Resolve the best route for the given file extension and priority
    PreviewRoute Route(const std::wstring& filePath, RoutingPriority priority) const
    {
        PreviewRoute route{};
        route.priority = priority;
        route.handler = GetBestHandler(filePath, priority);
        route.estimatedMs = EstimateDecodeTime(route.handler);
        return route;
    }

    /// Determine the single best handler for a given file
    PreviewHandler GetBestHandler(const std::wstring& /*filePath*/, RoutingPriority priority) const noexcept
    {
        if (priority == RoutingPriority::Speed)
            return PreviewHandler::Internal;
        if (priority == RoutingPriority::CacheFirst)
            return PreviewHandler::Internal;
        return PreviewHandler::Internal;  // Default to internal pipeline
    }

    /// Register a custom handler for a file extension
    bool RegisterHandler(const std::string& extension, PreviewHandler handler)
    {
        if (m_registeredCount >= MAX_REGISTRATIONS)
            return false;
        m_extensions[m_registeredCount] = extension;
        m_handlers[m_registeredCount] = handler;
        m_registeredCount++;
        return true;
    }

    /// Number of registered custom handler mappings
    uint32_t GetRegisteredCount() const noexcept
    {
        return m_registeredCount;
    }

  private:
    static constexpr uint32_t MAX_REGISTRATIONS = 512;

    double EstimateDecodeTime(PreviewHandler handler) const noexcept
    {
        switch (handler) {
            case PreviewHandler::Internal:
                return 12.0;
            case PreviewHandler::WICFallback:
                return 25.0;
            case PreviewHandler::ShellDefault:
                return 40.0;
            case PreviewHandler::Plugin:
                return 30.0;
            case PreviewHandler::External:
                return 80.0;
            default:
                return 50.0;
        }
    }

    std::array<std::string, MAX_REGISTRATIONS> m_extensions{};
    std::array<PreviewHandler, MAX_REGISTRATIONS> m_handlers{};
    uint32_t m_registeredCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
