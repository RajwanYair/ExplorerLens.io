// ThumbnailETagValidator.cpp — Cloud File ETag + Mtime Cache Invalidation Validator
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailETagValidator.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

static uint64_t NowMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

ETagValidationResult ThumbnailETagValidator::Validate(
    const std::wstring& filePath,
    const std::string&  currentETag,
    uint64_t            currentMtimeMs,
    uint64_t            /*currentFileSize*/) const
{
    auto it = m_records.find(filePath);
    if (it == m_records.end()) return ETagValidationResult::NOT_PRESENT;

    const auto& rec = it->second;

    // ETag check takes priority when both are present
    if (!rec.etag.empty() && !currentETag.empty()) {
        return (rec.etag == currentETag)
            ? ETagValidationResult::VALID
            : ETagValidationResult::ETAG_CHANGED;
    }

    // Fall back to mtime comparison
    return (rec.mtimeMs == currentMtimeMs)
        ? ETagValidationResult::VALID
        : ETagValidationResult::MTIME_CHANGED;
}

void ThumbnailETagValidator::Record(
    const std::wstring& filePath,
    const std::string&  etag,
    uint64_t            mtimeMs,
    uint64_t            fileSize)
{
    ETagRecord& rec  = m_records[filePath];
    rec.etag         = etag;
    rec.mtimeMs      = mtimeMs;
    rec.fileSize     = fileSize;
    rec.recordedAt   = NowMs();
}

void ThumbnailETagValidator::Remove(const std::wstring& filePath)
{
    m_records.erase(filePath);
}

uint32_t ThumbnailETagValidator::RecordCount() const
{
    return static_cast<uint32_t>(m_records.size());
}

uint32_t ThumbnailETagValidator::PurgeOld(uint64_t ageSeconds)
{
    const uint64_t cutoffMs = NowMs() - ageSeconds * 1000ULL;
    uint32_t purged = 0;
    for (auto it = m_records.begin(); it != m_records.end(); )
    {
        if (it->second.recordedAt < cutoffMs) {
            it = m_records.erase(it);
            ++purged;
        } else {
            ++it;
        }
    }
    return purged;
}

void ThumbnailETagValidator::Clear()
{
    m_records.clear();
}

const char* ThumbnailETagValidator::ValidationLabel(ETagValidationResult r)
{
    switch (r) {
        case ETagValidationResult::VALID:          return "Valid";
        case ETagValidationResult::ETAG_CHANGED:   return "ETagChanged";
        case ETagValidationResult::MTIME_CHANGED:  return "MtimeChanged";
        default:                                   return "NotPresent";
    }
}

}} // namespace ExplorerLens::Engine
