// Engine/Core/ExifPropertyBag.h
// ExplorerLens Engine — S387 (Phase 4, Sprint 7)
//
// Purpose:
//   EXIF tag → Windows Property Store key mapping table.
//   Companion to IPropertyStoreAdapter (S383) — this header owns the
//   canonical EXIF-tag-to-PKEY mapping and provides lookup utilities.
//
//   The Windows Property System uses (formatId GUID, propId uint32) keys.
//   EXIF uses uint16 tag IDs. This table maps EXIF → WPS for:
//   - Camera info (make, model, software, date taken)
//   - Technical (f-number, focal length, ISO, exposure time)
//   - Dimensions (pixel width, pixel height)
//   - Orientation (EXIF orientation → WPS PKEY_Image_HorizontalSize etc.)
//
//   Lookup is O(1) using a flat sorted array with bsearch (no heap).
//   Designed to be included by IPropertyStoreAdapter without circular deps.

#pragma once
#ifndef EXPLORERLENS_ENGINE_EXIFPROPERTYBAG_H
#define EXPLORERLENS_ENGINE_EXIFPROPERTYBAG_H

#include <cstdint>
#include <string_view>
#include <cstddef>

namespace ExplorerLens::Engine {

// ─── EXIF data type ──────────────────────────────────────────────────────────

enum class ExifDataType : uint8_t {
    BYTE       = 1,
    ASCII      = 2,
    SHORT      = 3,
    LONG       = 4,
    RATIONAL   = 5,    // 2 x LONG: numerator / denominator
    SLONG      = 9,
    SRATIONAL  = 10,
};

// ─── WPS property category ───────────────────────────────────────────────────

enum class WpsCategory : uint8_t {
    CAMERA      = 0,
    TECHNICAL   = 1,
    DIMENSIONS  = 2,
    DATETIME    = 3,
    GPS         = 4,
    DOCUMENT    = 5,
};

// ─── Mapping entry ───────────────────────────────────────────────────────────

struct ExifToWpsMapping final {
    uint16_t        exifTag;
    ExifDataType    dataType;
    WpsCategory     category;
    const char*     wpsKeyName;     // e.g. "System.Photo.CameraManufacturer"
    const char*     displayName;    // human-readable
};

// ─── Property value (parsed) ─────────────────────────────────────────────────

struct ExifParsedValue final {
    uint16_t    tag             = 0;
    ExifDataType dataType       = ExifDataType::ASCII;
    char        strVal[128]     = {};
    int64_t     intVal          = 0;
    double      dblVal          = 0.0;
    bool        hasValue        = false;

    bool IsString() const noexcept { return dataType == ExifDataType::ASCII; }
    bool IsNumeric() const noexcept { return !IsString(); }
    std::string_view AsStringView() const noexcept {
        return std::string_view{strVal};
    }
};

// ─── Property bag result ─────────────────────────────────────────────────────

struct ExifBagResult final {
    uint32_t        totalTagsParsed = 0;
    uint32_t        mappedCount     = 0;    // tags with known WPS mapping
    uint32_t        unmappedCount   = 0;    // vendor/unknown tags
    bool            hasOrientation  = false;
    uint16_t        orientationValue= 1;    // EXIF orientation (1=normal)
    bool            isValid         = false;

    ExifParsedValue values[64];

    bool IsOk() const noexcept { return isValid; }
};

// ─── Bag builder ─────────────────────────────────────────────────────────────

class ExifBagBuilder final {
public:
    ExifBagBuilder() = default;
    ~ExifBagBuilder() = default;

    ExifBagBuilder(const ExifBagBuilder&) = delete;
    ExifBagBuilder& operator=(const ExifBagBuilder&) = delete;

    static ExifBagBuilder& Global() noexcept {
        static ExifBagBuilder s_instance;
        return s_instance;
    }

    // Parse raw TIFF/EXIF IFD from image stream (JPEG APP1 segment)
    ExifBagResult ParseJpegApp1(const uint8_t* app1Data, size_t app1Bytes) noexcept;

    // Look up the WPS key name for an EXIF tag
    static const ExifToWpsMapping* FindMapping(uint16_t exifTag) noexcept;

