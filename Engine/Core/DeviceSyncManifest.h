// DeviceSyncManifest.h — Cross-Device Thumbnail Cache Manifest
// Copyright (c) 2026 ExplorerLens Project
//
// Portable manifest describing which thumbnails are cached on a device,
// keyed by path hash + ETag so remote devices can decide what to download.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace ExplorerLens {
namespace Engine {

// Uniquely identifies one cached thumbnail entry in the manifest.
struct SyncManifestEntry {
    uint64_t    pathHash    = 0;        // FNV-1a hash of the canonical file path
    std::string etag;                   // Cloud ETag or mtime-based token
    uint32_t    width       = 0;
    uint32_t    height      = 0;
    uint32_t    sizeBytes   = 0;        // Compressed thumbnail blob size
    uint64_t    generatedAt = 0;        // Unix epoch seconds
};

// Serialisation format version tag embedded in each manifest file.
enum class ManifestVersion : uint8_t {
    V1 = 1
};

// Portable manifest of all cached thumbnails on a device.
// Serialised to a compact binary format for cloud upload.
class DeviceSyncManifest {
public:
    DeviceSyncManifest() = default;
    explicit DeviceSyncManifest(std::string deviceId);

    // Append or update one entry. Returns true if the entry was new.
    bool Upsert(const SyncManifestEntry& entry);

    // Remove an entry by path hash. Returns true if found.
    bool Remove(uint64_t pathHash);

    // Look up an entry. Returns empty optional if not found.
    std::optional<SyncManifestEntry> Find(uint64_t pathHash) const;

    // Serialise to a compact binary blob (little-endian, no external deps).
    std::vector<uint8_t> Serialize() const;

    // Deserialise from a binary blob. Returns false on format error.
    bool Deserialize(const std::vector<uint8_t>& data);

    // Range accessors.
    const std::vector<SyncManifestEntry>& Entries() const { return m_entries; }
    size_t EntryCount() const { return m_entries.size(); }
    const std::string& DeviceId() const { return m_deviceId; }
    ManifestVersion Version() const { return ManifestVersion::V1; }

private:
    std::string                    m_deviceId;
    std::vector<SyncManifestEntry> m_entries;
};

} // namespace Engine
} // namespace ExplorerLens
