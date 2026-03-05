// PointCloudStreamDecoder.h — LAS/LAZ Point Cloud Streaming Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// LAS/LAZ point cloud streaming decoder. Reads LAS header, extracts point subset,
// generates top-down density plot thumbnail.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct LASHeader {
    char signature[4] = {};
    uint16_t fileSourceId = 0;
    uint8_t versionMajor = 0;
    uint8_t versionMinor = 0;
    uint16_t pointDataFormatId = 0;
    uint16_t pointDataRecordLength = 0;
    uint64_t numberOfPoints = 0;
    double scaleX = 1.0, scaleY = 1.0, scaleZ = 1.0;
    double offsetX = 0.0, offsetY = 0.0, offsetZ = 0.0;
    double minX = 0.0, minY = 0.0, minZ = 0.0;
    double maxX = 0.0, maxY = 0.0, maxZ = 0.0;
    uint32_t offsetToPointData = 0;
    bool isValid = false;
};

struct LASPoint {
    double x = 0.0, y = 0.0, z = 0.0;
    uint16_t intensity = 0;
    uint8_t classification = 0;
    uint8_t r = 255, g = 255, b = 255;
};

class PointCloudStreamDecoder {
public:
    static PointCloudStreamDecoder& Instance() {
        static PointCloudStreamDecoder instance;
        return instance;
    }

    inline bool IsLASFile(const uint8_t* data, size_t size) const {
        if (!data || size < 4) return false;
        return data[0] == 'L' && data[1] == 'A' && data[2] == 'S' && data[3] == 'F';
    }

    inline LASHeader ParseHeader(const uint8_t* data, size_t size) const {
        LASHeader header;
        if (!data || size < 227) return header;

        std::memcpy(header.signature, data, 4);
        header.isValid = IsLASFile(data, size);
        if (!header.isValid) return header;

        header.fileSourceId = ReadU16LE(data, 4);
        header.versionMajor = data[24];
        header.versionMinor = data[25];
        header.offsetToPointData = ReadU32LE(data, 96);
        header.pointDataFormatId = data[104];
        header.pointDataRecordLength = ReadU16LE(data, 105);

        if (header.versionMajor <= 1 && header.versionMinor < 4) {
            header.numberOfPoints = ReadU32LE(data, 107);
        }
        else if (size >= 247) {
            header.numberOfPoints = ReadU64LE(data, 247);
        }

        if (size >= 227) {
            header.scaleX = ReadF64LE(data, 131);
            header.scaleY = ReadF64LE(data, 139);
            header.scaleZ = ReadF64LE(data, 147);
            header.offsetX = ReadF64LE(data, 155);
            header.offsetY = ReadF64LE(data, 163);
            header.offsetZ = ReadF64LE(data, 171);
            header.maxX = ReadF64LE(data, 179);
            header.minX = ReadF64LE(data, 187);
            header.maxY = ReadF64LE(data, 195);
            header.minY = ReadF64LE(data, 203);
            header.maxZ = ReadF64LE(data, 211);
            header.minZ = ReadF64LE(data, 219);
        }
        return header;
    }

    inline std::vector<uint8_t> GenerateDensityThumbnail(const LASPoint* points, size_t pointCount,
        double minX, double minY, double maxX, double maxY,
        uint32_t thumbWidth, uint32_t thumbHeight) const {
        std::vector<uint32_t> density(static_cast<size_t>(thumbWidth) * thumbHeight, 0);
        std::vector<uint8_t> thumbnail(static_cast<size_t>(thumbWidth) * thumbHeight * 3, 0);

        if (!points || pointCount == 0 || thumbWidth == 0 || thumbHeight == 0) return thumbnail;

        double rangeX = maxX - minX;
        double rangeY = maxY - minY;
        if (rangeX < 1e-10) rangeX = 1.0;
        if (rangeY < 1e-10) rangeY = 1.0;

        uint32_t maxDensity = 0;
        for (size_t i = 0; i < pointCount; ++i) {
            double nx = (points[i].x - minX) / rangeX;
            double ny = (points[i].y - minY) / rangeY;
            int px = static_cast<int>(nx * (thumbWidth - 1));
            int py = static_cast<int>((1.0 - ny) * (thumbHeight - 1));
            if (px >= 0 && px < static_cast<int>(thumbWidth) && py >= 0 && py < static_cast<int>(thumbHeight)) {
                uint32_t& d = density[static_cast<size_t>(py) * thumbWidth + px];
                ++d;
                if (d > maxDensity) maxDensity = d;
            }
        }

        if (maxDensity == 0) return thumbnail;
        float logMax = std::log(static_cast<float>(maxDensity) + 1.0f);

        for (uint32_t y = 0; y < thumbHeight; ++y) {
            for (uint32_t x = 0; x < thumbWidth; ++x) {
                size_t idx = static_cast<size_t>(y) * thumbWidth + x;
                if (density[idx] > 0) {
                    float normalized = std::log(static_cast<float>(density[idx]) + 1.0f) / logMax;
                    auto [r, g, b] = HeatMapColor(normalized);
                    size_t pIdx = idx * 3;
                    thumbnail[pIdx + 0] = r;
                    thumbnail[pIdx + 1] = g;
                    thumbnail[pIdx + 2] = b;
                }
            }
        }
        return thumbnail;
    }

    inline std::string FormatHeaderInfo(const LASHeader& header) const {
        return std::string("LAS ") + std::to_string(header.versionMajor) + "." +
            std::to_string(header.versionMinor) + " | Points: " +
            std::to_string(header.numberOfPoints) + " | Format: " +
            std::to_string(header.pointDataFormatId);
    }

private:
    PointCloudStreamDecoder() = default;

    inline uint16_t ReadU16LE(const uint8_t* d, size_t o) const {
        return static_cast<uint16_t>(d[o]) | (static_cast<uint16_t>(d[o + 1]) << 8);
    }
    inline uint32_t ReadU32LE(const uint8_t* d, size_t o) const {
        return static_cast<uint32_t>(d[o]) | (static_cast<uint32_t>(d[o + 1]) << 8) |
            (static_cast<uint32_t>(d[o + 2]) << 16) | (static_cast<uint32_t>(d[o + 3]) << 24);
    }
    inline uint64_t ReadU64LE(const uint8_t* d, size_t o) const {
        return static_cast<uint64_t>(ReadU32LE(d, o)) | (static_cast<uint64_t>(ReadU32LE(d, o + 4)) << 32);
    }
    inline double ReadF64LE(const uint8_t* d, size_t o) const {
        double v = 0.0;
        std::memcpy(&v, d + o, 8);
        return v;
    }

    struct RGB { uint8_t r, g, b; };
    inline RGB HeatMapColor(float t) const {
        t = (std::max)(0.0f, (std::min)(1.0f, t));
        uint8_t r = static_cast<uint8_t>((std::min)(255.0f, t * 3.0f * 255.0f));
        uint8_t g = static_cast<uint8_t>((std::min)(255.0f, (std::max)(0.0f, (t - 0.33f) * 3.0f * 255.0f)));
        uint8_t b = static_cast<uint8_t>((std::min)(255.0f, (std::max)(0.0f, (t - 0.67f) * 3.0f * 255.0f)));
        return { r, g, b };
    }
};

}
} // namespace ExplorerLens::Engine