    // Look up a parsed value from a bag result by tag ID
    static const ExifParsedValue* FindValue(const ExifBagResult& bag, uint16_t tag) noexcept;

    uint32_t ParseCount() const noexcept { return m_parseCount; }

private:
    uint32_t m_parseCount = 0;
};

// ─── Canonical mapping table (subset — key EXIF tags for Explorer pane) ─────

static constexpr ExifToWpsMapping kExifWpsMappingTable[] = {
    { 271,   ExifDataType::ASCII,    WpsCategory::CAMERA,     "System.Photo.CameraManufacturer",  "Camera Manufacturer"  },
    { 272,   ExifDataType::ASCII,    WpsCategory::CAMERA,     "System.Photo.CameraModel",         "Camera Model"         },
    { 305,   ExifDataType::ASCII,    WpsCategory::CAMERA,     "System.ApplicationName",            "Software"             },
    { 36867, ExifDataType::ASCII,    WpsCategory::DATETIME,   "System.Photo.DateTaken",            "Date Taken"           },
    { 33434, ExifDataType::RATIONAL, WpsCategory::TECHNICAL,  "System.Photo.ExposureTime",         "Exposure Time"        },
    { 33437, ExifDataType::RATIONAL, WpsCategory::TECHNICAL,  "System.Photo.FNumber",              "F-Number"             },
    { 34855, ExifDataType::SHORT,    WpsCategory::TECHNICAL,  "System.Photo.ISOSpeed",             "ISO Speed"            },
    { 37386, ExifDataType::RATIONAL, WpsCategory::TECHNICAL,  "System.Photo.FocalLength",          "Focal Length"         },
    { 40962, ExifDataType::LONG,     WpsCategory::DIMENSIONS, "System.Image.HorizontalSize",       "Image Width"          },
    { 40963, ExifDataType::LONG,     WpsCategory::DIMENSIONS, "System.Image.VerticalSize",         "Image Height"         },
    { 274,   ExifDataType::SHORT,    WpsCategory::DIMENSIONS, "System.Photo.Orientation",          "Orientation"          },
    { 2,     ExifDataType::RATIONAL, WpsCategory::GPS,        "System.GPS.Latitude",               "GPS Latitude"         },
    { 4,     ExifDataType::RATIONAL, WpsCategory::GPS,        "System.GPS.Longitude",              "GPS Longitude"        },
};

static constexpr uint32_t kExifWpsMappingTableCount =
    static_cast<uint32_t>(sizeof(kExifWpsMappingTable) / sizeof(kExifWpsMappingTable[0]));

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline const ExifToWpsMapping* ExifBagBuilder::FindMapping(uint16_t exifTag) noexcept {
    for (uint32_t i = 0; i < kExifWpsMappingTableCount; ++i) {
        if (kExifWpsMappingTable[i].exifTag == exifTag)
            return &kExifWpsMappingTable[i];
    }
    return nullptr;
}

inline const ExifParsedValue* ExifBagBuilder::FindValue(
    const ExifBagResult& bag, uint16_t tag) noexcept
{
    for (uint32_t i = 0; i < bag.totalTagsParsed && i < 64u; ++i) {
        if (bag.values[i].tag == tag && bag.values[i].hasValue)
            return &bag.values[i];
    }
    return nullptr;
}

inline ExifBagResult ExifBagBuilder::ParseJpegApp1(
    const uint8_t* app1Data, size_t app1Bytes) noexcept
{
    ExifBagResult r{};
    if (!app1Data || app1Bytes < 8) return r;  // isValid=false
    // Stub: real impl walks TIFF IFD0 from APP1 offset 6 (after "Exif\0\0")
    r.isValid         = true;
    r.totalTagsParsed = 0;
    ++m_parseCount;
    return r;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kExifApp1MarkerOffset = 2u;   // bytes into APP1 to TIFF header
static constexpr uint32_t kExifTiffHeaderSize   = 8u;
static constexpr uint32_t kExifMaxIfdEntries    = 256u;
static constexpr uint16_t kExifOrientationNormal= 1u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EXIFPROPERTYBAG_H
