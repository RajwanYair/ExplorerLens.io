// Engine/Core/IPropertyStoreAdapter.h
// ExplorerLens Engine — S383 (Phase 4, Sprint 3)
//
// Purpose:
//   IPropertyStore adapter for EXIF metadata in the Explorer details pane.
//   Phase 4 exit criterion: "IPropertyStore for EXIF in Explorer details pane (H46)"
//
//   Windows Explorer shows a "Details" pane for selected files. For images,
//   it reads IPropertyStore to display: camera make/model, date taken,
//   GPS location, focal length, ISO, aperture, exposure time, dimensions.
//
//   This header defines:
//   - PropertyKey constants (PKEY_* equivalents, GUID + PID pairs)
//   - ExifPropertyRecord: decoded EXIF tag → Windows property key mapping
//   - IPropertyStoreAdapter: COM-accessible adapter wrapping parsed EXIF tags
//   - ExifPropertyExtractor: extracts EXIF from a raw JPEG/HEIC/TIFF stream
//
//   Integration: LENSShell registers IPropertyStore via the handler registry
//   at the same CLSID as the IThumbnailProvider.

#pragma once
#ifndef EXPLORERLENS_ENGINE_IPROPERTYSTOREADAPTER_H
#define EXPLORERLENS_ENGINE_IPROPERTYSTOREADAPTER_H

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Status ──────────────────────────────────────────────────────────────────

enum class PropertyStoreStatus : uint8_t {
    OK                   = 0,
    NO_EXIF              = 1,   // file has no EXIF data
    PARSE_ERROR          = 2,
    PROPERTY_NOT_FOUND   = 3,
    BUFFER_TOO_SMALL     = 4,
    NULL_STREAM          = 5,
    NOT_WIN32            = 6,
};

// ─── Property type ───────────────────────────────────────────────────────────

enum class WpsPropertyType : uint8_t {
    STRING    = 0,
    UINT32    = 1,
    UINT64    = 2,
    DOUBLE    = 3,
    DATE_TIME = 4,  // FILETIME encoded as uint64
    GPS_COORD = 5,  // latitude/longitude as double
};

// ─── Well-known property keys (PKEY equivalents) ─────────────────────────────

struct WpsPropertyKey final {
    uint32_t    fmtId[4];  // GUID as 4 x uint32 (avoids Windows.h include)
    uint32_t    pid;

    bool operator==(const WpsPropertyKey& o) const noexcept {
        return fmtId[0] == o.fmtId[0] && fmtId[1] == o.fmtId[1] &&
               fmtId[2] == o.fmtId[2] && fmtId[3] == o.fmtId[3] &&
               pid == o.pid;
    }
};

// ─── Property record ─────────────────────────────────────────────────────────

struct ExifPropertyRecord final {
    WpsPropertyKey  key;
    WpsPropertyType type            = WpsPropertyType::STRING;
    char            strValue[128]   = {};
    uint64_t        numValue        = 0;
    double          dblValue        = 0.0;

    bool HasValue() const noexcept { return strValue[0] != '\0' || numValue != 0; }
};

// ─── EXIF extractor config ────────────────────────────────────────────────────

struct ExifExtractorConfig final {
    bool extractCameraInfo  = true;
    bool extractGps         = false;   // GPS requires privacy consent
    bool extractDimensions  = true;
    bool extractDateTaken   = true;
    bool extractTechnical   = true;    // aperture, ISO, focal length
    uint32_t maxTagsToRead  = 64u;

    static constexpr ExifExtractorConfig Default() noexcept {
        return ExifExtractorConfig{};
    }

    static constexpr ExifExtractorConfig ExplorerPane() noexcept {
        ExifExtractorConfig c{};
        c.extractCameraInfo  = true;
        c.extractGps         = false;
        c.extractDimensions  = true;
        c.extractDateTaken   = true;
        c.extractTechnical   = true;
        return c;
    }
};

// ─── Property bag (extraction result) ────────────────────────────────────────

struct ExifPropertyBag final {
    PropertyStoreStatus status          = PropertyStoreStatus::OK;
    uint32_t            tagCount        = 0;
    ExifPropertyRecord  records[64];    // stack allocation, no heap

    // Camera info
    char  cameraMake[64]    = {};
    char  cameraModel[64]   = {};
    char  dateTaken[32]     = {};
    char  software[64]      = {};

    // Technical
    uint32_t widthPx        = 0;
    uint32_t heightPx       = 0;
    uint32_t isoSpeed       = 0;
    double   apertureF      = 0.0;
    double   focalLengthMm  = 0.0;
    double   exposureSec    = 0.0;

