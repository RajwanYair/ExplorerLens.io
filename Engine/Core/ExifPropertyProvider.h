// =============================================================================
// ExplorerLens Engine — ExifPropertyProvider.h
// Sprint S356 | ROADMAP v8.0 Phase 4 (IPropertyStore EXIF — H46)
// IPropertyStore adapter for surfacing EXIF metadata in the Windows Explorer
// details pane / preview pane property handler.
//
// Phase 4 exit criterion: "IPropertyStore for EXIF in Explorer details pane (H46)"
// This header defines the data contract and property mapping tables that
// ExifPropertyStoreProvider.cpp uses to implement IPropertyStore::GetValue().
//
// EXIF properties are mapped to Windows Property System PKEYs:
//   EXIF_DateTimeOriginal → PKEY_Photo_DateTaken
//   EXIF_Make + Model     → PKEY_Photo_CameraModel
//   EXIF_ExposureTime     → PKEY_Photo_ExposureTime
//   EXIF_ISOSpeedRatings  → PKEY_Photo_ISOSpeed
//   EXIF_FNumber          → PKEY_Photo_Aperture
//   EXIF_FocalLength      → PKEY_Photo_FocalLength
//   EXIF_GPSLatitude/Long → PKEY_GPS_Latitude / Longitude
//   Width/Height          → PKEY_Image_HorizontalSize / VerticalSize
// =============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#ifndef EXPLORERLENS_ENGINE_EXIFPROPERTYPROVIDER_H
#define EXPLORERLENS_ENGINE_EXIFPROPERTYPROVIDER_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// ExifPropertyId — well-known EXIF tag codes (subset relevant for Explorer)
// ---------------------------------------------------------------------------
enum class ExifPropertyId : uint32_t {
    MAKE                    = 271u,   ///< Camera manufacturer
    MODEL                   = 272u,   ///< Camera model
    SOFTWARE                = 305u,   ///< Processing software
    DATE_TIME               = 306u,   ///< Last modified time
    ARTIST                  = 315u,   ///< Author
    IMAGE_WIDTH             = 256u,   ///< Pixel width
    IMAGE_HEIGHT            = 257u,   ///< Pixel height
    BITS_PER_SAMPLE         = 258u,   ///< Bit depth per channel
    ORIENTATION             = 274u,   ///< EXIF rotation/flip
    X_RESOLUTION            = 282u,
    Y_RESOLUTION            = 283u,
    COPYRIGHT               = 33432u,
    EXPOSURE_TIME           = 33434u, ///< Shutter speed in seconds
    F_NUMBER                = 33437u, ///< Aperture
    ISO_SPEED               = 34855u, ///< ISO sensitivity
    DATE_TIME_ORIGINAL      = 36867u, ///< Capture time
    DATE_TIME_DIGITIZED     = 36868u,
    FOCAL_LENGTH            = 37386u, ///< Focal length in mm
    FLASH                   = 37385u, ///< Flash fired?
    EXPOSURE_PROGRAM        = 34850u,
    METERING_MODE           = 37383u,
    WHITE_BALANCE           = 41987u,
    FOCAL_LENGTH_35MM       = 37396u,
    GPS_LATITUDE            = 2u,     ///< Within GPS IFD
    GPS_LONGITUDE           = 4u,     ///< Within GPS IFD
    GPS_ALTITUDE            = 6u,
    UNKNOWN                 = 0u,
};

// ---------------------------------------------------------------------------
// ExifValueType — TIFF/EXIF field type codes
// ---------------------------------------------------------------------------
enum class ExifValueType : uint8_t {
    BYTE      = 1,
    ASCII     = 2,
    SHORT     = 3,
    LONG      = 4,
    RATIONAL  = 5,
    SBYTE     = 6,
    UNDEFINED = 7,
    SSHORT    = 8,
    SLONG     = 9,
    SRATIONAL = 10,
    FLOAT     = 11,
    DOUBLE    = 12,
};

// ---------------------------------------------------------------------------
// ExifRational — unsigned rational (numerator / denominator)
// ---------------------------------------------------------------------------
struct ExifRational final {
    uint32_t numerator{0};
    uint32_t denominator{1};

    [[nodiscard]] double ToDouble() const noexcept {
        if (denominator == 0u) return 0.0;
        return static_cast<double>(numerator) / static_cast<double>(denominator);
    }
};

// ---------------------------------------------------------------------------
// ExifPropertyRecord — a single decoded EXIF tag value
// ---------------------------------------------------------------------------
struct ExifPropertyRecord final {
    ExifPropertyId  id{ExifPropertyId::UNKNOWN};
    ExifValueType   valueType{ExifValueType::ASCII};
    std::wstring    stringValue;    ///< For ASCII / string tags
    ExifRational    rationalValue;  ///< For RATIONAL tags
    uint32_t        uintValue{0};   ///< For SHORT / LONG tags
    int32_t         sintValue{0};   ///< For SLONG tags
    bool            hasValue{false};
};

