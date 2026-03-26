// CloudFileStreamer.h — Range-Request HTTP Streamer for Partial Cloud Downloads
// Copyright (c) 2026 ExplorerLens Project
//
// Performs HTTP/HTTPS range requests against cloud storage URLs (pre-signed
// Azure SAS, S3 pre-signed, OneDrive download URL) to fetch only file header
// bytes needed for thumbnail decode without pulling the full file.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Stream Request ---------------------------------------------------------

struct RangeStreamRequest {
    std::string  url;              // Direct download URL (SAS, pre-signed, etc.)
    uint64_t     offset     = 0;
    uint64_t     length     = 0;   // 0 = fetch entire resource
    uint32_t     timeoutMs  = 8000;
    std::string  userAgent;        // defaults to "ExplorerLens/18.1"
    std::string  bearerToken;      // OAuth token if required (non-SAS URLs)
};

enum class StreamStatus {
    Success          = 0,
    InvalidUrl       = 1,
    NetworkError     = 2,
    Timeout          = 3,
    RangeNotSatisfied = 4,  // HTTP 416
    Unauthorized     = 5,   // HTTP 401
    Forbidden        = 6,   // HTTP 403
    NotFound         = 7,   // HTTP 404
    RateLimited      = 8,   // HTTP 429
    InternalError    = 99,
};

struct RangeStreamResult {
    StreamStatus         status   = StreamStatus::InternalError;
    std::vector<uint8_t> data;
    uint64_t             totalFileSize = 0;  // From Content-Range header
    std::string          contentType;
    std::string          etag;
};

using StreamProgressCallback = std::function<void(uint64_t bytesReceived)>;

// ---- CloudFileStreamer -------------------------------------------------------

class CloudFileStreamer {
public:
    CloudFileStreamer();
    ~CloudFileStreamer();

    // Blocking range fetch — returns when bytes are available or error occurs.
    RangeStreamResult FetchRange(
        const RangeStreamRequest&  req,
        StreamProgressCallback     progress = nullptr) const;

    // Non-blocking: queues the request, calls callback when done.
    void FetchRangeAsync(
        const RangeStreamRequest&  req,
        std::function<void(RangeStreamResult)> callback) const;

    // Download only the first N bytes (header sniff for magic/format probe).
    RangeStreamResult FetchHeader(
        const std::string& url,
        uint64_t           headerBytes = 512,
        uint32_t           timeoutMs   = 4000) const;

    // Number of concurrent connections allowed.
    void SetConcurrency(uint32_t maxConcurrent);
    uint32_t Concurrency() const;

    static CloudFileStreamer& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
