#pragma once
//==============================================================================
// GeospatialDecoder
// Geospatial format decoder (GeoTIFF, Shapefile, KML, GeoJSON)
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class GeoFormat : uint8_t {
    GeoTIFF = 0,
    Shapefile,
    KML,
    KMZ,
    GeoJSON,
    GPX,
    Unknown
};

struct GeoCoordinate {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
};

struct GeoBoundingBox {
    double minLat = 0.0, maxLat = 0.0;
    double minLon = 0.0, maxLon = 0.0;
};

struct GeoLayerInfo {
    std::wstring name;
    GeoFormat format = GeoFormat::Unknown;
    uint32_t featureCount = 0;
    GeoBoundingBox bounds;
    std::wstring projection;  // e.g., "EPSG:4326"
    uint32_t bandCount = 0;   // For raster formats
};

enum class GeoProjection : uint8_t {
    WGS84 = 0,      // EPSG:4326
    WebMercator,     // EPSG:3857
    UTM,
    Custom
};

//------------------------------------------------------------------------------
class GeospatialDecoder {
public:
    GeospatialDecoder();
    ~GeospatialDecoder() = default;

    // Format detection
    static bool IsGeoFile(const uint8_t* data, size_t size);
    static GeoFormat DetectFormat(const uint8_t* data, size_t size);
    static uint32_t GetExtensionCount();
    static std::vector<std::wstring> GetExtensions();

    // Layer info
    GeoLayerInfo ReadLayerInfo(const uint8_t* data, size_t size) const;

    // Coordinate helpers
    static double DistanceKm(const GeoCoordinate& a, const GeoCoordinate& b);
    static GeoCoordinate MercatorToWGS84(double x, double y);

    // Static helpers
    static const wchar_t* GetFormatName(GeoFormat format);
    static const wchar_t* GetProjectionName(GeoProjection proj);

private:
    bool ParseGeoJSON(const uint8_t* data, size_t size, GeoLayerInfo& info) const;
};

}} // namespace ExplorerLens::Engine

