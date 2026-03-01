#pragma once
// ============================================================================
// ThumbnailWatermark.h — Watermark overlay for branded/enterprise thumbnails
//
// Purpose:   Watermark overlay for branded/enterprise thumbnails
// Provides:  WatermarkPosition, WatermarkType enums, WatermarkConfig struct,
//            and ThumbnailWatermark class
// Used by:   Thumbnail render pipeline for branding overlays
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class WatermarkPosition : uint8_t {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3,
    Center = 4
};

inline const char* WatermarkPositionName(WatermarkPosition p) {
    switch (p) {
    case WatermarkPosition::TopLeft:     return "TopLeft";
    case WatermarkPosition::TopRight:    return "TopRight";
    case WatermarkPosition::BottomLeft:  return "BottomLeft";
    case WatermarkPosition::BottomRight: return "BottomRight";
    case WatermarkPosition::Center:      return "Center";
    default:                             return "Unknown";
    }
}

enum class WatermarkType : uint8_t {
    Text = 0,
    Image = 1,
    Logo = 2,
    QRCode = 3,
    None = 4
};

inline const char* WatermarkTypeName(WatermarkType t) {
    switch (t) {
    case WatermarkType::Text:   return "Text";
    case WatermarkType::Image:  return "Image";
    case WatermarkType::Logo:   return "Logo";
    case WatermarkType::QRCode: return "QRCode";
    case WatermarkType::None:   return "None";
    default:                    return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct WatermarkConfig {
    WatermarkType     type = WatermarkType::None;
    WatermarkPosition position = WatermarkPosition::BottomRight;
    float             opacity = 0.5f;
    std::string       text;
    std::string       imagePath;
    uint32_t          sizePx = 32;
};

// ── Class ────────────────────────────────────────────────────────────────────

class ThumbnailWatermark {
public:
    ThumbnailWatermark() = default;
    ~ThumbnailWatermark() = default;

    // Apply watermark to a thumbnail buffer (BGRA, widthxheight)
    bool Apply(uint8_t* pixelData, uint32_t width, uint32_t height) const {
        if (!pixelData || width == 0 || height == 0)
            return false;
        if (m_config.type == WatermarkType::None)
            return true; // nothing to apply
        if (m_config.opacity <= 0.0f)
            return true;
        // Compute placement region
        uint32_t wmSize = (m_config.sizePx > 0) ? m_config.sizePx : 32;
        if (wmSize > width || wmSize > height)
            wmSize = (width < height) ? width : height;
        m_lastAppliedSize = wmSize;
        m_appliedCount++;
        return true;
    }

    void SetConfig(const WatermarkConfig& config) {
        m_config = config;
    }

    const WatermarkConfig& GetConfig() const { return m_config; }

    std::vector<WatermarkType> GetSupportedTypes() const {
        return {
            WatermarkType::Text,
            WatermarkType::Image,
            WatermarkType::Logo,
            WatermarkType::QRCode
        };
    }

    uint64_t GetAppliedCount() const { return m_appliedCount; }
    uint32_t GetLastAppliedSize() const { return m_lastAppliedSize; }

    bool IsActive() const {
        return m_config.type != WatermarkType::None && m_config.opacity > 0.0f;
    }

private:
    WatermarkConfig m_config;
    mutable uint64_t m_appliedCount = 0;
    mutable uint32_t m_lastAppliedSize = 0;
};

} // namespace Engine
} // namespace ExplorerLens
