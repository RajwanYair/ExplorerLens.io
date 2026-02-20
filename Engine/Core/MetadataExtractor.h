#pragma once
// =============================================================================
// MetadataExtractor.h — Sprint 242: EXIF/IPTC/XMP Metadata Extraction
// DarkThumbs Engine — Core Module
// =============================================================================

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace DarkThumbs {

/// Metadata standard type
enum class MetadataStandard : uint32_t {
    EXIF    = 0,   ///< Exchangeable Image File Format
    IPTC    = 1,   ///< International Press Telecommunications Council
    XMP     = 2,   ///< Extensible Metadata Platform
    ICC     = 3,   ///< ICC Color Profile
    GPS     = 4,   ///< GPS/Location data subset of EXIF
    Count   = 5
};

/// Common metadata field identifiers
enum class MetadataField : uint32_t {
    Title           = 0,
    Author          = 1,
    Copyright       = 2,
    DateCreated     = 3,
    DateModified    = 4,
    CameraMake      = 5,
    CameraModel     = 6,
    FocalLength     = 7,
    Aperture        = 8,
    ISO             = 9,
    ExposureTime    = 10,
    GPSLatitude     = 11,
    GPSLongitude    = 12,
    Width           = 13,
    Height          = 14,
    ColorSpace      = 15,
    Count           = 16
};

/// A single metadata tag entry
struct MetadataTag {
    MetadataStandard    standard    = MetadataStandard::EXIF;
    MetadataField       field       = MetadataField::Title;
    std::wstring        name;
    std::wstring        value;
    uint32_t            tagId       = 0;    ///< Raw tag ID (e.g. 0x010F for Make)
};

/// Extraction result
struct ExtractionResult {
    bool        success     = false;
    uint32_t    tagCount    = 0;
    std::wstring filePath;
    std::wstring errorMessage;
    std::vector<MetadataTag> tags;
};

/// MetadataExtractor — extracts image metadata from various standards
class MetadataExtractor {
public:
    MetadataExtractor();

    // Extraction
    ExtractionResult Extract(const std::wstring& filePath);
    ExtractionResult ExtractStandard(const std::wstring& filePath, MetadataStandard standard);

    // Tag queries
    std::wstring GetFieldValue(const ExtractionResult& result, MetadataField field) const;
    bool HasField(const ExtractionResult& result, MetadataField field) const;
    std::vector<MetadataTag> GetTagsByStandard(const ExtractionResult& result, MetadataStandard standard) const;

    // Formatting
    static std::wstring FormatGPSCoordinate(double degrees, double minutes, double seconds, wchar_t direction);
    static std::wstring FormatExposureTime(double seconds);

    // Static helpers
    static const wchar_t* GetStandardName(MetadataStandard standard);
    static const wchar_t* GetFieldName(MetadataField field);
    static constexpr uint32_t GetFieldCount() { return static_cast<uint32_t>(MetadataField::Count); }
    static constexpr uint32_t GetStandardCount() { return static_cast<uint32_t>(MetadataStandard::Count); }

private:
    std::vector<MetadataStandard> m_enabledStandards;
};

} // namespace DarkThumbs
