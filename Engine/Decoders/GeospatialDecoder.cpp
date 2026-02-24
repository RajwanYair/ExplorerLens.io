//==============================================================================
// GeospatialDecoder
//==============================================================================

#include "GeospatialDecoder.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ExplorerLens { namespace Engine {

GeospatialDecoder::GeospatialDecoder() {}

bool GeospatialDecoder::IsGeoFile(const uint8_t* data, size_t size) {
    return DetectFormat(data, size) != GeoFormat::Unknown;
}

GeoFormat GeospatialDecoder::DetectFormat(const uint8_t* data, size_t size) {
    if (!data || size < 4) return GeoFormat::Unknown;
    // GeoTIFF: TIFF magic + GeoKeys
    if ((data[0] == 'I' && data[1] == 'I' && data[2] == 42 && data[3] == 0) ||
        (data[0] == 'M' && data[1] == 'M' && data[2] == 0 && data[3] == 42)) {
        return GeoFormat::GeoTIFF;
    }
    // KML/KMZ
    if (data[0] == 'P' && data[1] == 'K') return GeoFormat::KMZ;
    if (size >= 5 && memcmp(data, "<?xml", 5) == 0) {
        std::string content(reinterpret_cast<const char*>(data),
                           std::min(size, (size_t)512));
        if (content.find("<kml") != std::string::npos) return GeoFormat::KML;
    }
    // GeoJSON
    if (data[0] == '{') {
        std::string content(reinterpret_cast<const char*>(data),
                           std::min(size, (size_t)256));
        if (content.find("\"type\"") != std::string::npos &&
            (content.find("FeatureCollection") != std::string::npos ||
             content.find("Feature") != std::string::npos)) {
            return GeoFormat::GeoJSON;
        }
    }
    // Shapefile (.shp): magic bytes 0x0000270a
    if (size >= 4 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x27 && data[3] == 0x0a) {
        return GeoFormat::Shapefile;
    }
    return GeoFormat::Unknown;
}

uint32_t GeospatialDecoder::GetExtensionCount() {
    return 8;
}

std::vector<std::wstring> GeospatialDecoder::GetExtensions() {
    return { L".tif", L".tiff", L".shp", L".kml", L".kmz", L".geojson", L".gpx", L".prj" };
}

GeoLayerInfo GeospatialDecoder::ReadLayerInfo(const uint8_t* data, size_t size) const {
    GeoLayerInfo info;
    info.format = DetectFormat(data, size);
    info.projection = L"EPSG:4326";
    return info;
}

double GeospatialDecoder::DistanceKm(const GeoCoordinate& a, const GeoCoordinate& b) {
    const double R = 6371.0; // Earth radius in km
    double dLat = (b.latitude - a.latitude) * M_PI / 180.0;
    double dLon = (b.longitude - a.longitude) * M_PI / 180.0;
    double lat1 = a.latitude * M_PI / 180.0;
    double lat2 = b.latitude * M_PI / 180.0;
    double aVal = sin(dLat / 2) * sin(dLat / 2) +
                  cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(aVal), sqrt(1 - aVal));
    return R * c;
}

GeoCoordinate GeospatialDecoder::MercatorToWGS84(double x, double y) {
    GeoCoordinate coord;
    coord.longitude = x / 20037508.34 * 180.0;
    coord.latitude = 180.0 / M_PI * (2 * atan(exp(y / 20037508.34 * M_PI)) - M_PI / 2);
    return coord;
}

const wchar_t* GeospatialDecoder::GetFormatName(GeoFormat format) {
    switch (format) {
        case GeoFormat::GeoTIFF:   return L"GeoTIFF";
        case GeoFormat::Shapefile: return L"Shapefile";
        case GeoFormat::KML:       return L"KML";
        case GeoFormat::KMZ:       return L"KMZ";
        case GeoFormat::GeoJSON:   return L"GeoJSON";
        case GeoFormat::GPX:       return L"GPX";
        default: return L"Unknown";
    }
}

const wchar_t* GeospatialDecoder::GetProjectionName(GeoProjection proj) {
    switch (proj) {
        case GeoProjection::WGS84:        return L"WGS 84 (EPSG:4326)";
        case GeoProjection::WebMercator:  return L"Web Mercator (EPSG:3857)";
        case GeoProjection::UTM:          return L"UTM";
        case GeoProjection::Custom:       return L"Custom";
        default: return L"Unknown";
    }
}

bool GeospatialDecoder::ParseGeoJSON(const uint8_t* data, size_t size, GeoLayerInfo& info) const {
    if (!data || size == 0) return false;
    info.format = GeoFormat::GeoJSON;
    return true;
}

}} // namespace ExplorerLens::Engine

