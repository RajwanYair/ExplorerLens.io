// PCBLayoutDecoder.h — PCB Design File Preview
// Copyright (c) 2026 ExplorerLens Project
//
// PCB design file preview decoder for Gerber and KiCad formats. Parses board
// outline, generates 2D board layout thumbnail with layer colors.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

enum class PCBFormat : uint8_t {
    Gerber,
    GerberX2,
    KiCadPCB,
    EagleBRD,
    Unknown
};

enum class PCBLayer : uint8_t {
    TopCopper,
    BottomCopper,
    TopSilk,
    BottomSilk,
    TopMask,
    BottomMask,
    BoardOutline,
    Drill,
    InnerCopper
};

struct PCBPad {
    float x = 0.0f, y = 0.0f;
    float width = 0.5f, height = 0.5f;
    bool isCircle = true;
    PCBLayer layer = PCBLayer::TopCopper;
};

struct PCBTrace {
    float x0 = 0.0f, y0 = 0.0f;
    float x1 = 0.0f, y1 = 0.0f;
    float traceWidth = 0.25f;
    PCBLayer layer = PCBLayer::TopCopper;
};

struct PCBBoardInfo {
    PCBFormat format = PCBFormat::Unknown;
    float width = 0.0f;
    float height = 0.0f;
    uint32_t layerCount = 2;
    uint32_t padCount = 0;
    uint32_t traceCount = 0;
    uint32_t viaCount = 0;
    std::string title;
    bool isValid = false;
};

class PCBLayoutDecoder {
public:
    static PCBLayoutDecoder& Instance() {
        static PCBLayoutDecoder instance;
        return instance;
    }

    inline PCBFormat DetectFormat(const uint8_t* data, size_t size) const {
        if (!data || size < 8) return PCBFormat::Unknown;
        std::string header(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(512)));
        if (header.find("(kicad_pcb") != std::string::npos) return PCBFormat::KiCadPCB;
        if (header.find("%FSLAX") != std::string::npos || header.find("G04") != std::string::npos)
            return PCBFormat::Gerber;
        if (header.find("TF.FileFunction") != std::string::npos) return PCBFormat::GerberX2;
        if (header.find("<?xml") != std::string::npos && header.find("<eagle") != std::string::npos)
            return PCBFormat::EagleBRD;
        return PCBFormat::Unknown;
    }

    inline PCBBoardInfo ParseBoardInfo(const uint8_t* data, size_t size) const {
        PCBBoardInfo info;
        info.format = DetectFormat(data, size);
        info.isValid = info.format != PCBFormat::Unknown;
        if (!info.isValid) return info;

        std::string content(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(8192)));

        if (info.format == PCBFormat::KiCadPCB) {
            info.layerCount = CountOccurrences(content, "(layer ");
            info.padCount = CountOccurrences(content, "(pad ");
            info.traceCount = CountOccurrences(content, "(segment ");
            info.viaCount = CountOccurrences(content, "(via ");
        }

        return info;
    }

    inline std::vector<uint8_t> GenerateBoardThumbnail(const std::vector<PCBPad>& pads,
        const std::vector<PCBTrace>& traces,
        float boardW, float boardH,
        uint32_t thumbWidth, uint32_t thumbHeight) const {
        std::vector<uint8_t> thumbnail(static_cast<size_t>(thumbWidth) * thumbHeight * 3, 0);
        if (thumbWidth == 0 || thumbHeight == 0 || boardW <= 0.0f || boardH <= 0.0f) return thumbnail;

        FillRect(thumbnail.data(), thumbWidth, thumbHeight, 0, 0, thumbWidth, thumbHeight, 20, 60, 20);

        float margin = 0.05f;
        float scaleX = (thumbWidth * (1.0f - 2 * margin)) / boardW;
        float scaleY = (thumbHeight * (1.0f - 2 * margin)) / boardH;
        float scale = (std::min)(scaleX, scaleY);
        float offsetX = thumbWidth * margin + (thumbWidth * (1.0f - 2 * margin) - boardW * scale) * 0.5f;
        float offsetY = thumbHeight * margin + (thumbHeight * (1.0f - 2 * margin) - boardH * scale) * 0.5f;

        for (const auto& trace : traces) {
            auto [r, g, b] = GetLayerColor(trace.layer);
            int x0 = static_cast<int>(trace.x0 * scale + offsetX);
            int y0 = static_cast<int>(trace.y0 * scale + offsetY);
            int x1 = static_cast<int>(trace.x1 * scale + offsetX);
            int y1 = static_cast<int>(trace.y1 * scale + offsetY);
            DrawLine(thumbnail.data(), thumbWidth, thumbHeight, x0, y0, x1, y1, r, g, b);
        }

        for (const auto& pad : pads) {
            auto [r, g, b] = GetLayerColor(pad.layer);
            int px = static_cast<int>(pad.x * scale + offsetX);
            int py = static_cast<int>(pad.y * scale + offsetY);
            int pr = static_cast<int>((std::max)(pad.width, pad.height) * scale * 0.5f);
            pr = (std::max)(1, pr);

            if (pad.isCircle) {
                DrawFilledCircle(thumbnail.data(), thumbWidth, thumbHeight, px, py, pr, r, g, b);
            }
            else {
                int hw = static_cast<int>(pad.width * scale * 0.5f);
                int hh = static_cast<int>(pad.height * scale * 0.5f);
                FillRect(thumbnail.data(), thumbWidth, thumbHeight, px - hw, py - hh, hw * 2, hh * 2, r, g, b);
            }
        }

        return thumbnail;
    }

    inline std::string FormatToString(PCBFormat fmt) const {
        switch (fmt) {
        case PCBFormat::Gerber:   return "Gerber";
        case PCBFormat::GerberX2: return "Gerber X2";
        case PCBFormat::KiCadPCB: return "KiCad PCB";
        case PCBFormat::EagleBRD: return "Eagle BRD";
        default:                  return "Unknown";
        }
    }

