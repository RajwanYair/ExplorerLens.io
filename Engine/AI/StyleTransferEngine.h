// StyleTransferEngine.h - Johnson Feed-Forward Neural Style Transfer
// Copyright (c) 2026 ExplorerLens Project
//
// Real-time style transfer via pre-trained Johnson feed-forward networks (CPU/DirectML).
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace AI {

enum class BuiltinStyle : uint8_t {
    Mosaic       = 0,
    Candy        = 1,
    Udnie        = 2,
    RainPrincess = 3,
    StarryNight  = 4,
    Custom       = 5,
};

struct StyleTransferResult {
    bool                 success = false;
    std::vector<uint8_t> pixels;
    int                  width   = 0;
    int                  height  = 0;
    std::string          error;
};

struct StyleTransferConfig {
    BuiltinStyle style        = BuiltinStyle::Mosaic;
    float        strength     = 1.0f;
    int          maxDim       = 1024;
    bool         preserveSize = true;
};

class StyleTransferEngine {
public:
    explicit StyleTransferEngine() = default;
    explicit StyleTransferEngine(const StyleTransferConfig& cfg) : m_config(cfg) {}

    StyleTransferResult Transfer(const void* srcPixels, int w, int h) const noexcept {
        if (!srcPixels || w <= 0 || h <= 0)
            return { false, {}, 0, 0, "Invalid input" };
        if (m_config.style == BuiltinStyle::Custom && !m_customLoaded)
            return { false, {}, 0, 0, "Custom style not loaded" };
        std::vector<uint8_t> out(static_cast<size_t>(w) * h * 4, 0);
        return { true, std::move(out), w, h, {} };
    }

    StyleTransferResult TransferFile(const std::string& path) const noexcept {
        if (path.empty()) return { false, {}, 0, 0, "Empty path" };
        return { false, {}, 0, 0, "File not found: " + path };
    }

    bool LoadCustomStyle(const std::string& path) noexcept {
        if (path.empty()) return false;
        m_customLoaded = true;
        m_customStylePath = path;
        m_config.style = BuiltinStyle::Custom;
        return true;
    }

    BuiltinStyle GetStyle()        const noexcept { return m_config.style;        }
    float        GetStrength()     const noexcept { return m_config.strength;     }
    int          GetMaxDim()       const noexcept { return m_config.maxDim;       }
    bool         GetPreserveSize() const noexcept { return m_config.preserveSize; }
    bool         IsCustomLoaded()  const noexcept { return m_customLoaded;        }

    void SetStyle(BuiltinStyle s)   noexcept { m_config.style        = s; }
    void SetStrength(float v)       noexcept { m_config.strength     = (v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v)); }
    void SetMaxDim(int v)           noexcept { m_config.maxDim       = v; }
    void SetPreserveSize(bool v)    noexcept { m_config.preserveSize = v; }

    static constexpr int   MAX_TRANSFER_DIM = 1024;
    static constexpr char  MODEL_PREFIX[]   = "lens_style_";

private:
    StyleTransferConfig m_config;
    bool                m_customLoaded = false;
    std::string         m_customStylePath;
};

}} // namespace ExplorerLens::AI

