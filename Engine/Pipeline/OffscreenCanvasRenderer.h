// OffscreenCanvasRenderer.h — WebGL2 OffscreenCanvas Thumbnail Renderer
// Copyright (c) 2026 ExplorerLens Project
//
// Renders decoded thumbnail pixel data into a WebGL2 OffscreenCanvas target,
// enabling in-browser thumbnail display with GPU acceleration without a
// visible DOM canvas element.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class RenderError : uint32_t
{
    NONE             = 0,
    NULL_INPUT       = 1,
    INVALID_DIMS     = 2,
    CONTEXT_LOST     = 3,
    UNSUPPORTED_FMT  = 4
};

struct CanvasRenderOptions
{
    uint32_t    width  = 0;
    uint32_t    height = 0;
    std::string format;    // "rgba8", "rgb8", "bgra8"
    bool        yFlip  = false;
};

struct CanvasRenderResult
{
    uint32_t             width       = 0;
    uint32_t             height      = 0;
    std::vector<uint8_t> frameBuffer;
    RenderError          error       = RenderError::NONE;
};

class OffscreenCanvasRenderer
{
public:
    OffscreenCanvasRenderer()  = default;
    ~OffscreenCanvasRenderer() = default;

    OffscreenCanvasRenderer(const OffscreenCanvasRenderer&)            = delete;
    OffscreenCanvasRenderer& operator=(const OffscreenCanvasRenderer&) = delete;
    OffscreenCanvasRenderer(OffscreenCanvasRenderer&&)                 = default;
    OffscreenCanvasRenderer& operator=(OffscreenCanvasRenderer&&)      = default;

    CanvasRenderResult RenderFrame(const uint8_t*           srcPixels,
                                   size_t                   srcSize,
                                   const CanvasRenderOptions& opts);

    size_t RenderCount() const;
    size_t ErrorCount()  const;

private:
    size_t m_renders = 0u;
    size_t m_errors  = 0u;
};

}} // namespace ExplorerLens::Engine
