// StyleTransferPreview.h — Neural Style Transfer Thumbnail Effects
// Copyright (c) 2026 ExplorerLens Project
//
// Neural style transfer for thumbnail effects. Applies artistic filters including
// oil paint, watercolor, and pencil sketch using image processing techniques.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <array>

namespace ExplorerLens {
namespace Engine {

enum class ArtStyle : uint8_t {
    OilPaint,
    Watercolor,
    PencilSketch,
    Charcoal,
    Posterize,
    Emboss,
    Vignette
};

struct StyleConfig {
    ArtStyle style = ArtStyle::OilPaint;
    float intensity = 1.0f;
    uint32_t brushRadius = 4;
    uint32_t quantizationLevels = 8;
    float edgeStrength = 1.0f;
};

struct StyledImage {
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 3;
    ArtStyle appliedStyle = ArtStyle::OilPaint;
};

class StyleTransferPreview {
public:
    static StyleTransferPreview& Instance() {
        static StyleTransferPreview instance;
        return instance;
    }

    inline StyledImage ApplyStyle(const uint8_t* pixels, uint32_t width, uint32_t height,
        uint32_t channels, const StyleConfig& config) const {
        StyledImage result;
        result.width = width;
        result.height = height;
        result.channels = 3;
        result.appliedStyle = config.style;
        size_t outSize = static_cast<size_t>(width) * height * 3;
        result.pixels.resize(outSize);

        if (!pixels || width == 0 || height == 0) return result;

        switch (config.style) {
        case ArtStyle::OilPaint:
            ApplyOilPaint(pixels, width, height, channels, result.pixels.data(), config.brushRadius);
            break;
        case ArtStyle::Watercolor:
            ApplyWatercolor(pixels, width, height, channels, result.pixels.data(), config.intensity);
            break;
        case ArtStyle::PencilSketch:
            ApplyPencilSketch(pixels, width, height, channels, result.pixels.data(), config.edgeStrength);
            break;
        case ArtStyle::Posterize:
            ApplyPosterize(pixels, width, height, channels, result.pixels.data(), config.quantizationLevels);
            break;
        case ArtStyle::Emboss:
            ApplyEmboss(pixels, width, height, channels, result.pixels.data());
            break;
        case ArtStyle::Vignette:
            ApplyVignette(pixels, width, height, channels, result.pixels.data(), config.intensity);
            break;
        default:
            for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
                for (uint32_t c = 0; c < 3 && c < channels; ++c) {
                    result.pixels[i * 3 + c] = pixels[i * channels + c];
                }
            }
            break;
        }
        return result;
    }

    inline std::string StyleToString(ArtStyle style) const {
        switch (style) {
        case ArtStyle::OilPaint:     return "Oil Paint";
        case ArtStyle::Watercolor:   return "Watercolor";
        case ArtStyle::PencilSketch: return "Pencil Sketch";
        case ArtStyle::Charcoal:     return "Charcoal";
        case ArtStyle::Posterize:    return "Posterize";
        case ArtStyle::Emboss:       return "Emboss";
        case ArtStyle::Vignette:     return "Vignette";
        default:                     return "Unknown";
        }
    }

private:
    StyleTransferPreview() = default;

