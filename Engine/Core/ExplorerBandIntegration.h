#pragma once

#include <string>
#include <cstdint>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// ExplorerBandIntegration — Explorer toolbar band object
// ============================================================================

enum class BandPosition {
    Top,
    Bottom,
    Left,
    Right,
    Float
};

inline const char* BandPositionName(BandPosition value) {
    switch (value) {
    case BandPosition::Top:    return "Top";
    case BandPosition::Bottom: return "Bottom";
    case BandPosition::Left:   return "Left";
    case BandPosition::Right:  return "Right";
    case BandPosition::Float:  return "Float";
    default:                   return "Unknown";
    }
}

enum class BandState {
    Hidden,
    Visible,
    Docked,
    Floating,
    Minimized
};

inline const char* BandStateName(BandState value) {
    switch (value) {
    case BandState::Hidden:    return "Hidden";
    case BandState::Visible:   return "Visible";
    case BandState::Docked:    return "Docked";
    case BandState::Floating:  return "Floating";
    case BandState::Minimized: return "Minimized";
    default:                   return "Unknown";
    }
}

struct BandConfig {
    BandPosition position = BandPosition::Top;
    uint32_t     width = 300;
    uint32_t     height = 80;
    std::wstring title = L"ExplorerLens";
    bool         showThumbnailPreview = true;
    bool         autoHide = false;
    uint32_t     opacity = 255;   // 0-255
    uint32_t     maxPreviewItems = 5;

    bool IsValid() const {
        return width > 0 && height > 0 && width <= 4096 && height <= 4096;
    }
};

struct BandRegistrationInfo {
    std::wstring clsid;
    std::wstring progId;
    std::wstring description;
    bool         isRegistered = false;
    uint32_t     bandId = 0;
};

class ExplorerBandIntegration {
public:
    static constexpr uint32_t MIN_BAND_WIDTH = 100;
    static constexpr uint32_t MIN_BAND_HEIGHT = 40;
    static constexpr uint32_t MAX_BAND_WIDTH = 2048;
    static constexpr uint32_t MAX_BAND_HEIGHT = 1024;
    static constexpr uint32_t BAND_VERSION = 1;

    ExplorerBandIntegration() = default;
    ~ExplorerBandIntegration() = default;

    ExplorerBandIntegration(const ExplorerBandIntegration&) = delete;
    ExplorerBandIntegration& operator=(const ExplorerBandIntegration&) = delete;

    bool Register(const BandConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (!config.IsValid()) {
            return false;
        }

        m_config = config;

        // Clamp dimensions
        m_config.width = ClampValue(m_config.width, MIN_BAND_WIDTH, MAX_BAND_WIDTH);
        m_config.height = ClampValue(m_config.height, MIN_BAND_HEIGHT, MAX_BAND_HEIGHT);

        m_registration.isRegistered = true;
        m_registration.description = L"ExplorerLens Thumbnail Band";
        m_registration.bandId = BAND_VERSION;
        m_state = BandState::Hidden;
        return true;
    }

    bool Unregister() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_registration.isRegistered) {
            return false;
        }
        m_registration.isRegistered = false;
        m_state = BandState::Hidden;
        return true;
    }

    BandState GetState() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_state;
    }

    bool SetVisibility(bool visible) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_registration.isRegistered) {
            return false;
        }

        if (visible) {
            m_state = (m_config.position == BandPosition::Float)
                ? BandState::Floating : BandState::Docked;
        }
        else {
            m_state = BandState::Hidden;
        }
        return true;
    }

    const BandConfig& GetConfig() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_config;
    }

    bool IsRegistered() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_registration.isRegistered;
    }

    bool UpdateConfig(const BandConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_registration.isRegistered || !config.IsValid()) {
            return false;
        }
        m_config = config;
        m_config.width = ClampValue(m_config.width, MIN_BAND_WIDTH, MAX_BAND_WIDTH);
        m_config.height = ClampValue(m_config.height, MIN_BAND_HEIGHT, MAX_BAND_HEIGHT);
        return true;
    }

private:
    static uint32_t ClampValue(uint32_t val, uint32_t lo, uint32_t hi) {
        return (val < lo) ? lo : (val > hi) ? hi : val;
    }

    mutable std::mutex       m_mutex;
    BandConfig               m_config;
    BandRegistrationInfo     m_registration;
    BandState                m_state = BandState::Hidden;
};

} // namespace Engine
} // namespace ExplorerLens
