// ThumbnailPackFile.h — Compact Multi-Thumbnail Bundle Format (.tlpk)
// Copyright (c) 2026 ExplorerLens Project
//
// Packs multiple thumbnail blobs into a single zstd-compressed archive
// for efficient cloud upload/download of a device sync manifest payload.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Magic bytes at the start of every .tlpk file.
static constexpr uint32_t TLPK_MAGIC = 0x544C504Bu; // "TLPK"

// Per-entry header inside a pack file.
struct PackEntry {
    uint64_t pathHash    = 0;   // Matches SyncManifestEntry::pathHash
    uint32_t offset      = 0;   // Byte offset of blob within the packed payload
    uint32_t compSize    = 0;   // Compressed size of this thumbnail
    uint32_t origSize    = 0;   // Original (uncompressed) size
};

// Error codes for pack/unpack operations.
enum class PackFileError : uint8_t {
    OK,
    INVALID_MAGIC,
    CORRUPT_HEADER,
    DECOMPRESS_FAILED,
    ENTRY_NOT_FOUND
};

// Builds and reads .tlpk files — multi-thumbnail bundles with zstd compression.
class ThumbnailPackFile {
public:
    ThumbnailPackFile() = default;

    // Add a raw thumbnail blob keyed by path hash.
    void AddEntry(uint64_t pathHash, const std::vector<uint8_t>& blob);

    // Finalise and serialise the entire pack to a binary blob.
    // Compression is performed here.
    std::vector<uint8_t> Pack() const;

    // Load a pack blob and index its entries without decompressing payloads.
    PackFileError Load(const std::vector<uint8_t>& data);

    // Extract one thumbnail by hash. Returns empty vector on miss.
    std::vector<uint8_t> Extract(uint64_t pathHash) const;

    // Accessors.
    size_t EntryCount() const { return m_entries.size(); }
    PackFileError LastError() const { return m_lastError; }

private:
    std::vector<PackEntry>             m_entries;
    std::vector<std::vector<uint8_t>>  m_blobs;      // parallel to m_entries
    std::vector<uint8_t>               m_packedData;  // set after Load()
    PackFileError                      m_lastError = PackFileError::OK;
};

} // namespace Engine
} // namespace ExplorerLens
