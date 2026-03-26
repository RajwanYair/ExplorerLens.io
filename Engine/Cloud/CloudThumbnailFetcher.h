// CloudThumbnailFetcher.h — Cloud-Side Thumbnail Pre-Fetch
// Copyright (c) 2026 ExplorerLens Project
//
// Fetches server-generated thumbnails from cloud APIs (Graph API for OneDrive,
// SharePoint REST) to bypass local file download for large un-hydrated files.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Request / Response -----------------------------------------------------

struct CloudThumbRequest {
    std::string localPath;      // Local placeholder or UNC path
    std::string driveItemId;    // Graph API driveItem ID (if known)
    uint32_t    width    = 256; // Requested thumbnail width
    uint32_t    height   = 256;
    uint32_t    timeoutMs = 3000;
};

enum class CloudThumbStatus {
    Success          = 0,
    NotAuthenticated = 1,
    ItemNotFound     = 2,
    NoThumbnailAvail = 3,
    NetworkError     = 4,
    Timeout          = 5,
    RateLimited      = 6,
    InternalError    = 99,
};

struct CloudThumbResult {
    CloudThumbStatus     status  = CloudThumbStatus::InternalError;
    std::vector<uint8_t> pixels; // BGRA (decoded from JPEG/PNG cloud response)
    uint32_t             width   = 0;
    uint32_t             height  = 0;
    std::string          source; // e.g. "graph-api/1.0", "spo-rest"
};

using CloudThumbCallback = std::function<void(CloudThumbResult)>;

// ---- CloudThumbnailFetcher --------------------------------------------------

class CloudThumbnailFetcher {
public:
    CloudThumbnailFetcher();
    ~CloudThumbnailFetcher();

    // Synchronous fetch — blocks up to request.timeoutMs.
    CloudThumbResult Fetch(const CloudThumbRequest& req) const;

    // Async fetch — calls callback on a background thread.
    void FetchAsync(const CloudThumbRequest& req, CloudThumbCallback callback) const;

    // Resolve a local OneDrive placeholder path to a Graph API driveItem ID.
    bool ResolveItemId(const std::string& localPath, std::string& outItemId) const;

    // Check if the Graph API access token is valid (non-expired).
    bool IsAuthenticated() const;

    // Set auth token (obtained from WAM / MSAL). Cleared on token expiry.
    void SetAccessToken(const std::string& bearerToken);
    void ClearAccessToken();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
