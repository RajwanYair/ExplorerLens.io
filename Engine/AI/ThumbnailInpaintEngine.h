// ThumbnailInpaintEngine.h — AI inpainting for damaged/corrupt thumbnail regions
// Copyright (c) 2026 ExplorerLens Project
//
// Detects and fills damaged/missing regions in partially-decoded thumbnails using
// on-device AI inpainting. Integrates with the DiffusionModelEngine for high-quality
// region reconstruction from surrounding context pixels.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct ThumbnailInpaintRegion
{
    uint32_t x      = 0;
    uint32_t y      = 0;
    uint32_t width  = 0;
    uint32_t height = 0;
};

struct InpaintRequest
{
    std::vector<uint8_t> pixels;
    uint32_t             imageWidth  = 0;
    uint32_t             imageHeight = 0;
    ThumbnailInpaintRegion        region;
    uint32_t             steps       = 4;
};

struct InpaintResult
{
    bool                 success    = false;
    std::vector<uint8_t> pixels;
    uint32_t             width      = 0;
    uint32_t             height     = 0;
    float                confidenceScore = 0.0f;
    std::string          errorMsg;
};

class ThumbnailInpaintEngine
{
public:
    ThumbnailInpaintEngine();
    ~ThumbnailInpaintEngine();

    ThumbnailInpaintEngine(const ThumbnailInpaintEngine&)            = delete;
    ThumbnailInpaintEngine& operator=(const ThumbnailInpaintEngine&) = delete;

    bool           Initialize();
    void           Shutdown();
    InpaintResult  Inpaint(const InpaintRequest& req);
    uint64_t       InpaintCount() const noexcept { return m_count; }
    bool           IsReady()      const noexcept { return m_initialized; }

    static ThumbnailInpaintEngine& Instance() noexcept;

private:
    bool                    m_initialized = false;
    uint64_t                m_count       = 0;
    static ThumbnailInpaintEngine s_instance;
};

}} // namespace ExplorerLens::Engine
