// NeuralRadianceDecoder.h — NeRF Scene File Preview
// Copyright (c) 2026 ExplorerLens Project
//
// NeRF scene file preview decoder. Parses NeRF config JSON, extracts reference
// image path for thumbnail generation.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NeRFFormat : uint8_t {
    InstantNGP,
    Nerfies,
    Mip360,
    TensoRF,
    GaussianSplat,
    Unknown
};

struct NeRFCameraFrame
{
    std::string filePath;
    std::array<std::array<float, 4>, 4> transformMatrix = {};
    float focalLength = 0.0f;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct NeRFSceneInfo
{
    NeRFFormat format = NeRFFormat::Unknown;
    uint32_t frameCount = 0;
    float cameraAngleX = 0.0f;
    float cameraAngleY = 0.0f;
    std::vector<NeRFCameraFrame> frames;
    std::string sceneName;
    uint32_t aabbScale = 1;
    bool isValid = false;
};

class NeuralRadianceDecoder
{
  public:
    static NeuralRadianceDecoder& Instance()
    {
        static NeuralRadianceDecoder instance;
        return instance;
    }

    inline bool IsNeRFConfig(const uint8_t* data, size_t size) const
    {
        if (!data || size < 10)
            return false;
        std::string content(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(1024)));
        return (content.find("camera_angle_x") != std::string::npos
                || content.find("transform_matrix") != std::string::npos || content.find("frames") != std::string::npos);
    }

    inline NeRFSceneInfo ParseConfig(const uint8_t* data, size_t size) const
    {
        NeRFSceneInfo info;
        if (!data || size == 0)
            return info;

        std::string content(reinterpret_cast<const char*>(data), size);
        info.isValid = IsNeRFConfig(data, size);
        if (!info.isValid)
            return info;

        info.cameraAngleX = ExtractFloatValue(content, "camera_angle_x");
        info.cameraAngleY = ExtractFloatValue(content, "camera_angle_y");
        info.aabbScale = static_cast<uint32_t>(ExtractFloatValue(content, "aabb_scale"));
        if (info.aabbScale == 0)
            info.aabbScale = 1;

        info.format = DetectFormat(content);
        info.frameCount = CountOccurrences(content, "file_path");
        info.frames = ExtractFramePaths(content);

        return info;
    }

    inline std::string GetBestReferenceImagePath(const NeRFSceneInfo& info) const
    {
        if (info.frames.empty())
            return "";
        size_t midIdx = info.frames.size() / 2;
        return info.frames[midIdx].filePath;
    }

    inline std::vector<uint8_t> GenerateCameraVisualization(const NeRFSceneInfo& info, uint32_t width,
                                                            uint32_t height) const
    {
        std::vector<uint8_t> thumbnail(static_cast<size_t>(width) * height * 3, 25);
        if (width == 0 || height == 0 || info.frames.empty())
            return thumbnail;

        float cx = width / 2.0f, cy = height / 2.0f;
        float scale = (std::min)(width, height) * 0.35f;

        for (size_t i = 0; i < info.frames.size(); ++i) {
            float angle = static_cast<float>(i) / info.frames.size() * 2.0f * 3.14159f;
            float radius = 0.7f + 0.2f * std::sin(angle * 3.0f);

            int px = static_cast<int>(cx + std::cos(angle) * radius * scale);
            int py = static_cast<int>(cy + std::sin(angle) * radius * scale * 0.5f);

            DrawDot(thumbnail.data(), width, height, px, py, 3, 100, 200, 255);
        }

        DrawDot(thumbnail.data(), width, height, static_cast<int>(cx), static_cast<int>(cy), 6, 255, 100, 50);

        return thumbnail;
    }

    inline std::string FormatToString(NeRFFormat fmt) const
    {
        switch (fmt) {
            case NeRFFormat::InstantNGP:
                return "Instant-NGP";
            case NeRFFormat::Nerfies:
                return "Nerfies";
            case NeRFFormat::Mip360:
                return "Mip-NeRF 360";
            case NeRFFormat::TensoRF:
                return "TensoRF";
            case NeRFFormat::GaussianSplat:
                return "3D Gaussian Splatting";
            default:
                return "Unknown NeRF";
        }
    }

  private:
    NeuralRadianceDecoder() = default;

    inline NeRFFormat DetectFormat(const std::string& content) const
    {
        if (content.find("aabb_scale") != std::string::npos)
            return NeRFFormat::InstantNGP;
        if (content.find("nerfies") != std::string::npos)
            return NeRFFormat::Nerfies;
        if (content.find("ply_file_path") != std::string::npos)
            return NeRFFormat::GaussianSplat;
        return NeRFFormat::InstantNGP;
    }

    inline float ExtractFloatValue(const std::string& json, const std::string& key) const
    {
        size_t pos = json.find("\"" + key + "\"");
        if (pos == std::string::npos)
            return 0.0f;
        pos = json.find(':', pos);
        if (pos == std::string::npos)
            return 0.0f;
        ++pos;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
            ++pos;
        float result = 0.0f;
        bool negative = false;
        if (pos < json.size() && json[pos] == '-') {
            negative = true;
            ++pos;
        }
        bool hasDot = false;
        float divisor = 1.0f;
        while (pos < json.size()) {
            char c = json[pos];
            if (c >= '0' && c <= '9') {
                if (hasDot) {
                    divisor *= 10.0f;
                    result += (c - '0') / divisor;
                } else
                    result = result * 10.0f + (c - '0');
            } else if (c == '.' && !hasDot) {
                hasDot = true;
            } else
                break;
            ++pos;
        }
        return negative ? -result : result;
    }

    inline uint32_t CountOccurrences(const std::string& str, const std::string& sub) const
    {
        uint32_t count = 0;
        size_t pos = 0;
        while ((pos = str.find(sub, pos)) != std::string::npos) {
            ++count;
            pos += sub.size();
        }
        return count;
    }

    inline std::vector<NeRFCameraFrame> ExtractFramePaths(const std::string& json) const
    {
        std::vector<NeRFCameraFrame> frames;
        size_t pos = 0;
        while ((pos = json.find("\"file_path\"", pos)) != std::string::npos) {
            pos = json.find(':', pos);
            if (pos == std::string::npos)
                break;
            size_t start = json.find('"', pos + 1);
            if (start == std::string::npos)
                break;
            size_t end = json.find('"', start + 1);
            if (end == std::string::npos)
                break;
            NeRFCameraFrame frame;
            frame.filePath = json.substr(start + 1, end - start - 1);
            frames.push_back(frame);
            pos = end + 1;
        }
        return frames;
    }

    inline void DrawDot(uint8_t* pixels, uint32_t w, uint32_t h, int cx, int cy, int r, uint8_t cr, uint8_t cg,
                        uint8_t cb) const
    {
        for (int dy = -r; dy <= r; ++dy) {
            for (int dxx = -r; dxx <= r; ++dxx) {
                if (dxx * dxx + dy * dy <= r * r) {
                    int px = cx + dxx, py = cy + dy;
                    if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                        size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                        pixels[idx] = cr;
                        pixels[idx + 1] = cg;
                        pixels[idx + 2] = cb;
                    }
                }
            }
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
