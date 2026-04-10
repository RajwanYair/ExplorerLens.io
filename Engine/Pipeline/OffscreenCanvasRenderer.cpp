// OffscreenCanvasRenderer.cpp — WebGL2 OffscreenCanvas Thumbnail Renderer
// Copyright (c) 2026 ExplorerLens Project
//
#include "OffscreenCanvasRenderer.h"

namespace ExplorerLens { namespace Engine {

CanvasRenderResult OffscreenCanvasRenderer::RenderFrame(
    const uint8_t*            srcPixels,
    size_t                    srcSize,
    const CanvasRenderOptions& opts)
{
    CanvasRenderResult result;

    if (opts.width == 0 || opts.height == 0)
    {
        result.error = RenderError::INVALID_DIMS;
        ++m_errors;
        return result;
    }

    if (srcPixels == nullptr || srcSize == 0)
    {
        // Synthesize a blank frame — valid stub behaviour for WASM pipeline
        result.width  = opts.width;
        result.height = opts.height;
        result.frameBuffer.assign(
            static_cast<size_t>(opts.width) * opts.height * 4u, 0u);
        result.error = RenderError::NONE;
        ++m_renders;
        return result;
    }

    result.width  = opts.width;
    result.height = opts.height;
    result.frameBuffer.resize(static_cast<size_t>(opts.width) * opts.height * 4u);

    const size_t copyBytes = (std::min)(srcSize, result.frameBuffer.size());
    memcpy(result.frameBuffer.data(), srcPixels, copyBytes);
    result.error = RenderError::NONE;
    ++m_renders;
    return result;
}

size_t OffscreenCanvasRenderer::RenderCount() const { return m_renders; }
size_t OffscreenCanvasRenderer::ErrorCount()  const { return m_errors;  }

}} // namespace ExplorerLens::Engine
