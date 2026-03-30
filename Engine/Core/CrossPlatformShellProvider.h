// CrossPlatformShellProvider.h — Shell Provider Entry Points
// Copyright (c) 2026 ExplorerLens Project
//
// Unified shell integration layer abstracting Windows Shell (IThumbnailProvider),
// macOS Quick Look, GNOME Nautilus, and KDE Dolphin thumbnail protocols.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class ShellProviderType : uint8_t {
    WindowsShell      = 0,  // IThumbnailProvider COM
    MacOSQuickLook    = 1,  // QLThumbnailProvider
    NautilusThumbnail = 2,  // GNOME thumbnailer spec
    DolphinThumbnail  = 3,  // KDE ThumbCreator
    None              = 255
};

enum class ThumbnailFormat : uint8_t {
    BGRA32 = 0,
    RGBA32 = 1,
    RGB24  = 2,
    PNG    = 3
};

struct ThumbnailRequest {
    std::string     filePath;
    uint32_t        requestedWidth  = 256;
    uint32_t        requestedHeight = 256;
    ThumbnailFormat outputFormat    = ThumbnailFormat::BGRA32;
};

struct ThumbnailResult {
    std::vector<uint8_t> pixelData;
    uint32_t             width     = 0;
    uint32_t             height    = 0;
    uint32_t             stride    = 0;
    ThumbnailFormat      format    = ThumbnailFormat::BGRA32;
    bool                 success   = false;
    std::string          errorMsg;
};

struct ShellProviderInfo {
    ShellProviderType type       = ShellProviderType::None;
    std::string       displayName;
    std::string       version;
    bool              registered = false;
};

using ThumbnailGenerator = std::function<ThumbnailResult(const ThumbnailRequest&)>;

class CrossPlatformShellProvider {
public:
    static CrossPlatformShellProvider& Instance() {
        static CrossPlatformShellProvider s_instance;
        return s_instance;
    }

    ShellProviderType GetNativeProviderType() const {
#ifdef _WIN32
        return ShellProviderType::WindowsShell;
#elif defined(__APPLE__)
        return ShellProviderType::MacOSQuickLook;
#elif defined(__linux__)
        return ShellProviderType::NautilusThumbnail;
#else
        return ShellProviderType::None;
#endif
    }

    bool RegisterProvider(ShellProviderType type, ThumbnailGenerator generator) {
        if (!generator) return false;

        m_providerType = type;
        m_generator    = std::move(generator);
        m_registered   = true;
        return true;
    }

    bool UnregisterProvider() {
        m_providerType = ShellProviderType::None;
        m_generator    = nullptr;
        m_registered   = false;
        return true;
    }

    ThumbnailResult GenerateThumbnail(const ThumbnailRequest& request) const {
        ThumbnailResult result;
        if (!m_registered || !m_generator) {
            result.errorMsg = "No provider registered";
            return result;
        }
        if (request.filePath.empty()) {
            result.errorMsg = "Empty file path";
            return result;
        }
        if (request.requestedWidth == 0 || request.requestedHeight == 0 ||
            request.requestedWidth > MAX_THUMB_DIM || request.requestedHeight > MAX_THUMB_DIM) {
            result.errorMsg = "Invalid thumbnail dimensions";
            return result;
        }
        return m_generator(request);
    }

    ShellProviderInfo GetProviderInfo() const {
        ShellProviderInfo info;
        info.type       = m_providerType;
        info.registered = m_registered;
        info.version    = "30.0.0";
        switch (m_providerType) {
            case ShellProviderType::WindowsShell:      info.displayName = "Windows IThumbnailProvider"; break;
            case ShellProviderType::MacOSQuickLook:    info.displayName = "macOS QLThumbnailProvider";  break;
            case ShellProviderType::NautilusThumbnail: info.displayName = "GNOME Nautilus Thumbnailer"; break;
            case ShellProviderType::DolphinThumbnail:  info.displayName = "KDE Dolphin ThumbCreator";   break;
            default:                                   info.displayName = "None";
        }
        return info;
    }

    bool IsRegistered() const { return m_registered; }

    std::vector<ShellProviderType> GetSupportedProviders() const {
        return {
            ShellProviderType::WindowsShell,
            ShellProviderType::MacOSQuickLook,
            ShellProviderType::NautilusThumbnail,
            ShellProviderType::DolphinThumbnail
        };
    }

private:
    CrossPlatformShellProvider() = default;

    ShellProviderType  m_providerType = ShellProviderType::None;
    ThumbnailGenerator m_generator;
    bool               m_registered   = false;

    static constexpr uint32_t MAX_THUMB_DIM = 4096;
};

}} // namespace ExplorerLens::Engine