    inline void ApplyOilPaint(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c,
        uint8_t* dst, uint32_t radius) const {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint32_t buckets[20] = {};
                uint32_t sumR[20] = {}, sumG[20] = {}, sumB[20] = {};
                uint32_t levels = 20;

                int r = static_cast<int>(radius);
                for (int dy = -r; dy <= r; ++dy) {
                    for (int dx = -r; dx <= r; ++dx) {
                        int sx = (std::max)(0, (std::min)(static_cast<int>(w) - 1, static_cast<int>(x) + dx));
                        int sy = (std::max)(0, (std::min)(static_cast<int>(h) - 1, static_cast<int>(y) + dy));
                        size_t idx = (sy * w + sx) * c;
                        uint8_t intensity = static_cast<uint8_t>(
                            (0.299f * src[idx] + 0.587f * src[idx + (std::min)(1u, c - 1)] +
                                0.114f * src[idx + (std::min)(2u, c - 1)]));
                        uint32_t bucket = intensity * (levels - 1) / 255;
                        buckets[bucket]++;
                        sumR[bucket] += src[idx];
                        sumG[bucket] += src[idx + (std::min)(1u, c - 1)];
                        sumB[bucket] += src[idx + (std::min)(2u, c - 1)];
                    }
                }

                uint32_t maxBucket = 0, maxCount = 0;
                for (uint32_t b = 0; b < levels; ++b) {
                    if (buckets[b] > maxCount) { maxCount = buckets[b]; maxBucket = b; }
                }

                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                if (maxCount > 0) {
                    dst[dstIdx] = static_cast<uint8_t>(sumR[maxBucket] / maxCount);
                    dst[dstIdx + 1] = static_cast<uint8_t>(sumG[maxBucket] / maxCount);
                    dst[dstIdx + 2] = static_cast<uint8_t>(sumB[maxBucket] / maxCount);
                }
            }
        }
    }

    inline void ApplyWatercolor(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c,
        uint8_t* dst, float intensity) const {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                float sumR = 0, sumG = 0, sumB = 0;
                int count = 0;
                int radius = 3;
                for (int dy = -radius; dy <= radius; ++dy) {
                    for (int dx = -radius; dx <= radius; ++dx) {
                        int sx = (std::max)(0, (std::min)(static_cast<int>(w) - 1, static_cast<int>(x) + dx));
                        int sy = (std::max)(0, (std::min)(static_cast<int>(h) - 1, static_cast<int>(y) + dy));
                        size_t idx = (sy * w + sx) * c;
                        sumR += src[idx]; sumG += src[idx + (std::min)(1u, c - 1)]; sumB += src[idx + (std::min)(2u, c - 1)];
                        ++count;
                    }
                }
                float avgR = sumR / count, avgG = sumG / count, avgB = sumB / count;

                size_t srcIdx = (y * w + x) * c;
                float r = src[srcIdx] * (1.0f - intensity) + avgR * intensity;
                float g = src[srcIdx + (std::min)(1u, c - 1)] * (1.0f - intensity) + avgG * intensity;
                float b = src[srcIdx + (std::min)(2u, c - 1)] * (1.0f - intensity) + avgB * intensity;

                float sat = 1.3f;
                float gray = 0.299f * r + 0.587f * g + 0.114f * b;
                r = gray + (r - gray) * sat;
                g = gray + (g - gray) * sat;
                b = gray + (b - gray) * sat;

                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                dst[dstIdx] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, r)));
                dst[dstIdx + 1] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, g)));
                dst[dstIdx + 2] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, b)));
            }
        }
    }

    inline void ApplyPencilSketch(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c,
        uint8_t* dst, float strength) const {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                size_t idx = (y * w + x) * c;
                float lum = 0.299f * src[idx] + 0.587f * src[idx + (std::min)(1u, c - 1)] +
                    0.114f * src[idx + (std::min)(2u, c - 1)];

                float gx = 0, gy = 0;
                if (x > 0 && x < w - 1 && y > 0 && y < h - 1) {
                    auto lumAt = [&](int px, int py) {
                        size_t i = (py * w + px) * c;
                        return 0.299f * src[i] + 0.587f * src[i + (std::min)(1u, c - 1)] +
                            0.114f * src[i + (std::min)(2u, c - 1)];
                        };
                    gx = lumAt(x + 1, y) - lumAt(x - 1, y);
                    gy = lumAt(x, y + 1) - lumAt(x, y - 1);
                }
                float edge = (std::min)(255.0f, std::sqrt(gx * gx + gy * gy) * strength);
                float sketch = 255.0f - edge;

                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                uint8_t v = static_cast<uint8_t>((std::max)(0.0f, sketch));
                dst[dstIdx] = dst[dstIdx + 1] = dst[dstIdx + 2] = v;
            }
        }
    }

    inline void ApplyPosterize(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c,
        uint8_t* dst, uint32_t levels) const {
        levels = (std::max)(2u, levels);
        float step = 255.0f / (levels - 1);
        for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {
            for (uint32_t ch = 0; ch < 3 && ch < c; ++ch) {
                float val = src[i * c + ch];
                float quantized = std::round(val / step) * step;
                dst[i * 3 + ch] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(255.0f, quantized)));
            }
        }
    }

    inline void ApplyEmboss(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c, uint8_t* dst) const {
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                if (x > 0 && y > 0 && x < w - 1 && y < h - 1) {
                    for (uint32_t ch = 0; ch < 3 && ch < c; ++ch) {
                        int val = -src[((y - 1) * w + (x - 1)) * c + ch] - src[((y - 1) * w + x) * c + ch]
                            + src[((y + 1) * w + x) * c + ch] + src[((y + 1) * w + (x + 1)) * c + ch] + 128;
                        dst[dstIdx + ch] = static_cast<uint8_t>((std::max)(0, (std::min)(255, val)));
                    }
                }
                else {
                    for (uint32_t ch = 0; ch < 3; ++ch) dst[dstIdx + ch] = 128;
                }
            }
        }
    }

    inline void ApplyVignette(const uint8_t* src, uint32_t w, uint32_t h, uint32_t c,
        uint8_t* dst, float intensity) const {
        float cx = w * 0.5f, cy = h * 0.5f;
        float maxDist = std::sqrt(cx * cx + cy * cy);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                float dx = x - cx, dy = y - cy;
                float dist = std::sqrt(dx * dx + dy * dy) / maxDist;
                float factor = 1.0f - dist * dist * intensity;
                factor = (std::max)(0.0f, factor);
                size_t srcIdx = (y * w + x) * c;
                size_t dstIdx = (static_cast<size_t>(y) * w + x) * 3;
                for (uint32_t ch = 0; ch < 3 && ch < c; ++ch) {
                    dst[dstIdx + ch] = static_cast<uint8_t>(src[srcIdx + ch] * factor);
                }
            }
        }
    }
};

}
} // namespace ExplorerLens::Engine
