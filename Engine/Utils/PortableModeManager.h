#pragma once
//==============================================================================
// PortableModeManager
// Enables registry-free portable mode for USB/removable drives.
// Configuration stored in local INI files alongside the executable.
//==============================================================================

#include <cstdint>
#include <string>
#include <map>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class PortableStatus : uint8_t {
    Installed   = 0,  // Normal registry-based installation
    Portable    = 1,  // Running from portable directory
    Hybrid      = 2,  // Installed + portable config override
    Unknown     = 3
};

enum class StorageLocation : uint8_t {
    Registry     = 0,
    IniFile      = 1,
    JsonFile     = 2,
    AppData      = 3,
    ExeDirectory = 4,
    LocationCount = 5
};

struct PortableConfig {
    PortableStatus status = PortableStatus::Unknown;
    StorageLocation configLocation = StorageLocation::IniFile;
    std::wstring basePath;
    std::wstring configPath;
    std::wstring cachePath;
    std::wstring pluginPath;
    bool isRemovable = false;
    uint64_t availableSpaceBytes = 0;
};

struct PortableDetectionResult {
    bool isPortable = false;
    PortableStatus status = PortableStatus::Unknown;
    std::wstring exePath;
    std::wstring driveLetter;
    bool hasWriteAccess = false;
    bool hasIniFile = false;
};

class PortableModeManager {
public:
    PortableModeManager();

    PortableDetectionResult Detect() const;
    PortableConfig GetConfig() const { return m_config; }

    bool InitializePortableMode(const std::wstring& basePath);
    bool SaveConfig(const std::map<std::wstring, std::wstring>& settings);
    std::map<std::wstring, std::wstring> LoadConfig() const;

    void SetCacheSize(uint64_t maxBytes) { m_maxCacheBytes = maxBytes; }
    uint64_t GetCacheSize() const { return m_maxCacheBytes; }

    static const wchar_t* GetStatusName(PortableStatus status);
    static const wchar_t* GetLocationName(StorageLocation location);
    static uint32_t GetLocationCount() { return static_cast<uint32_t>(StorageLocation::LocationCount); }

private:
    PortableConfig m_config;
    uint64_t m_maxCacheBytes = 256 * 1024 * 1024; // 256 MB default
};

}} // namespace ExplorerLens::Engine