private:
    PCBLayoutDecoder() = default;

    struct RGB { uint8_t r, g, b; };

    inline RGB GetLayerColor(PCBLayer layer) const {
        switch (layer) {
        case PCBLayer::TopCopper:    return { 200, 40, 40 };
        case PCBLayer::BottomCopper: return { 40, 40, 200 };
        case PCBLayer::TopSilk:      return { 220, 220, 220 };
        case PCBLayer::BottomSilk:   return { 180, 180, 220 };
        case PCBLayer::TopMask:      return { 100, 200, 100 };
        case PCBLayer::BottomMask:   return { 100, 100, 200 };
        case PCBLayer::BoardOutline: return { 255, 255, 0 };
        case PCBLayer::Drill:        return { 255, 255, 255 };
        case PCBLayer::InnerCopper:  return { 200, 150, 50 };
        default:                     return { 128, 128, 128 };
        }
    }

    inline uint32_t CountOccurrences(const std::string& str, const std::string& sub) const {
        uint32_t count = 0;
        size_t pos = 0;
        while ((pos = str.find(sub, pos)) != std::string::npos) { ++count; pos += sub.size(); }
        return count;
    }

    inline void FillRect(uint8_t* p, uint32_t w, uint32_t h, int x, int y, int rw, int rh,
        uint8_t cr, uint8_t cg, uint8_t cb) const {
        for (int dy = 0; dy < rh; ++dy)
            for (int dx = 0; dx < rw; ++dx) {
                int px = x + dx, py = y + dy;
                if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                    size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                    p[idx] = cr; p[idx + 1] = cg; p[idx + 2] = cb;
                }
            }
    }

    inline void DrawFilledCircle(uint8_t* p, uint32_t w, uint32_t h, int cx, int cy, int r,
        uint8_t cr, uint8_t cg, uint8_t cb) const {
        for (int dy = -r; dy <= r; ++dy)
            for (int dx = -r; dx <= r; ++dx)
                if (dx * dx + dy * dy <= r * r) {
                    int px = cx + dx, py = cy + dy;
                    if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                        size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                        p[idx] = cr; p[idx + 1] = cg; p[idx + 2] = cb;
                    }
                }
    }

    inline void DrawLine(uint8_t* p, uint32_t w, uint32_t h, int x0, int y0, int x1, int y1,
        uint8_t cr, uint8_t cg, uint8_t cb) const {
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        for (int i = 0; i < 10000; ++i) {
            if (x0 >= 0 && x0 < static_cast<int>(w) && y0 >= 0 && y0 < static_cast<int>(h)) {
                size_t idx = (static_cast<size_t>(y0) * w + x0) * 3;
                p[idx] = cr; p[idx + 1] = cg; p[idx + 2] = cb;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
};

}
} // namespace ExplorerLens::Engine
