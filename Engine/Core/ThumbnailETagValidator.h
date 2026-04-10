// ThumbnailETagValidator.h — Cloud File ETag + Mtime Cache Invalidation Validator
// Copyright (c) 2026 ExplorerLens Project
//
// For cloud-hosted files (OneDrive, SharePoint, S3, Azure Blob) the OS mtime may
// not change when the cloud provider modifies the remote file.  This validator
// maintains a local ETag → decoded-thumbnail-key mapping so the cache is
// invalidated whenever the cloud ETag changes, even if mtime is identical.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>

namespace ExplorerLens { namespace Engine {

/// Validated state for a cached thumbnail.
enum class ETagValidationResult : uint8_t {
    VALID         = 0,  ///< ETag + mtime match; cache hit is safe
    ETAG_CHANGED  = 1,  ///< ETag differs from stored; must re-decode
    MTIME_CHANGED = 2,  ///< Mtime differs (ETag absent); must re-decode
    NOT_PRESENT   = 3,  ///< No record; first-time validation
};

/// Stored validation record for one file.
struct ETagRecord {
    std::string etag;           ///< Last known ETag (may be empty for local files)
    uint64_t    mtimeMs  = 0;   ///< Last known modification time (ms since epoch)
    uint64_t    fileSize = 0;   ///< Size cross-check
    uint64_t    recordedAt = 0; ///< Wall clock ms when record was stored
};

/// ETag-based thumbnail cache validator.
class ThumbnailETagValidator {
public:
    /// Validate a file against the stored record.
    ETagValidationResult Validate(
        const std::wstring& filePath,
        const std::string&  currentETag,
        uint64_t            currentMtimeMs,
        uint64_t            currentFileSize) const;

    /// Update / insert the record after a successful decode.
    void Record(
        const std::wstring& filePath,
        const std::string&  etag,
        uint64_t            mtimeMs,
        uint64_t            fileSize);

    /// Remove a record (e.g. on file deletion).
    void Remove(const std::wstring& filePath);

    /// Number of records stored.
    uint32_t RecordCount() const;

    /// Purge records older than ageSeconds.
    uint32_t PurgeOld(uint64_t ageSeconds);

    void Clear();

    static const char* ValidationLabel(ETagValidationResult r);

private:
    mutable std::unordered_map<std::wstring, ETagRecord> m_records;
};

}} // namespace ExplorerLens::Engine
