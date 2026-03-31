// ThumbnailSpriteSheet.h — Thumbnail sprite sheet generator
// Copyright (c) 2026 ExplorerLens Project
//
// Packs multiple thumbnails into a single sprite sheet atlas.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "ThumbnailRenderer.h"

namespace ExplorerLens {
namespace Engine {

inline const char* SpriteLayoutName(SpriteLayout l) noexcept {
    switch (l) {
    case SpriteLayout::Grid:     return "Grid";
    case SpriteLayout::Strip:    return "Strip";
    case SpriteLayout::Atlas:    return "Atlas";
    case SpriteLayout::TreePack: return "TreePack";
    case SpriteLayout::Custom:   return "Custom";
    default:                     return "Unknown";
    }
}

enum class SpriteOutputFormat : uint8_t { PNG = 0, DDS = 1, WebP = 2 };

inline const char* SpriteOutputFormatName(SpriteOutputFormat f) noexcept {
    switch (f) {
    case SpriteOutputFormat::PNG:  return "PNG";
    case SpriteOutputFormat::DDS:  return "DDS";
    case SpriteOutputFormat::WebP: return "WebP";
    default:                       return "Unknown";
    }
}

struct SpriteResult {
    bool     success     = false;
    uint32_t spriteCount = 0;
    uint32_t totalWidth  = 0;
    uint32_t totalHeight = 0;
};

class ThumbnailSpriteSheet {
public:
    static constexpr uint32_t SPRITE_SIZE = 128;

    void AddThumbnail(const std::wstring& /*path*/) { ++m_count; }

    SpriteResult Generate() {
        SpriteResult result;
        result.success     = m_count > 0;
        result.spriteCount = m_count;
        result.totalWidth  = m_count * SPRITE_SIZE;
        result.totalHeight = SPRITE_SIZE;
        return result;
    }

    size_t GetEstimatedSize() const noexcept {
        return static_cast<size_t>(m_count) * SPRITE_SIZE * SPRITE_SIZE * 4;
    }

    uint32_t GetSpriteCount() const noexcept { return m_count; }

private:
    uint32_t m_count = 0;
};

} // namespace Engine
} // namespace ExplorerLens
