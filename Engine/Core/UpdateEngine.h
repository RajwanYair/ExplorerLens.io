#pragma once
// Update Engine — auto-update check, download, verify, and install
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Update channel
enum class UpdateChannel : uint32_t {
    Stable    = 0,
    Beta      = 1,
    Nightly   = 2,
    Enterprise = 3,
    COUNT     = 4
};

/// Update check status
enum class UpdateStatus : uint32_t {
    Unknown      = 0,
    UpToDate     = 1,
    Available    = 2,
    Downloading  = 3,
    Verifying    = 4,
    Ready        = 5,
    Failed       = 6,
    COUNT        = 7
};

/// Info about an available update
struct UpdateInfo {
    std::wstring version;
    std::wstring releaseNotes;
    std::wstring downloadUrl;
    uint64_t     sizeBytes    = 0;
    std::wstring sha256Hash;
    UpdateChannel channel     = UpdateChannel::Stable;
    UpdateStatus  status      = UpdateStatus::Unknown;
};

/// Manages software update lifecycle
class UpdateEngine {
public:
    UpdateEngine();

    static const wchar_t* GetChannelName(UpdateChannel ch);
    static const wchar_t* GetStatusName(UpdateStatus status);
    static uint32_t GetChannelCount() { return static_cast<uint32_t>(UpdateChannel::COUNT); }

    /// Set current version
    void SetCurrentVersion(const std::wstring& ver) { m_currentVersion = ver; }
    std::wstring GetCurrentVersion() const { return m_currentVersion; }

    /// Set update channel
    void SetChannel(UpdateChannel ch) { m_channel = ch; }
    UpdateChannel GetChannel() const { return m_channel; }

    /// Simulate a check for update
    UpdateInfo CheckForUpdate(const std::wstring& latestVersion);
    /// Verify hash of downloaded file
    static bool VerifyHash(const std::wstring& expected, const std::wstring& actual);
    /// Compare version strings (a > b => positive)
    static int CompareVersions(const std::wstring& a, const std::wstring& b);

private:
    std::wstring  m_currentVersion = L"0.0.0";
    UpdateChannel m_channel = UpdateChannel::Stable;
};

}} // namespace ExplorerLens::Engine

