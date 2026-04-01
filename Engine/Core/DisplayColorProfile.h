// DisplayColorProfile.h — Display ICC Profile Loader for Color-Managed Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Loads the ICC profile associated with each HMONITOR (via ICM API),
// enabling color-managed thumbnail rendering to match the display gamut.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Color Space Tags -------------------------------------------------------

// Color space identifiers for ICC profile matching
enum class DisplayColorSpace : uint8_t {
    // IEC 61966-2-1 standard sRGB (most consumer monitors)
    sRGB          = 0,
    // DCI-P3 gamut with D65 white point (wide-gamut displays)
    DisplayP3     = 1,
    // Adobe RGB 1998
    AdobeRGB      = 2,
    // Rec.2020 (HDR-capable displays)
    BT2020        = 3,
    Unknown       = 0xFF,
};

// ---- ICC Profile Descriptor -------------------------------------------------

struct DisplayProfile {
    // Absolute file system path to the .icm/.icc profile
    std::string          iccPath;
    // GDI device name, e.g. "\\.\DISPLAY1"
    std::string          deviceName;
    DisplayColorSpace    colorSpace = DisplayColorSpace::Unknown;
    bool                 isWideGamut = false;
    bool                 isHDR       = false;
    // D65 CIE xy chromaticity coordinates
    float                whitePointX = 0.3127f;
    float                whitePointY = 0.3290f;
    // Raw ICC profile bytes; populated on first access (lazy load)
    std::vector<uint8_t> rawICC;
};

// ---- Loader -----------------------------------------------------------------

class DisplayColorProfileLoader {
public:
    DisplayColorProfileLoader();
    ~DisplayColorProfileLoader();

    // Load ICC profile for the given HMONITOR handle.
    bool LoadForMonitor(void* hmonitor, DisplayProfile& outProfile) const;

    // Load sRGB fallback profile (always succeeds).
    static DisplayProfile SRGBFallback();

    // Detect the color space from a loaded ICC profile's header.
    static DisplayColorSpace DetectColorSpace(const std::vector<uint8_t>& rawICC);

    // Check if a display needs gamut-mapping (non-sRGB profiles).
    static bool NeedsGamutMapping(const DisplayProfile& profile);

    // Refresh profile list for all monitors (call on WM_DISPLAYCHANGE).
    void RefreshAll(std::vector<DisplayProfile>& outProfiles) const;

private:
    struct Impl {};
    std::unique_ptr<Impl> m_impl;
};

enum class MonitorProfileType : uint8_t {
    sRGB     = 0,
    DCI_P3   = 1,
    AdobeRGB = 2,
    BT2020   = 3,
    HDR10    = 4,
    Custom   = 5
};

class MultiMonitorColorProfile {
public:
    static int ProfileTypeCount() { return 6; }
    static const wchar_t* ProfileTypeName(MonitorProfileType t) {
        static const wchar_t* names[] = {
            L"sRGB", L"DCI-P3", L"Adobe RGB", L"BT.2020", L"HDR10", L"Custom"
        };
        return names[static_cast<uint8_t>(t)];
    }
    static bool NeedsGamutMapping(MonitorProfileType src, MonitorProfileType dst) {
        return src != dst;
    }
    MultiMonitorColorProfile() = delete;
};

} // namespace Engine
} // namespace ExplorerLens
