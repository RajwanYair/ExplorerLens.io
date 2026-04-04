// AlembicDecoder.h — Alembic Interchange Format Preview
// Copyright (c) 2026 ExplorerLens Project
//
// Alembic interchange format (.abc) preview decoder. Parses Alembic header,
// generates wireframe bounding box thumbnail for 3D scene files.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct AlembicBBox
{
    std::array<float, 3> minCorner = {0.0f, 0.0f, 0.0f};
    std::array<float, 3> maxCorner = {1.0f, 1.0f, 1.0f};

    inline float Width() const
    {
        return maxCorner[0] - minCorner[0];
    }
    inline float Height() const
    {
        return maxCorner[1] - minCorner[1];
    }
    inline float Depth() const
    {
        return maxCorner[2] - minCorner[2];
    }
};

struct AlembicObjectInfo
{
    std::string name;
    std::string typeName;
    uint32_t childCount = 0;
    AlembicBBox bounds;
};

struct AlembicFileInfo
{
    std::string application;
    std::string dateWritten;
    uint32_t version = 0;
    uint32_t objectCount = 0;
    double startTime = 0.0;
    double endTime = 0.0;
    uint32_t frameCount = 0;
    AlembicBBox sceneBounds;
    bool isValid = false;
};

class AlembicDecoder
{
  public:
    static constexpr uint8_t ABC_MAGIC[] = {0x89, 0x48, 0x44, 0x46};

    static AlembicDecoder& Instance()
    {
        static AlembicDecoder instance;
        return instance;
    }

    inline bool IsAlembicFile(const uint8_t* data, size_t size) const
    {
        if (!data || size < 4)
            return false;
        return data[0] == 0x89 && data[1] == 0x48 && data[2] == 0x44 && data[3] == 0x46;
    }

    inline AlembicFileInfo ParseHeader(const uint8_t* data, size_t size) const
    {
        AlembicFileInfo info;
        if (!data || size < 8)
            return info;

        info.isValid = IsAlembicFile(data, size);
        if (!info.isValid)
            return info;

        if (size >= 12) {
            info.version = static_cast<uint32_t>(data[8]) | (static_cast<uint32_t>(data[9]) << 8);
        }

        info.application = "Alembic";
        info.sceneBounds.minCorner = {-1.0f, -1.0f, -1.0f};
        info.sceneBounds.maxCorner = {1.0f, 1.0f, 1.0f};
        return info;
    }

    inline std::vector<uint8_t> GenerateWireframeThumbnail(const AlembicBBox& bounds, uint32_t thumbWidth,
                                                           uint32_t thumbHeight, uint8_t bgColor = 30,
                                                           uint8_t lineColor = 200) const
    {
        std::vector<uint8_t> thumbnail(static_cast<size_t>(thumbWidth) * thumbHeight * 3, bgColor);
        if (thumbWidth == 0 || thumbHeight == 0)
            return thumbnail;

        float cx = (bounds.minCorner[0] + bounds.maxCorner[0]) * 0.5f;
        float cy = (bounds.minCorner[1] + bounds.maxCorner[1]) * 0.5f;
        float cz = (bounds.minCorner[2] + bounds.maxCorner[2]) * 0.5f;

        float scale = (std::max)({bounds.Width(), bounds.Height(), bounds.Depth()});
        if (scale < 1e-6f)
            scale = 1.0f;
        float invScale = 0.8f / scale;

        std::array<std::array<float, 3>, 8> corners;
        int idx = 0;
        for (int zz = 0; zz < 2; ++zz) {
            for (int yy = 0; yy < 2; ++yy) {
                for (int xx = 0; xx < 2; ++xx) {
                    corners[idx][0] = (xx == 0 ? bounds.minCorner[0] : bounds.maxCorner[0]) - cx;
                    corners[idx][1] = (yy == 0 ? bounds.minCorner[1] : bounds.maxCorner[1]) - cy;
                    corners[idx][2] = (zz == 0 ? bounds.minCorner[2] : bounds.maxCorner[2]) - cz;
                    ++idx;
                }
            }
        }

        auto project = [&](const std::array<float, 3>& p) -> std::pair<int, int> {
            float cosA = 0.866f, sinA = 0.5f;
            float px = p[0] * cosA - p[2] * sinA;
            float py = -p[1] + (p[0] * sinA + p[2] * cosA) * 0.3f;
            int sx = static_cast<int>(thumbWidth / 2 + px * invScale * thumbWidth * 0.4f);
            int sy = static_cast<int>(thumbHeight / 2 + py * invScale * thumbHeight * 0.4f);
            return {sx, sy};
        };

        static const int edges[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3},
                                         {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

        for (const auto& edge : edges) {
            auto [x0, y0] = project(corners[edge[0]]);
            auto [x1, y1] = project(corners[edge[1]]);
            DrawLine(thumbnail.data(), thumbWidth, thumbHeight, x0, y0, x1, y1, lineColor);
        }

        return thumbnail;
    }

    inline std::string FormatSceneInfo(const AlembicFileInfo& info) const
    {
        std::string result = "Alembic v" + std::to_string(info.version);
        result += " | Objects: " + std::to_string(info.objectCount);
        if (info.frameCount > 0) {
            result += " | Frames: " + std::to_string(info.frameCount);
        }
        return result;
    }

  private:
    AlembicDecoder() = default;

    inline void DrawLine(uint8_t* pixels, uint32_t w, uint32_t h, int x0, int y0, int x1, int y1, uint8_t color) const
    {
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        for (int iter = 0; iter < 10000; ++iter) {
            if (x0 >= 0 && x0 < static_cast<int>(w) && y0 >= 0 && y0 < static_cast<int>(h)) {
                size_t idx = (static_cast<size_t>(y0) * w + x0) * 3;
                pixels[idx] = color;
                pixels[idx + 1] = color;
                pixels[idx + 2] = color;
            }
            if (x0 == x1 && y0 == y1)
                break;
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