// ---------------------------------------------------------------------------
// ExifPropertySet — collection of decoded EXIF records for one image
// ---------------------------------------------------------------------------
struct ExifPropertySet final {
    std::vector<ExifPropertyRecord> records;
    bool parsedSuccessfully{false};

    [[nodiscard]] const ExifPropertyRecord* Find(ExifPropertyId id) const noexcept {
        for (const auto& r : records) {
            if (r.id == id && r.hasValue) return &r;
        }
        return nullptr;
    }

    [[nodiscard]] std::wstring GetString(ExifPropertyId id) const noexcept {
        const auto* r = Find(id);
        return r ? r->stringValue : L"";
    }

    [[nodiscard]] bool HasGps() const noexcept {
        return Find(ExifPropertyId::GPS_LATITUDE) != nullptr;
    }
};

// ---------------------------------------------------------------------------
// ExifPropertyProviderStatus — result codes for property extraction
// ---------------------------------------------------------------------------
enum class ExifPropertyProviderStatus : uint8_t {
    OK               = 0,
    NO_EXIF          = 1, ///< File has no EXIF IFD
    PARSE_ERROR      = 2, ///< EXIF data is malformed
    NULL_BUFFER      = 3, ///< Input buffer is null or empty
    FORMAT_UNSUPPORTED = 4, ///< File format does not support EXIF
    TRUNCATED        = 5, ///< EXIF data was truncated (still returns partial)
};

// ---------------------------------------------------------------------------
// ExifPropertyProviderConfig — parser settings
// ---------------------------------------------------------------------------
struct ExifPropertyProviderConfig final {
    bool parseGps{true};       ///< Include GPS tags
    bool parseMakerNote{false}; ///< Skip proprietary maker notes (large + complex)
    bool strictMode{false};    ///< Reject files with any parse error
    uint32_t maxExifBytes{128u * 1024u}; ///< Reject EXIF segments > 128 KB

    [[nodiscard]] static ExifPropertyProviderConfig Default() noexcept {
        return ExifPropertyProviderConfig{};
    }
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kExifHeaderMagicJpeg = 0xFFE1u; ///< JPEG APP1 marker
static constexpr uint8_t  kExifLittleEndian[2] = {'I', 'I'};
static constexpr uint8_t  kExifBigEndian[2]    = {'M', 'M'};
static constexpr uint32_t kExifMaxProperties   = 256u;
static constexpr uint32_t kExifMaxExifBytes    = 128u * 1024u; ///< 128 KB

// ---------------------------------------------------------------------------
// ExifPropertyProvider — static EXIF parser + Windows property mapper
// ---------------------------------------------------------------------------
class ExifPropertyProvider final {
public:
    ExifPropertyProvider() = delete;

    /// Parse EXIF data from a raw byte buffer (JPEG APP1 segment body).
    /// @param exifData    Pointer to EXIF segment data (after APP1 marker)
    /// @param exifSize    Byte length of EXIF segment
    /// @param cfg         Parser configuration
    /// @param outStatus   Populated with detailed status code
    /// @return            Populated ExifPropertySet
    [[nodiscard]] static ExifPropertySet ParseExif(
        const uint8_t* exifData,
        uint32_t       exifSize,
        const ExifPropertyProviderConfig& cfg,
        ExifPropertyProviderStatus& outStatus) noexcept;

    /// Convenience: parse EXIF from a JPEG file byte buffer.
    [[nodiscard]] static ExifPropertySet ParseFromJpegBytes(
        const uint8_t* jpegData,
        uint32_t       jpegSize,
        ExifPropertyProviderStatus& outStatus) noexcept;

    /// Map an ExifPropertySet to a flat property bag for IPropertyStore.
    /// Returns an opaque void* pointer to an engine-internal PropertyBag
    /// that the IPropertyStore implementation can consume.
    /// Caller must release via FreePropertyBag().
    [[nodiscard]] static void* BuildPropertyBag(
        const ExifPropertySet& props) noexcept;

    /// Release a PropertyBag allocated by BuildPropertyBag().
    static void FreePropertyBag(void* bag) noexcept;

    /// Returns the Windows PKEY GUID string for a known ExifPropertyId.
    [[nodiscard]] static std::wstring GetPKeyGuid(ExifPropertyId id) noexcept;

private:
    [[nodiscard]] static bool IsLittleEndian(
        const uint8_t* header) noexcept;
    [[nodiscard]] static uint32_t ReadUInt32(
        const uint8_t* p, bool littleEndian) noexcept;
    [[nodiscard]] static uint16_t ReadUInt16(
        const uint8_t* p, bool littleEndian) noexcept;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EXIFPROPERTYPROVIDER_H
