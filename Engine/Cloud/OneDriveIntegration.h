// OneDriveIntegration.h — OneDrive Cloud File Thumbnail Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides thumbnail generation for OneDrive virtual/stub files (CF_API
// placeholder files) without forcing full file hydration.  Uses the
// Microsoft Graph API thumbnail endpoint when available, falling back
// to local hydration via CfHydratePlaceholder for offline/no-API scenarios.
//
// Coverage: OneDrive Personal, OneDrive for Business (SharePoint), Teams files.
//
#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine {

// OneDrive tunnel result — how the thumbnail was obtained.
enum class OneDriveThumbnailSource : uint8_t {
    GraphApi    = 0,  // Retrieved from Microsoft Graph /thumbnails endpoint
    LocalFile   = 1,  // File was already local (fully hydrated)
    Hydrated    = 2,  // Hydrated via CF_API, decoded locally, then dehydrated
    Unavailable = 3,  // Not possible to get thumbnail (no auth, no file)
};

// Authentication token provider callback.
using TokenProviderFn = std::function<std::string()>;  // Returns Bearer token or ""

// OneDriveIntegration — Thumbnail resolver for CF_API virtual files.
//
// Integrates with the Windows Cloud Files API (CfApi) to:
//   - Detect whether a file is a OneDrive placeholder
//   - Query Graph API for server-side thumbnails (no local decode needed)
//   - Fall back to CfHydratePlaceholder + local decode for formats Graph doesn't support
//   - Optionally dehydrate the file again after thumbnail extraction
class OneDriveIntegration {
public:
    OneDriveIntegration() noexcept;
    ~OneDriveIntegration() noexcept;

    OneDriveIntegration(const OneDriveIntegration&)            = delete;
    OneDriveIntegration& operator=(const OneDriveIntegration&) = delete;

    // Initialise with a token provider for Graph API calls.
    void Initialize(TokenProviderFn tokenProvider) noexcept;

    // Returns true if the given path is a OneDrive placeholder file.
    bool IsPlaceholder(const std::string& localPath) const noexcept;

    // Attempt to get thumbnail bytes (BGRA raw or JPEG) for a virtual file.
    // thumbSize: desired thumbnail edge size in pixels (e.g. 256, 512, 1024).
    // Returns empty vector on failure.
    std::vector<uint8_t> GetThumbnail(const std::string& localPath,
                                       uint32_t           thumbSize,
                                       OneDriveThumbnailSource& outSource) noexcept;

    // Hydrate a placeholder to allow local decode, then optionally dehydrate.
    bool HydratePlaceholder(const std::string& localPath,
                             bool               dehydrateAfter = true) noexcept;

    // Get the Graph item path for a local OneDrive file.
    std::string ResolveGraphPath(const std::string& localPath) const noexcept;

    bool IsAvailable() const noexcept { return m_available; }

private:
    bool           m_available { false };
    TokenProviderFn m_tokenProvider;
};

}} // namespace ExplorerLens::Engine
