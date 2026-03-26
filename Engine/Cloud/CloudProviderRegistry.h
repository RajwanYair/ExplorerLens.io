// CloudProviderRegistry.h — Cloud Storage Provider Registry
// Copyright (c) 2026 ExplorerLens Project
//
// Registry of cloud storage providers whose virtual files can be thumbnail-decoded
// by ExplorerLens.  Each provider registers a CloudProviderDescriptor describing
// file-system namespaces, stub-file hydration behavior, and rate limit profiles.
//
// Supported providers (v16.1.0+):
//   OneDrive (Personal + Business), SharePoint, Teams, Box.com,
//   Dropbox, Google Drive (FUSE), Amazon S3-compatible buckets.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

// Virtual-file hydration policy for a cloud provider.
enum class HydrationPolicy : uint8_t {
    AlwaysOnline    = 0,  // File must be downloaded before decode; no local copy left
    OnDemand        = 1,  // Download to temp, decode, then clean up (default)
    PinIfDecoded    = 2,  // Leave file local after first decode (user can unpin)
    ThumbnailOnly   = 3,  // Use provider's own thumbnail API; never hydrate full file
};

// Rate limiting profile for provider API calls.
struct ProviderRateLimit {
    uint32_t requestsPerMinute  { 60 };
    uint32_t maxConcurrent      { 4 };
    uint32_t retryAfterMs       { 1000 };
    uint32_t maxRetries         { 3 };
};

// Descriptor for a registered cloud provider.
struct CloudProviderDescriptor {
    std::string       providerId;    // Unique ID e.g. "onedrive-personal"
    std::string       displayName;   // Human-readable e.g. "OneDrive (Personal)"
    std::string       fsRootPrefix;  // Local CF_MountRoot prefix for matching virtual files
    std::string       storageServerId; // CfApi StorageProviderId e.g. "OneDriveConsumer"
    HydrationPolicy   policy;
    ProviderRateLimit rateLimit;
    bool              hasThumbnailApi { false };  // Provider exposes native thumbnail endpoint
    std::string       thumbnailApiBase;            // e.g. "https://graph.microsoft.com/v1.0"
};

// Callback fired when a provider is registered or deregistered.
using ProviderChangeCallback = std::function<void(const CloudProviderDescriptor&, bool registered)>;

// CloudProviderRegistry — Central registry of cloud storage providers.
//
// Populated at startup from HKCU\Software\Microsoft\Windows\CurrentVersion\CloudStore
// and augmented with ExplorerLens-specific descriptors.
// Thread-safe for concurrent reads; write-serialised internally.
class CloudProviderRegistry {
public:
    CloudProviderRegistry() noexcept;
    ~CloudProviderRegistry() noexcept;

    CloudProviderRegistry(const CloudProviderRegistry&)            = delete;
    CloudProviderRegistry& operator=(const CloudProviderRegistry&) = delete;

    // Discover all installed cloud providers on the current machine.
    void Discover() noexcept;

    // Register a custom / enterprise provider descriptor.
    void Register(CloudProviderDescriptor desc) noexcept;

    // Deregister by ID.
    void Unregister(const std::string& providerId) noexcept;

    // Find which provider (if any) owns the given local file path.
    const CloudProviderDescriptor* FindForPath(const std::string& localPath) const noexcept;

    // Get all registered providers.
    std::vector<CloudProviderDescriptor> All() const noexcept;

    // Subscribe to provider registration changes.
    void OnChange(ProviderChangeCallback cb) noexcept;

    uint32_t Count() const noexcept;

    // Singleton accessor.
    static CloudProviderRegistry& Instance() noexcept;

private:
    struct Impl;
    Impl* m_impl { nullptr };
};

}} // namespace ExplorerLens::Engine
