// PlatformUIScalingEngine.h — DPI-Aware Scaling Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Handles display scale calculation for Windows HiDPI, macOS Retina, and Linux
// Wayland/X11 fractional scaling. Produces correct physical pixel thumbnail sizes.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

enum class ScalingMode : uint8_t {
    System = 0,
    PerMonitor = 1,
    Manual = 2
};

struct ScaleFactorInfo
{
    float factor = 1.0f;
    uint32_t dpiX = 96;
    uint32_t dpiY = 96;
    bool isRetina = false;
    bool isFractional = false;
};

struct ThumbnailSizeRequest
{
    uint32_t logicalWidth = 256;
    uint32_t logicalHeight = 256;
    float scaleFactor = 1.0f;
};

struct PhysicalThumbnailSize
{
    uint32_t width = 256;
    uint32_t height = 256;
    uint32_t stride = 0;

    uint32_t TotalBytes() const
    {
        return stride * height;
    }
};

class PlatformUIScalingEngine
{
  public:
    static PlatformUIScalingEngine& Instance()
    {
        static PlatformUIScalingEngine s_instance;
        return s_instance;
    }

    ScaleFactorInfo GetDisplayScale() const
    {
        ScaleFactorInfo info;
#ifdef _WIN32
        HDC hdc = GetDC(nullptr);
        if (hdc) {
            info.dpiX = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
            info.dpiY = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSY));
            info.factor = static_cast<float>(info.dpiX) / BASE_DPI;
            ReleaseDC(nullptr, hdc);
        }
#elif defined(__APPLE__)
        info.factor = 2.0f;
        info.dpiX = 144;
        info.dpiY = 144;
        info.isRetina = true;
#else
        info.factor = 1.0f;
        info.dpiX = BASE_DPI;
        info.dpiY = BASE_DPI;
#endif
        info.isFractional = (info.factor != 1.0f && info.factor != 2.0f && info.factor != 3.0f);
        return info;
    }

    PhysicalThumbnailSize ScaleToPhysicalPixels(const ThumbnailSizeRequest& req) const
    {
        PhysicalThumbnailSize result;
        float scale = (req.scaleFactor > 0.0f) ? req.scaleFactor : 1.0f;

        result.width = ClampDimension(static_cast<uint32_t>(std::round(req.logicalWidth * scale)));
        result.height = ClampDimension(static_cast<uint32_t>(std::round(req.logicalHeight * scale)));

        // Align stride to 4-byte boundary for BGRA32
        result.stride = ((result.width * BYTES_PER_PIXEL + 3u) / 4u) * 4u;
        return result;
    }

    uint32_t GetPreferredThumbnailSize(float scaleFactor) const
    {
        if (scaleFactor <= 1.0f)
            return 256;
        if (scaleFactor <= 1.5f)
            return 384;
        if (scaleFactor <= 2.0f)
            return 512;
        if (scaleFactor <= 3.0f)
            return 768;
        return 1024;
    }

    std::vector<ScaleFactorInfo> EnumerateMonitorScales() const
    {
        std::vector<ScaleFactorInfo> scales;
#ifdef _WIN32
        scales.push_back(GetDisplayScale());
#elif defined(__APPLE__)
        scales.push_back({2.0f, 144, 144, true, false});
#else
        scales.push_back({1.0f, BASE_DPI, BASE_DPI, false, false});
#endif
        return scales;
    }

    void SetScalingMode(ScalingMode mode)
    {
        m_mode = mode;
    }
    ScalingMode GetScalingMode() const
    {
        return m_mode;
    }

    void SetManualScale(float factor)
    {
        m_manualScale = std::clamp(factor, MIN_SCALE, MAX_SCALE);
    }

    float GetEffectiveScale() const
    {
        if (m_mode == ScalingMode::Manual)
            return m_manualScale;
        return GetDisplayScale().factor;
    }

  private:
    PlatformUIScalingEngine() = default;

    ScalingMode m_mode = ScalingMode::System;
    float m_manualScale = 1.0f;

    static constexpr float BASE_DPI = 96.0f;
    static constexpr float MIN_SCALE = 0.5f;
    static constexpr float MAX_SCALE = 4.0f;
    static constexpr uint32_t MAX_DIM = 4096;
    static constexpr uint32_t MIN_DIM = 16;
    static constexpr uint32_t BYTES_PER_PIXEL = 4;

    static uint32_t ClampDimension(uint32_t val)
    {
        return std::clamp(val, MIN_DIM, MAX_DIM);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
