// ThumbnailHashRegistry.h — Expected perceptual hash registry for corpus validation
// Copyright (c) 2026 ExplorerLens Project
//
// Loads and manages expected thumbnail hashes from the corpus MANIFEST.json.
// Used by FormatValidationRunner to detect regressions: if the perceptual hash
// of a newly decoded thumbnail differs from the stored expected hash, the
// decoder output has changed and requires investigation.
//
#pragma once
#include "PerceptualHashUtility.h"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ExpectedHash {
    std::string     filePath;   // corpus-relative path
    std::string     formatId;   // e.g. "JPEG"
    PerceptualHash  dhash;      // expected dHash
    PerceptualHash  phash;      // expected pHash
    int             targetSize; // thumbnail size this hash was computed at
};

struct HashCheckResult {
    std::string filePath;
    bool        passed;
    int         dhammingDist;   // dHash Hamming distance from expected
    int         phammingDist;   // pHash Hamming distance from expected
    std::string failReason;     // empty on pass
};

class ThumbnailHashRegistry {
public:
    ThumbnailHashRegistry() = default;

    // Load expected hashes from a MANIFEST.json file.
    // The manifest may or may not have an "expected_hashes" section; if absent,
    // the registry starts empty and AcceptAllNew() can register new baselines.
    // Throws std::runtime_error if the file cannot be read or is malformed JSON.
    void LoadFromManifest(const std::string& manifestPath);

    // Save current expected hashes back into the manifest (under "expected_hashes").
    void SaveToManifest(const std::string& manifestPath) const;

    // Register or update the expected hash for a corpus file.
    // Used when setting a new baseline (e.g. after a deliberate decoder change).
    void Register(const ExpectedHash& entry);

    // Look up the expected hash for a corpus file.
    // Returns nullopt if the file has no baseline yet.
    std::optional<ExpectedHash> Lookup(const std::string& filePath) const;

    // Check a decoded thumbnail against the stored expected hash.
    // pixels: BGRA32, width/height: thumbnail dimensions
    HashCheckResult Check(const std::string&         filePath,
                          std::span<const uint8_t>   pixels,
                          uint32_t                   width,
                          uint32_t                   height,
                          int                        dhashThreshold = 8,
                          int                        phashThreshold = 10) const;

    // Returns all entries currently loaded
    const std::vector<ExpectedHash>& Entries() const noexcept { return m_entries; }

    // Number of registered expected hashes
    size_t Count() const noexcept { return m_entries.size(); }

    // Mark all entries as accepted (used for initial baseline generation)
    void Clear() noexcept { m_entries.clear(); m_index.clear(); }

private:
    std::vector<ExpectedHash>               m_entries;
    std::unordered_map<std::string, size_t> m_index;    // filePath → index in m_entries
};

} // namespace Engine
} // namespace ExplorerLens
