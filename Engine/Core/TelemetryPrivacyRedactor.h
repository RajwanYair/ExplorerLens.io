// ============================================================================
// TelemetryPrivacyRedactor.h -- S276 / ROADMAP v6.0 S8 telemetry privacy
//
// Phase 3 redactor contract.  Before any ETW / OpenTelemetry payload leaves
// the Engine, user-identifying fields (paths, filenames, GPS, camera serial
// numbers) must be hashed or dropped according to this policy.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class TelemetryPrivacyField : uint16_t
{
    NONE                = 0,
    FILE_PATH           = 1,   // "C:\Users\alice\..." -> SHA256 prefix
    FILE_NAME           = 2,
    CONTAINING_FOLDER   = 3,
    USER_NAME           = 4,
    HOST_NAME           = 5,
    CAMERA_SERIAL       = 6,
    GPS_COORDS          = 7,
    EMBEDDED_URL        = 8,
    WINDOWS_SID         = 9,
    EMAIL_ADDRESS       = 10,
};

enum class TelemetryPrivacyRedactMode : uint8_t
{
    PASS_THROUGH        = 0,   // dev builds only; forbidden in Release
    HASH_SHA256_PREFIX  = 1,   // first 16 hex chars of SHA-256
    FIXED_TOKEN         = 2,   // "<redacted:FILE_PATH>"
    EMPTY_STRING        = 3,
    FULL_DROP           = 4,   // omit property entirely
};

struct TelemetryPrivacyRule
{
    TelemetryPrivacyField      field;
    TelemetryPrivacyRedactMode mode;
    const char*                etwPropertyName;  // "file.path", "gps.lat"
};

inline constexpr TelemetryPrivacyRule kTelemetryPrivacyRules[] = {
    { TelemetryPrivacyField::FILE_PATH,         TelemetryPrivacyRedactMode::HASH_SHA256_PREFIX, "file.path"        },
    { TelemetryPrivacyField::FILE_NAME,         TelemetryPrivacyRedactMode::HASH_SHA256_PREFIX, "file.name"        },
    { TelemetryPrivacyField::CONTAINING_FOLDER, TelemetryPrivacyRedactMode::FIXED_TOKEN,        "file.folder"      },
    { TelemetryPrivacyField::USER_NAME,         TelemetryPrivacyRedactMode::FULL_DROP,          "user.name"        },
    { TelemetryPrivacyField::HOST_NAME,         TelemetryPrivacyRedactMode::HASH_SHA256_PREFIX, "host.name"        },
    { TelemetryPrivacyField::CAMERA_SERIAL,     TelemetryPrivacyRedactMode::HASH_SHA256_PREFIX, "photo.serial"     },
    { TelemetryPrivacyField::GPS_COORDS,        TelemetryPrivacyRedactMode::FULL_DROP,          "photo.gps"        },
    { TelemetryPrivacyField::EMBEDDED_URL,      TelemetryPrivacyRedactMode::FIXED_TOKEN,        "resource.url"     },
    { TelemetryPrivacyField::WINDOWS_SID,       TelemetryPrivacyRedactMode::FULL_DROP,          "user.sid"         },
    { TelemetryPrivacyField::EMAIL_ADDRESS,     TelemetryPrivacyRedactMode::FULL_DROP,          "contact.email"    },
};
inline constexpr size_t kTelemetryPrivacyRulesCount =
    sizeof(kTelemetryPrivacyRules) / sizeof(kTelemetryPrivacyRules[0]);

struct TelemetryPrivacyPolicy
{
    bool   redactFilePaths         = true;
    bool   redactHostIdentifiers   = true;
    bool   redactPhotoExifPii      = true;
    bool   allowPassThroughInDev   = false;   // never flip in shipping builds
    uint32_t maxPayloadBytes       = 32 * 1024;
};

static_assert(kTelemetryPrivacyRulesCount == 10,
              "TelemetryPrivacy rule table must list 10 fields");
static_assert(std::is_trivially_copyable_v<TelemetryPrivacyPolicy>,
              "TelemetryPrivacyPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TelemetryPrivacyRule>,
              "TelemetryPrivacyRule must be trivially copyable");

} // namespace ExplorerLens::Engine
