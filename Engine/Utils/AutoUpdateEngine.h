//==============================================================================
// DarkThumbs Engine — Sprint 279: Auto-Update Engine
// Silent background update checking, download, and staged rollout.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Update channel
enum class UpdateChannel : uint8_t {
    Stable,         // Production releases
    Beta,           // Pre-release testing
    Canary,         // Nightly/daily builds
    Enterprise,     // Enterprise managed
    COUNT
};

/// Update check result
enum class UpdateCheckResult : uint8_t {
    UpToDate,
    UpdateAvailable,
    MandatoryUpdate,
    CheckFailed,
    NetworkError,
    ServerUnavailable,
    COUNT
};

/// Update download state
enum class DownloadState : uint8_t {
    NotStarted,
    Downloading,
    Verifying,
    Ready,
    Installing,
    Complete,
    Failed,
    COUNT
};

/// Update info
struct UpdateInfo {
    std::wstring    currentVersion;
    std::wstring    latestVersion;
    std::wstring    downloadUrl;
    std::wstring    releaseNotes;
    uint64_t        downloadSizeBytes   = 0;
    std::wstring    sha256Hash;
    bool            isMandatory         = false;
    bool            requiresRestart     = true;
};

/// Auto-update configuration
struct AutoUpdateConfig {
    UpdateChannel   channel         = UpdateChannel::Stable;
    uint32_t        checkIntervalHours  = 24;
    bool            autoDownload    = true;
    bool            autoInstall     = false;
    bool            notifyUser      = true;
    bool            rollbackEnabled = true;
    uint32_t        maxRollbackVersions = 3;
};

/// Auto-update engine
class AutoUpdateEngine {
public:
    static const wchar_t* ChannelName(UpdateChannel c) {
        switch (c) {
            case UpdateChannel::Stable:     return L"Stable";
            case UpdateChannel::Beta:       return L"Beta";
            case UpdateChannel::Canary:     return L"Canary";
            case UpdateChannel::Enterprise: return L"Enterprise";
            default: return L"Unknown";
        }
    }

    static const wchar_t* CheckResultName(UpdateCheckResult r) {
        switch (r) {
            case UpdateCheckResult::UpToDate:           return L"Up to Date";
            case UpdateCheckResult::UpdateAvailable:    return L"Update Available";
            case UpdateCheckResult::MandatoryUpdate:    return L"Mandatory Update";
            case UpdateCheckResult::CheckFailed:        return L"Check Failed";
            case UpdateCheckResult::NetworkError:       return L"Network Error";
            case UpdateCheckResult::ServerUnavailable:  return L"Server Unavailable";
            default: return L"Unknown";
        }
    }

    static const wchar_t* DownloadStateName(DownloadState s) {
        switch (s) {
            case DownloadState::NotStarted:  return L"Not Started";
            case DownloadState::Downloading: return L"Downloading";
            case DownloadState::Verifying:   return L"Verifying";
            case DownloadState::Ready:       return L"Ready";
            case DownloadState::Installing:  return L"Installing";
            case DownloadState::Complete:    return L"Complete";
            case DownloadState::Failed:      return L"Failed";
            default: return L"Unknown";
        }
    }

    static constexpr size_t ChannelCount() { return static_cast<size_t>(UpdateChannel::COUNT); }
    static constexpr size_t CheckResultCount() { return static_cast<size_t>(UpdateCheckResult::COUNT); }
    static constexpr size_t DownloadStateCount() { return static_cast<size_t>(DownloadState::COUNT); }

    /// Parse version string "A.B.C" to components
    static bool ParseVersion(const std::wstring& ver, uint32_t& major, uint32_t& minor, uint32_t& patch) {
        size_t pos1 = ver.find(L'.');
        if (pos1 == std::wstring::npos) return false;
        size_t pos2 = ver.find(L'.', pos1 + 1);
        if (pos2 == std::wstring::npos) return false;
        try {
            major = static_cast<uint32_t>(std::stoul(ver.substr(0, pos1)));
            minor = static_cast<uint32_t>(std::stoul(ver.substr(pos1 + 1, pos2 - pos1 - 1)));
            patch = static_cast<uint32_t>(std::stoul(ver.substr(pos2 + 1)));
            return true;
        } catch (...) { return false; }
    }
};

}} // namespace DarkThumbs::Engine
