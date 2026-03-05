// TextRecognitionThumb.h — OCR Text Extraction for Document Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// OCR text extraction for document thumbnail previews. Extracts first few lines
// of text content and renders them as a clean text overlay on the thumbnail.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

struct TextLine {
    std::string text;
    int32_t y = 0;
    int32_t x = 0;
    float confidence = 0.0f;
};

struct TextRegion {
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<TextLine> lines;
    float avgConfidence = 0.0f;
};

struct TextRenderConfig {
    uint32_t fontSize = 10;
    uint32_t maxLines = 6;
    uint32_t marginX = 8;
    uint32_t marginY = 8;
    uint32_t lineSpacing = 2;
    std::array<uint8_t, 3> textColor = { 32, 32, 32 };
    std::array<uint8_t, 3> bgColor = { 255, 255, 255 };
    bool drawBorder = true;
};

class TextRecognitionThumb {
public:
    static TextRecognitionThumb& Instance() {
        static TextRecognitionThumb instance;
        return instance;
    }

    inline std::vector<TextLine> ExtractTextLines(const uint8_t* grayPixels, uint32_t width, uint32_t height,
        uint32_t maxLines = 6) const {
        std::vector<TextLine> lines;
        if (!grayPixels || width == 0 || height == 0) return lines;

        auto rowProjection = ComputeHorizontalProjection(grayPixels, width, height);
        auto lineRegions = FindTextLineRegions(rowProjection, height);

        uint32_t lineCount = 0;
        for (const auto& region : lineRegions) {
            if (lineCount >= maxLines) break;
            TextLine line;
            line.y = region.first;
            line.x = 0;
            line.confidence = ComputeLineConfidence(grayPixels, width, region.first, region.second);
            line.text = "[Line " + std::to_string(lineCount + 1) + "]";
            lines.push_back(line);
            ++lineCount;
        }
        return lines;
    }

    inline std::vector<uint8_t> RenderTextPreview(uint32_t width, uint32_t height,
        const std::vector<std::string>& textLines,
        const TextRenderConfig& config = {}) const {
        std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 3);

        for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
            pixels[i * 3] = config.bgColor[0];
            pixels[i * 3 + 1] = config.bgColor[1];
            pixels[i * 3 + 2] = config.bgColor[2];
        }

        if (config.drawBorder) {
            DrawBorder(pixels.data(), width, height, { 180, 180, 180 });
        }

        uint32_t lineHeight = config.fontSize + config.lineSpacing;
        uint32_t y = config.marginY;
        uint32_t maxVisibleLines = (std::min)(static_cast<uint32_t>(textLines.size()), config.maxLines);

        for (uint32_t i = 0; i < maxVisibleLines && y + lineHeight < height; ++i) {
            RenderTextLine(pixels.data(), width, height, config.marginX, y,
                textLines[i], config.fontSize, config.textColor);
            y += lineHeight;
        }

        return pixels;
    }

    inline bool DetectTextPresence(const uint8_t* grayPixels, uint32_t width, uint32_t height) const {
        if (!grayPixels || width < 10 || height < 10) return false;

        uint32_t edgeCount = 0;
        uint32_t totalSampled = 0;
        uint32_t step = (std::max)(1u, (std::min)(width, height) / 32);

        for (uint32_t y = 1; y < height - 1; y += step) {
            for (uint32_t x = 1; x < width - 1; x += step) {
                int gx = grayPixels[y * width + x + 1] - grayPixels[y * width + x - 1];
                int gy = grayPixels[(y + 1) * width + x] - grayPixels[(y - 1) * width + x];
                if (std::abs(gx) + std::abs(gy) > 40) ++edgeCount;
                ++totalSampled;
            }
        }

        float edgeRatio = totalSampled > 0 ? static_cast<float>(edgeCount) / totalSampled : 0.0f;
        return edgeRatio > 0.1f && edgeRatio < 0.7f;
    }

