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

enum class DisplayColorSpace : uint8_t {
    sRGB          = 0,   // IEC 61966-2-1 standard sRGB (most monitors)
    DisplayP3     = 1,   // DCI-P3 with D65 white point (wide-gamut)
    AdobeRGB      = 2,   // Adobe RGB 1998
    BT2020        = 3,   // Rec.2020 (HDR displays)
    Unknown       = 0xFF,
};

// ---- ICC Profile Descriptor -------------------------------------------------

struct DisplayProfile {
    std::string          iccPath;           // Absolute path to .icm/.icc file
    std::string          deviceName;        // "\\.\DISPLAY1"
    DisplayColorSpace    colorSpace = DisplayColorSpace::Unknown;
    bool                 isWideGamut = false;
    bool                 isHDR       = false;
    float                whitePointX = 0.3127f;  // D65 CIE xy
    float                whitePointY = 0.3290f;
    std::vector<uint8_t> rawICC;             // Raw ICC bytes (lazily loaded)
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
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens
