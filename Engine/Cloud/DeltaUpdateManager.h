// DeltaUpdateManager.h — Incremental / Delta Update Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages delta (incremental) updates for ExplorerLens binaries and external
// library packages.  Downloads binary patches (bsdiff-format), verifies SHA-256
// checksums, applies patches atomically, and triggers installer-less in-place
// updates for enterprise deployments.
//
// Update channels: Stable, Preview (RC), Enterprise (quarterly)
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <chrono>

namespace ExplorerLens { namespace Engine {

// Update release channel.
enum class UpdateChannel : uint8_t {
    Stable     = 0,   // Monthly stable releases
    Preview    = 1,   // RC/pre-releases — opt-in
    Enterprise = 2,   // Quarterly batched updates — WSUS-friendly
    None       = 0xFF, // Updates disabled
};

// Per-component update descriptor.
struct UpdateDescriptor {
    std::string  componentId;       // e.g. "LENSShell.dll", "ExplorerLensEngine.lib"
    std::string  currentVersion;
    std::string  targetVersion;
    uint64_t     patchSizeBytes;
    std::string  patchUrl;          // HTTPS URL for bsdiff patch
    std::string  sha256Checksum;    // Expected SHA-256 of the patch file
    bool         requiresElevation; // True if component is in HKLM or Program Files
    bool         isDeltaPatch;      // False = full replacement
};

// Update check result.
struct UpdateCheckResult {
    bool                          updatesAvailable { false };
    std::vector<UpdateDescriptor> updates;
    std::string                   releaseNotesUrl;
    std::chrono::system_clock::time_point checkedAt;
};

// Progress callback for downloads and apply operations.
using UpdateProgressFn =
    std::function<void(const std::string& componentId,
                       uint64_t           bytesReceived,
                       uint64_t           totalBytes)>;

// DeltaUpdateManager — Orchestrates incremental update check, download, verify, apply.
//
// Update flow:
//   1. CheckForUpdates() — GET /releases/latest JSON from manifest URL
//   2. Download() — Download only components that changed (bsdiff patches preferred)
//   3. Verify() — SHA-256 checksum of each patch
//   4. Apply() — bsdiff apply, atomic file replace, session-restart notification
class DeltaUpdateManager {
public:
    DeltaUpdateManager() noexcept;
    ~DeltaUpdateManager() noexcept;

    DeltaUpdateManager(const DeltaUpdateManager&)            = delete;
    DeltaUpdateManager& operator=(const DeltaUpdateManager&) = delete;

    // Configure update channel and manifest URL.
    void Configure(UpdateChannel channel,
                   const std::string& manifestUrl = {}) noexcept;

    // Check for available updates.  Non-blocking; calls onComplete when done.
    void CheckForUpdates(std::function<void(UpdateCheckResult)> onComplete) noexcept;

    // Download all pending updates.  Returns false if any download fails.
    bool Download(const std::vector<UpdateDescriptor>& updates,
                  UpdateProgressFn                      onProgress = nullptr) noexcept;

    // Verify SHA-256 checksums of downloaded patches.
    bool Verify(const std::vector<UpdateDescriptor>& updates) noexcept;

    // Apply all verified downloads.  Returns false if any component fails to apply.
    bool Apply(const std::vector<UpdateDescriptor>& updates) noexcept;

    // Convenience: check + download + verify + apply in sequence.
    bool RunFullUpdate(UpdateProgressFn onProgress = nullptr) noexcept;

    // Set working directory for patch downloads (default: %TEMP%\ExplorerLens\updates).
    void SetWorkDirectory(const std::string& dir) noexcept;

    UpdateChannel GetChannel() const noexcept { return m_channel; }
    bool IsUpdateAvailable() const noexcept;

private:
    UpdateChannel m_channel { UpdateChannel::Stable };
    std::string   m_manifestUrl;
    std::string   m_workDir;
    UpdateCheckResult m_lastCheck;
};

}} // namespace ExplorerLens::Engine
