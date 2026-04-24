// ============================================================================
// ShellPropertyStoreContract.h -- S262 / ROADMAP v6.0 F6 IPropertyStore
//
// Phase 3 IPropertyStore contract.  Header-only.  Declares the EXIF / XMP /
// IPTC keys surfaced in Explorer's Details pane so the decoder family owners
// know exactly which property keys to populate.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ShellPropStoreKeyId : uint16_t
{
    // System.Photo.*
    IMAGE_WIDTH              = 1,
    IMAGE_HEIGHT             = 2,
    IMAGE_BIT_DEPTH          = 3,
    IMAGE_HORIZONTAL_DPI     = 4,
    IMAGE_VERTICAL_DPI       = 5,
    IMAGE_COLOR_SPACE        = 6,
    IMAGE_COMPRESSION        = 7,
    PHOTO_CAMERA_MAKE        = 10,
    PHOTO_CAMERA_MODEL       = 11,
    PHOTO_DATE_TAKEN         = 12,
    PHOTO_EXPOSURE_TIME      = 13,
    PHOTO_F_NUMBER           = 14,
    PHOTO_ISO_SPEED          = 15,
    PHOTO_FOCAL_LENGTH       = 16,
    PHOTO_ORIENTATION        = 17,
    PHOTO_FLASH              = 18,
    PHOTO_GPS_LATITUDE       = 19,
    PHOTO_GPS_LONGITUDE      = 20,
    // System.Image.*
    IMAGE_COMMENT            = 30,
    IMAGE_RATING             = 31,
    IMAGE_TITLE              = 32,
    // Decoder-internal
    DECODER_ID               = 100,
    DECODER_VERSION          = 101,
    DECODER_LATENCY_MS       = 102,
};

enum class ShellPropStoreValueType : uint8_t
{
    NONE                     = 0,
    UINT32                   = 1,
    UINT64                   = 2,
    INT32                    = 3,
    DOUBLE                   = 4,
    STRING_UTF8              = 5,
    FILETIME                 = 6,
    RATIONAL                 = 7,   // numerator / denominator (EXIF style)
};

struct ShellPropStoreKey
{
    ShellPropStoreKeyId     id        = ShellPropStoreKeyId::IMAGE_WIDTH;
    ShellPropStoreValueType valueType = ShellPropStoreValueType::NONE;
    bool                    readOnly  = true;
};

struct ShellPropStoreRational
{
    int32_t numerator   = 0;
    int32_t denominator = 1;
};

// Compile-time schema descriptor.  One row per key the provider supports.
struct ShellPropStoreSchemaRow
{
    ShellPropStoreKeyId     id;
    ShellPropStoreValueType valueType;
    const char*             pkeyName;        // canonical "System.Photo.CameraMake"
};

inline constexpr ShellPropStoreSchemaRow kShellPropStoreSchema[] = {
    { ShellPropStoreKeyId::IMAGE_WIDTH,          ShellPropStoreValueType::UINT32,      "System.Image.HorizontalSize" },
    { ShellPropStoreKeyId::IMAGE_HEIGHT,         ShellPropStoreValueType::UINT32,      "System.Image.VerticalSize"   },
    { ShellPropStoreKeyId::IMAGE_BIT_DEPTH,      ShellPropStoreValueType::UINT32,      "System.Image.BitDepth"       },
    { ShellPropStoreKeyId::IMAGE_HORIZONTAL_DPI, ShellPropStoreValueType::DOUBLE,      "System.Image.HorizontalResolution" },
    { ShellPropStoreKeyId::IMAGE_VERTICAL_DPI,   ShellPropStoreValueType::DOUBLE,      "System.Image.VerticalResolution"   },
    { ShellPropStoreKeyId::PHOTO_CAMERA_MAKE,    ShellPropStoreValueType::STRING_UTF8, "System.Photo.CameraManufacturer" },
    { ShellPropStoreKeyId::PHOTO_CAMERA_MODEL,   ShellPropStoreValueType::STRING_UTF8, "System.Photo.CameraModel"        },
    { ShellPropStoreKeyId::PHOTO_DATE_TAKEN,     ShellPropStoreValueType::FILETIME,    "System.Photo.DateTaken"          },
    { ShellPropStoreKeyId::PHOTO_EXPOSURE_TIME,  ShellPropStoreValueType::RATIONAL,    "System.Photo.ExposureTime"       },
    { ShellPropStoreKeyId::PHOTO_F_NUMBER,       ShellPropStoreValueType::DOUBLE,      "System.Photo.FNumber"            },
    { ShellPropStoreKeyId::PHOTO_ISO_SPEED,      ShellPropStoreValueType::UINT32,      "System.Photo.ISOSpeed"           },
    { ShellPropStoreKeyId::PHOTO_FOCAL_LENGTH,   ShellPropStoreValueType::DOUBLE,      "System.Photo.FocalLength"        },
    { ShellPropStoreKeyId::PHOTO_ORIENTATION,    ShellPropStoreValueType::UINT32,      "System.Photo.Orientation"        },
    { ShellPropStoreKeyId::DECODER_ID,           ShellPropStoreValueType::STRING_UTF8, "ExplorerLens.Decoder.Id"         },
    { ShellPropStoreKeyId::DECODER_VERSION,      ShellPropStoreValueType::UINT32,      "ExplorerLens.Decoder.Version"    },
    { ShellPropStoreKeyId::DECODER_LATENCY_MS,   ShellPropStoreValueType::UINT32,      "ExplorerLens.Decoder.LatencyMs"  },
};

inline constexpr size_t kShellPropStoreSchemaCount =
    sizeof(kShellPropStoreSchema) / sizeof(kShellPropStoreSchema[0]);

static_assert(kShellPropStoreSchemaCount == 16,
              "ShellPropStoreSchema must declare 16 rows");
static_assert(std::is_trivially_copyable_v<ShellPropStoreKey>,
              "ShellPropStoreKey must be trivially copyable");

} // namespace ExplorerLens::Engine
