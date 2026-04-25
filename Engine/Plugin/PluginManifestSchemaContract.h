// ============================================================================
// PluginManifestSchemaContract.h -- S282 / ROADMAP v6.0 H4 + H23 plugin schema
//
// Phase 3 contract: every `.lens` plugin bundle carries a typed JSON manifest.
// This header declares the canonical key list (14 keys), value-type tags, and
// required/optional flags for the schema validator.  Header-only.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class PluginManifestValueKind : uint8_t
{
    STRING      = 0,
    SEMVER      = 1,
    INTEGER     = 2,
    BOOLEAN     = 3,
    SHA256_HEX  = 4,
    UUID        = 5,
    STRING_ARRAY = 6,
    OBJECT       = 7,
    ENUM         = 8,
};

struct PluginManifestKey
{
    const char*             jsonPath;       // "decoder.name", "trust.signer"
    PluginManifestValueKind kind;
    bool                    required;
    uint16_t                maxLength;      // 0 = no cap
};

inline constexpr PluginManifestKey kPluginManifestSchema[] = {
    { "schemaVersion",      PluginManifestValueKind::INTEGER,     true,  0 },
    { "id",                 PluginManifestValueKind::UUID,        true, 36 },
    { "name",               PluginManifestValueKind::STRING,      true, 64 },
    { "version",            PluginManifestValueKind::SEMVER,      true, 32 },
    { "abiVersion",         PluginManifestValueKind::INTEGER,     true,  0 },
    { "decoder.formats",    PluginManifestValueKind::STRING_ARRAY, true, 0 },
    { "decoder.mimeTypes",  PluginManifestValueKind::STRING_ARRAY, false, 0 },
    { "trust.signer",       PluginManifestValueKind::STRING,      true, 128 },
    { "trust.sha256",       PluginManifestValueKind::SHA256_HEX,  true, 64 },
    { "capabilities",       PluginManifestValueKind::STRING_ARRAY, true, 0 },
    { "sandbox.level",      PluginManifestValueKind::ENUM,        true, 16 },
    { "budgets.decodeMs",   PluginManifestValueKind::INTEGER,     false, 0 },
    { "budgets.memoryMb",   PluginManifestValueKind::INTEGER,     false, 0 },
    { "homepageUrl",        PluginManifestValueKind::STRING,      false, 256 },
};
inline constexpr size_t kPluginManifestSchemaKeyCount =
    sizeof(kPluginManifestSchema) / sizeof(kPluginManifestSchema[0]);

enum class PluginManifestValidationStatus : uint8_t
{
    OK                    = 0,
    SCHEMA_VERSION_MISMATCH = 1,
    REQUIRED_KEY_MISSING  = 2,
    TYPE_MISMATCH         = 3,
    VALUE_TOO_LONG        = 4,
    UNKNOWN_CAPABILITY    = 5,
    UNKNOWN_SANDBOX_LEVEL = 6,
    MALFORMED_JSON        = 7,
};

inline constexpr uint32_t kPluginManifestMaxBytes     = 64u * 1024u;   // 64 KiB JSON cap
inline constexpr uint32_t kPluginManifestSchemaVersion = 1;

static_assert(kPluginManifestSchemaKeyCount == 14,
              "Plugin manifest schema must have 14 keys");
static_assert(std::is_trivially_copyable_v<PluginManifestKey>,
              "PluginManifestKey must be trivially copyable");

} // namespace ExplorerLens::Engine
