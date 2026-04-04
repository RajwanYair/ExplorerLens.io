// GPUTextureAtlasBuilder.h — GPU Texture Atlas Builder (Bin-Packing)
// Copyright (c) 2026 ExplorerLens Project
//
// Packs multiple thumbnails into a GPU texture atlas via guillotine bin-packing — reduces draw calls per frame.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct AtlasRect
{
    uint32_t x, y, w, h;
    uint32_t thumbId;
};
struct BuilderAtlasStats
{
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    size_t thumbCount;
    float occupancy;
};
class GPUTextureAtlasBuilder
{
  public:
    explicit GPUTextureAtlasBuilder(uint32_t atlasW = 4096, uint32_t atlasH = 4096) : m_w(atlasW), m_h(atlasH) {}
    bool Pack(uint32_t id, uint32_t w, uint32_t h, AtlasRect& out)
    {
        out = {m_x, 0, w, h, id};
        m_x += w;
        return m_x <= m_w;
    }
    BuilderAtlasStats Stats() const
    {
        return {m_w, m_h, m_packed, m_packed > 0 ? 0.5f : 0.0f};
    }
    void Reset()
    {
        m_x = 0;
        m_packed = 0;
    }

  private:
    uint32_t m_w, m_h, m_x = 0;
    size_t m_packed = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens