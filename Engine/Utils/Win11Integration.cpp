//==============================================================================
// Win11Integration
// Windows 11 native features
//==============================================================================

#include "Win11Integration.h"
#include <Windows.h>

namespace ExplorerLens {
namespace Engine {

Win11Integration::Win11Integration()
{
    DetectOS();
}

//------------------------------------------------------------------------------
void Win11Integration::DetectOS()
{
    // Use RtlGetVersion for accurate version detection
    typedef LONG(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        auto fn = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(ntdll, "RtlGetVersion"));
        if (fn) {
            RTL_OSVERSIONINFOW vi = {};
            vi.dwOSVersionInfoSize = sizeof(vi);
            if (fn(&vi) == 0) {
                m_versionInfo.major = vi.dwMajorVersion;
                m_versionInfo.minor = vi.dwMinorVersion;
                m_versionInfo.build = vi.dwBuildNumber;
            }
        }
    }

    m_versionInfo.isWin11 = (m_versionInfo.build >= 22000);
    m_versionInfo.isWin11_22H2 = (m_versionInfo.build >= 22621);
    m_versionInfo.isWin11_23H2 = (m_versionInfo.build >= 22631);
    m_versionInfo.isWin11_24H2 = (m_versionInfo.build >= 26100);

    if (m_versionInfo.isWin11_24H2)
        m_versionInfo.displayName = L"Windows 11 24H2";
    else if (m_versionInfo.isWin11_23H2)
        m_versionInfo.displayName = L"Windows 11 23H2";
    else if (m_versionInfo.isWin11_22H2)
        m_versionInfo.displayName = L"Windows 11 22H2";
    else if (m_versionInfo.isWin11)
        m_versionInfo.displayName = L"Windows 11";
    else
        m_versionInfo.displayName = L"Windows 10";

    if (m_versionInfo.isWin11) {
        m_micaMode = MicaMode::Mica;
    }
}

WindowsVersionInfo Win11Integration::DetectVersion() const
{
    return m_versionInfo;
}

//------------------------------------------------------------------------------
bool Win11Integration::IsFeatureAvailable(Win11Feature feature) const
{
    switch (feature) {
        case Win11Feature::DarkMode:
            return true;  // Supported on Win10+
        case Win11Feature::RoundedCorners:
        case Win11Feature::MicaMaterial:
        case Win11Feature::SnapLayouts:
            return m_versionInfo.isWin11;
        case Win11Feature::Widgets:
            return m_versionInfo.isWin11_22H2;
        case Win11Feature::DirectStorage:
            return m_versionInfo.isWin11;
        case Win11Feature::AutoHDR:
            return m_versionInfo.isWin11;
        case Win11Feature::WindowedGames:
            return m_versionInfo.isWin11_22H2;
        default:
            return false;
    }
}

std::vector<Win11Feature> Win11Integration::GetAvailableFeatures() const
{
    std::vector<Win11Feature> features;
    for (uint8_t i = 0; i < static_cast<uint8_t>(Win11Feature::FeatureCount); i++) {
        auto f = static_cast<Win11Feature>(i);
        if (IsFeatureAvailable(f))
            features.push_back(f);
    }
    return features;
}

uint32_t Win11Integration::GetAvailableFeatureCount() const
{
    uint32_t count = 0;
    for (uint8_t i = 0; i < static_cast<uint8_t>(Win11Feature::FeatureCount); i++) {
        if (IsFeatureAvailable(static_cast<Win11Feature>(i)))
            count++;
    }
    return count;
}

//------------------------------------------------------------------------------
bool Win11Integration::IsDarkModeEnabled() const
{
    return IsSystemDarkMode();
}

bool Win11Integration::IsSystemDarkMode() const
{
    HKEY hKey = nullptr;
    DWORD value = 0, size = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0,
                      KEY_READ, &hKey)
        == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(hKey);
    }
    return (value == 0);  // 0 = dark mode, 1 = light mode
}

//------------------------------------------------------------------------------
std::vector<SnapZone> Win11Integration::GetSnapZones(uint32_t monitorWidth, uint32_t monitorHeight) const
{
    std::vector<SnapZone> zones;
    if (!m_versionInfo.isWin11)
        return zones;

    // Standard 4-zone snap layout
    uint32_t halfW = monitorWidth / 2;
    uint32_t halfH = monitorHeight / 2;

    zones.push_back({0, 0, 0, halfW, halfH, 1.0f});                            // Top-left
    zones.push_back({1, (int32_t)halfW, 0, halfW, halfH, 1.0f});               // Top-right
    zones.push_back({2, 0, (int32_t)halfH, halfW, halfH, 1.0f});               // Bottom-left
    zones.push_back({3, (int32_t)halfW, (int32_t)halfH, halfW, halfH, 1.0f});  // Bottom-right
    return zones;
}

//------------------------------------------------------------------------------
const wchar_t* Win11Integration::GetFeatureName(Win11Feature feature)
{
    switch (feature) {
        case Win11Feature::RoundedCorners:
            return L"Rounded Corners";
        case Win11Feature::MicaMaterial:
            return L"Mica Material";
        case Win11Feature::SnapLayouts:
            return L"Snap Layouts";
        case Win11Feature::Widgets:
            return L"Widgets";
        case Win11Feature::DarkMode:
            return L"Dark Mode";
        case Win11Feature::DirectStorage:
            return L"DirectStorage";
        case Win11Feature::AutoHDR:
            return L"Auto HDR";
        case Win11Feature::WindowedGames:
            return L"Windowed Games";
        default:
            return L"Unknown";
    }
}

const wchar_t* Win11Integration::GetMicaModeName(MicaMode mode)
{
    switch (mode) {
        case MicaMode::None:
            return L"None";
        case MicaMode::Mica:
            return L"Mica";
        case MicaMode::MicaAlt:
            return L"Mica Alt";
        case MicaMode::Acrylic:
            return L"Acrylic";
        default:
            return L"Unknown";
    }
}

uint32_t Win11Integration::GetFeatureCount()
{
    return static_cast<uint32_t>(Win11Feature::FeatureCount);
}

}  // namespace Engine
}  // namespace ExplorerLens
