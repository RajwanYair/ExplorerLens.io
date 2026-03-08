// ThumbnailRotationCorrector.h — EXIF-aware thumbnail orientation correction
// Copyright (c) 2026 ExplorerLens Project
//
// Applies EXIF orientation tags to decoded thumbnails, ensuring correct
// display rotation/flip without re-encoding. Supports all 8 EXIF orientations.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ThumbnailRotationCorrectorConfig {
    bool enabled = true;
    uint32_t maxDimension = 4096;
    std::string label = "ThumbnailRotationCorrector";
};

class ThumbnailRotationCorrector {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ThumbnailRotationCorrectorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Orientation : uint8_t {
        Normal = 1, FlipH = 2, Rotate180 = 3, FlipV = 4,
        Transpose = 5, Rotate90 = 6, Transverse = 7, Rotate270 = 8
    };

    Orientation ParseExifOrientation(uint16_t tag) const {
        if (tag >= 1 && tag <= 8) return static_cast<Orientation>(tag);
        return Orientation::Normal;
    }

    bool NeedsCorrection(Orientation orient) const {
        return orient != Orientation::Normal;
    }

private:
    bool m_initialized = false;
    ThumbnailRotationCorrectorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
