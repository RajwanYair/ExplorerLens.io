// APNGFrameCombiner.cpp — Animated PNG Frame Compositor
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/APNGFrameCombiner.h"
#include <cstring>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

// BGRA composite: apply srcOver alpha blending (Over mode).
static void CompositeOver(uint8_t* dst, const uint8_t* src) noexcept
{
    const float sa = src[3] / 255.0f;
    const float da = dst[3] / 255.0f;
    const float oa = sa + da * (1.0f - sa);
    if (oa < 1e-6f) { dst[0] = dst[1] = dst[2] = dst[3] = 0; return; }
    for (int c = 0; c < 3; ++c) {
        dst[c] = static_cast<uint8_t>(
            (src[c] * sa + dst[c] * da * (1.0f - sa)) / oa + 0.5f);
    }
    dst[3] = static_cast<uint8_t>(oa * 255.0f + 0.5f);
}

APNGCompositeResult APNGFrameCombiner::Composite(
    const std::vector<APNGRawFrame>& rawFrames,
    uint32_t canvasWidth, uint32_t canvasHeight) const noexcept
{
    APNGCompositeResult result{};
    if (rawFrames.empty() || canvasWidth == 0 || canvasHeight == 0) return result;

    const size_t canvasBytes = static_cast<size_t>(canvasWidth) * canvasHeight * 4u;
    std::vector<uint8_t> canvas(canvasBytes, 0x00);
    std::vector<uint8_t> prevCanvas(canvasBytes, 0x00);

    result.width      = canvasWidth;
    result.height     = canvasHeight;
    result.frameCount = static_cast<uint32_t>(rawFrames.size());

    for (const auto& f : rawFrames) {
        if (!f.rgba || f.width == 0 || f.height == 0) continue;

        // Save previous canvas for Dispose::Previous.
        std::memcpy(prevCanvas.data(), canvas.data(), canvasBytes);

        // Blit this frame onto the canvas.
        for (uint32_t row = 0; row < f.height; ++row) {
            const uint32_t canvasRow = f.yOffset + row;
            if (canvasRow >= canvasHeight) break;
            for (uint32_t col = 0; col < f.width; ++col) {
                const uint32_t canvasCol = f.xOffset + col;
                if (canvasCol >= canvasWidth) break;
                const size_t dstOff = (canvasRow * canvasWidth + canvasCol) * 4u;
                const size_t srcOff = (row * f.width + col) * 4u;
                if (f.blend == APNGBlend::Over) {
                    CompositeOver(canvas.data() + dstOff, f.rgba + srcOff);
                } else {
                    std::memcpy(canvas.data() + dstOff, f.rgba + srcOff, 4);
                }
            }
        }

        // Save composite frame output.
        result.framesRGBA.push_back(canvas);

        // Apply disposal for next frame.
        if (f.dispose == APNGDispose::Background) {
            for (uint32_t row = 0; row < f.height && f.yOffset + row < canvasHeight; ++row)
                for (uint32_t col = 0; col < f.width && f.xOffset + col < canvasWidth; ++col) {
                    const size_t off = ((f.yOffset + row) * canvasWidth + (f.xOffset + col)) * 4u;
                    canvas[off] = canvas[off+1] = canvas[off+2] = canvas[off+3] = 0;
                }
        } else if (f.dispose == APNGDispose::Previous) {
            std::memcpy(canvas.data(), prevCanvas.data(), canvasBytes);
        }
    }

    result.success = !result.framesRGBA.empty();
    return result;
}

std::vector<uint32_t> APNGFrameCombiner::SelectKeyFrameIndices(
    uint32_t totalFrames, uint32_t maxFrames) noexcept
{
    std::vector<uint32_t> indices;
    if (totalFrames == 0 || maxFrames == 0) return indices;
    const uint32_t step = (maxFrames >= totalFrames) ? 1u : totalFrames / maxFrames;
    for (uint32_t i = 0; i < totalFrames && indices.size() < maxFrames; i += step)
        indices.push_back(i);
    return indices;
}

uint32_t APNGFrameCombiner::ProbeFrameCount(
    const uint8_t* pngData, size_t pngSize) noexcept
{
    // Count acTL chunk which carries animation frame count.
    if (!pngData || pngSize < 8) return 1;
    static const uint8_t pngMagic[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    if (std::memcmp(pngData, pngMagic, 8) != 0) return 0;

    size_t pos = 8;
    while (pos + 12 <= pngSize) {
        const uint32_t chunkLen = (static_cast<uint32_t>(pngData[pos])   << 24u)
                                | (static_cast<uint32_t>(pngData[pos+1]) << 16u)
                                | (static_cast<uint32_t>(pngData[pos+2]) <<  8u)
                                |  static_cast<uint32_t>(pngData[pos+3]);
        const char* type = reinterpret_cast<const char*>(pngData + pos + 4);
        if (std::memcmp(type, "acTL", 4) == 0 && chunkLen >= 4) {
            // acTL: num_frames (4 bytes) at offset pos + 8
            return (static_cast<uint32_t>(pngData[pos+8])  << 24u)
                 | (static_cast<uint32_t>(pngData[pos+9])  << 16u)
                 | (static_cast<uint32_t>(pngData[pos+10]) <<  8u)
                 |  static_cast<uint32_t>(pngData[pos+11]);
        }
        if (std::memcmp(type, "IEND", 4) == 0) break;
        pos += 12 + chunkLen;
    }
    return 1;  // Non-animated PNG = 1 frame
}

}} // namespace ExplorerLens::Engine
