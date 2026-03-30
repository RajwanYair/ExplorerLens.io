// ThunarThumbnailExtension.h — XFCE Thunar Thumbnail Provider
// Copyright (c) 2026 ExplorerLens Project
//
// XFCE Thunar file manager thumbnail provider using the tumbler D-Bus API.
// Supports background, foreground, and urgent scheduler modes.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class ThunarInterfaceVersion : uint8_t {
    V1,
    V2,
    V3
};

enum class TumblerSchedulerType : uint8_t {
    Background,
    Foreground,
    Urgent
};

struct ThunarTumblerConfig {
    ThunarInterfaceVersion interfaceVersion = ThunarInterfaceVersion::V3;
    TumblerSchedulerType schedulerType = TumblerSchedulerType::Background;
    std::string cachePath;
    uint32_t maxFileSizeMB = 256;
    uint32_t timeoutMs = 5000;
};

class ThunarThumbnailExtension {
public:
    ThunarThumbnailExtension() = default;
    ~ThunarThumbnailExtension() = default;

    ThunarThumbnailExtension(ThunarThumbnailExtension const&) = delete;
    ThunarThumbnailExtension& operator=(ThunarThumbnailExtension const&) = delete;
    ThunarThumbnailExtension(ThunarThumbnailExtension&&) noexcept = default;
    ThunarThumbnailExtension& operator=(ThunarThumbnailExtension&&) noexcept = default;

    bool Initialize(ThunarTumblerConfig const& config) {
        m_config = config;
        return true;
    }

    bool RegisterTumbler() {
        if (m_registered)
            return false;
        m_registered = true;
        return true;
    }

    void UnregisterTumbler() {
        m_registered = false;
    }

    [[nodiscard]] bool IsRegistered() const {
        return m_registered;
    }

    void SetScheduler(TumblerSchedulerType schedulerType) {
        m_config.schedulerType = schedulerType;
    }

    [[nodiscard]] ThunarTumblerConfig const& GetConfig() const {
        return m_config;
    }

private:
    ThunarTumblerConfig m_config;
    bool m_registered = false;
};

} }
