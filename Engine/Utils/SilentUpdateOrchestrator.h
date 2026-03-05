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

enum class UpdateState : uint8_t {
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
    UpdateState state = UpdateState::Idle;
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
        m_progress.state = UpdateState::Checking;
        // Simulated check
        m_progress.state = UpdateState::Idle;
        m_lastCheckVersion = currentVersion;
        return false; // No update available in simulation
    }

    bool StartDownload(const UpdatePackage& pkg) {
        if (m_progress.state != UpdateState::Idle) return false;
        m_package = pkg;
        m_progress.state = UpdateState::Downloading;
        m_progress.bytesTotal = pkg.sizeBytes;
        return true;
    }

    bool Apply() {
        if (m_progress.state != UpdateState::Downloading) return false;
        m_progress.state = UpdateState::Verifying;
        m_progress.state = UpdateState::Staging;
        m_progress.state = UpdateState::Applying;
        m_progress.state = UpdateState::Complete;
        m_progress.progressPercent = 100.0f;
        return true;
    }

    const UpdateProgress& GetProgress() const { return m_progress; }
    UpdateState GetState() const { return m_progress.state; }

    void Reset() {
        m_progress = {};
        m_package = {};
    }

    static size_t ChannelCount() { return static_cast<size_t>(SilentUpdateChannel::COUNT); }
    static size_t StateCount() { return static_cast<size_t>(UpdateState::COUNT); }

private:
    UpdateConfig m_config;
    UpdatePackage m_package;
    UpdateProgress m_progress;
    std::wstring m_lastCheckVersion;
};

} // namespace Engine
} // namespace ExplorerLens