    bool IsOk()    const noexcept { return status == PropertyStoreStatus::OK; }
    bool HasExif() const noexcept { return tagCount > 0; }
};

// ─── Adapter class ───────────────────────────────────────────────────────────

class IPropertyStoreAdapter final {
public:
    IPropertyStoreAdapter() = default;
    ~IPropertyStoreAdapter() = default;

    IPropertyStoreAdapter(const IPropertyStoreAdapter&) = delete;
    IPropertyStoreAdapter& operator=(const IPropertyStoreAdapter&) = delete;

    static IPropertyStoreAdapter& Global() noexcept {
        static IPropertyStoreAdapter s_instance;
        return s_instance;
    }

    void Configure(const ExifExtractorConfig& config) noexcept { m_config = config; }

    // Extract EXIF metadata from a raw image stream (JPEG / HEIC / TIFF)
    ExifPropertyBag Extract(
        const uint8_t* imageData,
        size_t         imageBytes,
        const char*    extension = nullptr) noexcept;

    // Get a specific property by EXIF tag ID
    PropertyStoreStatus GetByTagId(
        const ExifPropertyBag& bag,
        uint16_t               tagId,
        ExifPropertyRecord&    outRecord) const noexcept;

    // Count of successful extractions since construction
    uint32_t ExtractionCount() const noexcept { return m_extractionCount; }

    const ExifExtractorConfig& Config() const noexcept { return m_config; }

private:
    ExifExtractorConfig m_config{};
    uint32_t            m_extractionCount = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline ExifPropertyBag IPropertyStoreAdapter::Extract(
    const uint8_t* imageData,
    size_t         imageBytes,
    const char*    /*extension*/) noexcept
{
    ExifPropertyBag bag{};
#ifndef _WIN32
    bag.status = PropertyStoreStatus::NOT_WIN32;
    return bag;
#else
    if (!imageData || imageBytes < 4) {
        bag.status = PropertyStoreStatus::NULL_STREAM;
        return bag;
    }
    // Stub: real impl uses WIC IWICMetadataQueryReader or libexif
    bag.status   = PropertyStoreStatus::NO_EXIF;
    bag.tagCount = 0;
    ++m_extractionCount;
    return bag;
#endif
}

inline PropertyStoreStatus IPropertyStoreAdapter::GetByTagId(
    const ExifPropertyBag& bag,
    uint16_t               /*tagId*/,
    ExifPropertyRecord&    outRecord) const noexcept
{
    if (!bag.HasExif()) return PropertyStoreStatus::NO_EXIF;
    outRecord = ExifPropertyRecord{};
    return PropertyStoreStatus::PROPERTY_NOT_FOUND;
}

// ─── Well-known property key constants (PKEY_Photo_* equivalents) ─────────────

// These mirror Windows SDK PKEY_Photo_CameraManufacturer etc.
// Defined here without windows.h to keep the header portable.
static constexpr WpsPropertyKey kPKeyPhotoCameraManufacturer = {
    {0x14B81DA1u, 0x0135u, 0x4D31u, 0xB7E7EE0F7D3u}, 271u
};
static constexpr WpsPropertyKey kPKeyPhotoCameraModel = {
    {0x14B81DA1u, 0x0135u, 0x4D31u, 0xB7E7EE0F7D3u}, 272u
};
static constexpr WpsPropertyKey kPKeyPhotoDateTaken = {
    {0x14B81DA1u, 0x0135u, 0x4D31u, 0xB7E7EE0F7D3u}, 36867u
};
static constexpr WpsPropertyKey kPKeyPhotoISOSpeed = {
    {0x14B81DA1u, 0x0135u, 0x4D31u, 0xB7E7EE0F7D3u}, 34855u
};

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kExifMaxTagsRead          = 64u;
static constexpr uint32_t kExifCameraModelMaxLen     = 64u;
static constexpr uint16_t kExifTagMake               = 271u;
static constexpr uint16_t kExifTagModel              = 272u;
static constexpr uint16_t kExifTagDateTimeOriginal   = 36867u;
static constexpr uint16_t kExifTagISOSpeedRatings    = 34855u;
static constexpr uint16_t kExifTagFNumber            = 33437u;
static constexpr uint16_t kExifTagFocalLength        = 37386u;
static constexpr uint16_t kExifTagExposureTime       = 33434u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_IPROPERTYSTOREADAPTER_H
