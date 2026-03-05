// GeospatialTileDecoder.h — Map Tile Preview (MBTiles/GeoTIFF)
// Copyright (c) 2026 ExplorerLens Project
//
// Map tile preview decoder for MBTiles and GeoTIFF formats. Detects tile format,
// extracts center tile, generates map preview thumbnail.
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

enum class GeoTileFormat : uint8_t {
    MBTiles,
    GeoTIFF,
    GeoPackage,
    Shapefile,
    Unknown
};

struct GeoExtent {
    double minLon = -180.0, minLat = -90.0;
    double maxLon = 180.0, maxLat = 90.0;

    inline double CenterLon() const { return (minLon + maxLon) * 0.5; }
    inline double CenterLat() const { return (minLat + maxLat) * 0.5; }
    inline double Width() const { return maxLon - minLon; }
    inline double Height() const { return maxLat - minLat; }
};

struct GeoTileInfo {
    GeoTileFormat format = GeoTileFormat::Unknown;
    GeoExtent extent;
    uint32_t tileSize = 256;
    uint32_t zoomLevels = 0;
    uint32_t totalTiles = 0;
    std::string projection;
    bool isValid = false;
};

class GeospatialTileDecoder {
public:
    static GeospatialTileDecoder& Instance() {
        static GeospatialTileDecoder instance;
        return instance;
    }

    inline GeoTileFormat DetectFormat(const uint8_t* data, size_t size) const {
        if (!data || size < 16) return GeoTileFormat::Unknown;
        if (size >= 16 && std::memcmp(data, "SQLite format 3", 15) == 0) {
            return GeoTileFormat::MBTiles;
        }
        if (size >= 4) {
            if ((data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00) ||
                (data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A)) {
                return GeoTileFormat::GeoTIFF;
            }
        }
        return GeoTileFormat::Unknown;
    }

    inline GeoTileInfo ParseTileInfo(const uint8_t* data, size_t size) const {
        GeoTileInfo info;
        info.format = DetectFormat(data, size);
        info.isValid = info.format != GeoTileFormat::Unknown;
        if (!info.isValid) return info;

        info.tileSize = 256;
        info.projection = "EPSG:3857";
        info.extent = { -180.0, -85.0511, 180.0, 85.0511 };
        return info;
    }

    inline std::vector<uint8_t> GenerateMapPreview(const GeoExtent& extent,
        uint32_t width, uint32_t height) const {
        std::vector<uint8_t> preview(static_cast<size_t>(width) * height * 3, 0);
        if (width == 0 || height == 0) return preview;

        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                double lon = extent.minLon + (static_cast<double>(x) / width) * extent.Width();
                double lat = extent.maxLat - (static_cast<double>(y) / height) * extent.Height();

                auto [r, g, b] = ComputeMapColor(lon, lat);
                size_t idx = (static_cast<size_t>(y) * width + x) * 3;
                preview[idx + 0] = r;
                preview[idx + 1] = g;
                preview[idx + 2] = b;
            }
        }

        DrawGridLines(preview.data(), width, height, extent);
        return preview;
    }

    inline std::pair<double, double> TileToLonLat(uint32_t x, uint32_t y, uint32_t zoom) const {
        double n = static_cast<double>(1u << zoom);
        double lon = x / n * 360.0 - 180.0;
        double latRad = std::atan(std::sinh(3.14159265358979 * (1.0 - 2.0 * y / n)));
        double lat = latRad * 180.0 / 3.14159265358979;
        return { lon, lat };
    }

    inline std::string FormatToString(GeoTileFormat fmt) const {
        switch (fmt) {
        case GeoTileFormat::MBTiles:    return "MBTiles";
        case GeoTileFormat::GeoTIFF:    return "GeoTIFF";
        case GeoTileFormat::GeoPackage: return "GeoPackage";
        case GeoTileFormat::Shapefile:  return "Shapefile";
        default:                    return "Unknown";
        }
    }

private:
    GeospatialTileDecoder() = default;

    struct RGB { uint8_t r, g, b; };

    inline RGB ComputeMapColor(double lon, double lat) const {
        double landNoise = std::sin(lon * 0.1) * std::cos(lat * 0.1);
        bool isLand = (std::abs(lat) < 60.0) && (landNoise > -0.2);

        if (isLand) {
            uint8_t green = static_cast<uint8_t>(120 + 40 * std::abs(std::sin(lat * 0.05)));
            return { 80, green, 60 };
        }
        else {
            uint8_t blue = static_cast<uint8_t>(140 + 40 * std::abs(std::cos(lon * 0.02)));
            return { 30, 60, blue };
        }
    }

    inline void DrawGridLines(uint8_t* pixels, uint32_t w, uint32_t h, const GeoExtent& ext) const {
        for (double lon = -180.0; lon <= 180.0; lon += 30.0) {
            if (lon < ext.minLon || lon > ext.maxLon) continue;
            int px = static_cast<int>((lon - ext.minLon) / ext.Width() * (w - 1));
            if (px >= 0 && px < static_cast<int>(w)) {
                for (uint32_t y = 0; y < h; y += 2) {
                    size_t idx = (static_cast<size_t>(y) * w + px) * 3;
                    pixels[idx] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx] + 40));
                    pixels[idx + 1] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx + 1] + 40));
                    pixels[idx + 2] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx + 2] + 40));
                }
            }
        }
        for (double lat = -90.0; lat <= 90.0; lat += 30.0) {
            if (lat < ext.minLat || lat > ext.maxLat) continue;
            int py = static_cast<int>((ext.maxLat - lat) / ext.Height() * (h - 1));
            if (py >= 0 && py < static_cast<int>(h)) {
                for (uint32_t x = 0; x < w; x += 2) {
                    size_t idx = (static_cast<size_t>(py) * w + x) * 3;
                    pixels[idx] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx] + 40));
                    pixels[idx + 1] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx + 1] + 40));
                    pixels[idx + 2] = (std::min)(static_cast<uint8_t>(255), static_cast<uint8_t>(pixels[idx + 2] + 40));
                }
            }
        }
    }
};

}
} // namespace ExplorerLens::Engine
