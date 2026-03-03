// GeoTagMapThumbnail.h — Geographic Math for Map-Like Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Pure geographic math utilities for generating map-like thumbnails from
// GPS coordinates embedded in photos. Mercator projection, tile coordinates,
// distance computation. No HTTP or network I/O.
//
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// WGS-84 constants
inline constexpr double kEarthRadiusKm = 6371.0;
inline constexpr double kPI = 3.14159265358979323846;

// Mercator projection output
struct MercatorPoint {
    double x = 0.0;   // pixel x at the given zoom level
    double y = 0.0;   // pixel y at the given zoom level
    bool   valid = false;
};

// Tile coordinate within a slippy-map grid
struct TileCoord {
    uint32_t tileX = 0;
    uint32_t tileY = 0;
    uint32_t zoom = 0;
    bool     valid = false;
};

class GeoTagMapThumbnail {
public:
    // Store coordinates for the current photo
    void SetCoordinates(double latitude, double longitude) noexcept {
        m_latitude = latitude;
        m_longitude = longitude;
        m_hasCoords = IsValidCoordinate(latitude, longitude);
    }

    double GetLatitude()  const noexcept { return m_latitude; }
    double GetLongitude() const noexcept { return m_longitude; }
    bool   HasCoordinates() const noexcept { return m_hasCoords; }

    // Web-Mercator projection (EPSG:3857) returning pixel coordinates
    // for a 256-pixel tile size at the given zoom level.
    static MercatorPoint GetMercatorProjection(double lat,
        double lon,
        uint32_t zoom) noexcept {
        MercatorPoint pt{};
        if (!IsValidCoordinate(lat, lon))
            return pt;

        // Clamp zoom to reasonable range [0, 23]
        zoom = (std::min)(zoom, 23u);

        const double n = static_cast<double>(1u << zoom);
        const double latRad = lat * kPI / 180.0;

        pt.x = (lon + 180.0) / 360.0 * n * 256.0;
        pt.y = (1.0 - std::log(std::tan(latRad) +
            1.0 / std::cos(latRad)) / kPI) / 2.0 * n * 256.0;
        pt.valid = true;
        return pt;
    }

    // Return the slippy-map tile coordinate for a lat/lon at a zoom level.
    static TileCoord GetTileCoord(double lat,
        double lon,
        uint32_t zoom) noexcept {
        TileCoord tc{};
        if (!IsValidCoordinate(lat, lon))
            return tc;

        zoom = (std::min)(zoom, 23u);
        const double n = static_cast<double>(1u << zoom);
        const double latRad = lat * kPI / 180.0;

        tc.tileX = static_cast<uint32_t>((lon + 180.0) / 360.0 * n);
        tc.tileY = static_cast<uint32_t>(
            (1.0 - std::log(std::tan(latRad) +
                1.0 / std::cos(latRad)) / kPI) / 2.0 * n);

        // Clamp to valid tile range
        const uint32_t maxTile = (1u << zoom) - 1;
        tc.tileX = (std::min)(tc.tileX, maxTile);
        tc.tileY = (std::min)(tc.tileY, maxTile);
        tc.zoom = zoom;
        tc.valid = true;
        return tc;
    }

    // Check whether a latitude/longitude pair is within valid WGS-84 bounds.
    // Mercator is invalid beyond ~85.05 degrees, but we accept full [-90,90].
    static bool IsValidCoordinate(double lat, double lon) noexcept {
        if (std::isnan(lat) || std::isnan(lon))
            return false;
        if (std::isinf(lat) || std::isinf(lon))
            return false;
        return (lat >= -90.0 && lat <= 90.0) &&
            (lon >= -180.0 && lon <= 180.0);
    }

    // Haversine distance between two GPS points, in kilometres.
    static double GetDistanceKm(double lat1, double lon1,
        double lat2, double lon2) noexcept {
        if (!IsValidCoordinate(lat1, lon1) || !IsValidCoordinate(lat2, lon2))
            return -1.0;

        const double dLat = (lat2 - lat1) * kPI / 180.0;
        const double dLon = (lon2 - lon1) * kPI / 180.0;

        const double rLat1 = lat1 * kPI / 180.0;
        const double rLat2 = lat2 * kPI / 180.0;

        const double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
            std::cos(rLat1) * std::cos(rLat2) *
            std::sin(dLon / 2.0) * std::sin(dLon / 2.0);

        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return kEarthRadiusKm * c;
    }

private:
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    bool   m_hasCoords = false;
};

} // namespace Engine
} // namespace ExplorerLens
