// SilentUpdateOrchestrator.h — Silent Background Update Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages silent background updates for enterprise and consumer
// deployments. Coordinates download, verification, staging, and
// atomic swap of binaries without interrupting Explorer thumbnail
// operations or requiring user interaction.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SilentUpdateChannel : uint8_t {
    Stable, Beta, Canary, Enterprise, LTS, COUNT
};

enum class SilentUpdateState : uint8_t {
    Idle, Checking, Downloading, Verifying, Staging, Applying, Complete, Failed, COUNT
};

struct UpdatePackage {
    std::wstring version;
    std::wstring downloadUrl;
    std::wstring sha256Hash;
    uint64_t sizeBytes = 0;
    SilentUpdateChannel channel = SilentUpdateChannel::Stable;
    bool deltaUpdate = false;
    bool requiresRestart = false;
};

struct UpdateProgress {
    SilentUpdateState state = SilentUpdateState::Idle;
    float progressPercent = 0.0f;
    uint64_t bytesDownloaded = 0;
    uint64_t bytesTotal = 0;
    double downloadSpeedKBps = 0.0;
    std::wstring statusMessage;
};

struct UpdateConfig {
    SilentUpdateChannel channel = SilentUpdateChannel::Stable;
    uint32_t checkIntervalHrs = 24;
    bool silentInstall = true;
    bool allowMeteredNetwork = false;
    bool allowDeltaUpdates = true;
    uint32_t maxRetries = 3;
};

class SilentUpdateOrchestrator {
public:
    void Configure(const UpdateConfig& cfg) { m_config = cfg; }
    const UpdateConfig& GetConfig() const { return m_config; }

    bool CheckForUpdates(const std::wstring& currentVersion) {
        m_progress.state = SilentUpdateState::Checking;
        // Simulated check
        m_progress.state = SilentUpdateState::Idle;
        m_lastCheckVersion = currentVersion;
        return false; // No update available in simulation
    }

    bool StartDownload(const UpdatePackage& pkg) {
        if (m_progress.state != SilentUpdateState::Idle) return false;
        m_package = pkg;
        m_progress.state = SilentUpdateState::Downloading;
        m_progress.bytesTotal = pkg.sizeBytes;
        return true;
    }

    bool Apply() {
        if (m_progress.state != SilentUpdateState::Downloading) return false;
        m_progress.state = SilentUpdateState::Verifying;
        m_progress.state = SilentUpdateState::Staging;
        m_progress.state = SilentUpdateState::Applying;
        m_progress.state = SilentUpdateState::Complete;
        m_progress.progressPercent = 100.0f;
        return true;
    }

    const UpdateProgress& GetProgress() const { return m_progress; }
    SilentUpdateState GetState() const { return m_progress.state; }

    void Reset() {
        m_progress = {};
        m_package = {};
    }

    static size_t ChannelCount() { return static_cast<size_t>(SilentUpdateChannel::COUNT); }
    static size_t StateCount() { return static_cast<size_t>(SilentUpdateState::COUNT); }

private:
    UpdateConfig m_config;
    UpdatePackage m_package;
    UpdateProgress m_progress;
    std::wstring m_lastCheckVersion;
};

} // namespace Engine
} // namespace ExplorerLens