private:
    TextRecognitionThumb() = default;

    inline std::vector<uint32_t> ComputeHorizontalProjection(const uint8_t* gray, uint32_t w, uint32_t h) const {
        std::vector<uint32_t> projection(h, 0);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                if (gray[y * w + x] < 128) projection[y]++;
            }
        }
        return projection;
    }

    inline std::vector<std::pair<int32_t, int32_t>> FindTextLineRegions(
        const std::vector<uint32_t>& projection, uint32_t height) const {
        std::vector<std::pair<int32_t, int32_t>> regions;
        uint32_t threshold = 0;
        for (auto v : projection) threshold += v;
        threshold = threshold / (std::max)(1u, height) / 2;

        bool inLine = false;
        int32_t lineStart = 0;
        for (uint32_t y = 0; y < height; ++y) {
            if (projection[y] > threshold) {
                if (!inLine) { lineStart = static_cast<int32_t>(y); inLine = true; }
            }
            else {
                if (inLine) {
                    regions.push_back({ lineStart, static_cast<int32_t>(y) });
                    inLine = false;
                }
            }
        }
        if (inLine) regions.push_back({ lineStart, static_cast<int32_t>(height) });
        return regions;
    }

    inline float ComputeLineConfidence(const uint8_t* gray, uint32_t w, int32_t yStart, int32_t yEnd) const {
        if (yEnd <= yStart) return 0.0f;
        uint32_t darkPixels = 0, total = 0;
        for (int32_t y = yStart; y < yEnd; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                if (gray[y * w + x] < 128) ++darkPixels;
                ++total;
            }
        }
        float ratio = total > 0 ? static_cast<float>(darkPixels) / total : 0.0f;
        return (ratio > 0.02f && ratio < 0.5f) ? 0.8f : 0.3f;
    }

    inline void DrawBorder(uint8_t* pixels, uint32_t w, uint32_t h, std::array<uint8_t, 3> color) const {
        for (uint32_t x = 0; x < w; ++x) {
            SetPixel(pixels, w, x, 0, color);
            SetPixel(pixels, w, x, h - 1, color);
        }
        for (uint32_t y = 0; y < h; ++y) {
            SetPixel(pixels, w, 0, y, color);
            SetPixel(pixels, w, w - 1, y, color);
        }
    }

    inline void SetPixel(uint8_t* pixels, uint32_t w, uint32_t x, uint32_t y, std::array<uint8_t, 3> color) const {
        size_t idx = (static_cast<size_t>(y) * w + x) * 3;
        pixels[idx] = color[0]; pixels[idx + 1] = color[1]; pixels[idx + 2] = color[2];
    }

    inline void RenderTextLine(uint8_t* pixels, uint32_t w, uint32_t h, uint32_t x, uint32_t y,
        const std::string& text, uint32_t fontSize,
        std::array<uint8_t, 3> color) const {
        uint32_t charWidth = fontSize * 3 / 5;
        uint32_t maxChars = (w - x * 2) / (std::max)(1u, charWidth);
        uint32_t charsToRender = (std::min)(static_cast<uint32_t>(text.size()), maxChars);

        for (uint32_t i = 0; i < charsToRender; ++i) {
            RenderSimpleGlyph(pixels, w, h, x + i * charWidth, y, fontSize, color, text[i]);
        }
    }

    inline void RenderSimpleGlyph(uint8_t* pixels, uint32_t w, uint32_t h,
        uint32_t gx, uint32_t gy, uint32_t size,
        std::array<uint8_t, 3> color, char ch) const {
        if (ch == ' ') return;
        uint32_t cw = size * 3 / 5;
        uint32_t ch2 = size;
        uint32_t lineW = (std::max)(1u, size / 8);

        for (uint32_t dy = 0; dy < ch2 && gy + dy < h; ++dy) {
            for (uint32_t dx = 0; dx < cw && gx + dx < w; ++dx) {
                bool draw = false;
                float fy = static_cast<float>(dy) / ch2;
                float fx = static_cast<float>(dx) / cw;

                if (dx < lineW || dx >= cw - lineW) draw = true;
                if (dy < lineW || dy >= ch2 - lineW) draw = true;
                if (std::abs(fy - 0.5f) < static_cast<float>(lineW) / ch2) draw = true;

                if (draw) SetPixel(pixels, w, gx + dx, gy + dy, color);
            }
        }
    }
};

}
} // namespace ExplorerLens::Engine
