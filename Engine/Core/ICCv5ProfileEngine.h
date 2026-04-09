// ICCv5ProfileEngine.h — ICC Profile v5 (iccMAX) Color Management
// Copyright (c) 2026 ExplorerLens Project
//
// Reads ICC v4 and v5 (iccMAX / ICC.2) color profiles and applies the
// color transform to thumbnail pixel data. Supports multi-spectral
// profiles, per-channel curves, and matrix+TRC transforms. Falls back
// gracefully to ICC v4 path when v5 elements are not present.
// P50 target: < 2 ms per thumbnail.
//
#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class ICCProfileVersion : uint8_t {
    Unknown = 0,
    V2      = 2,
    V4      = 4,
    V5      = 5,   // iccMAX (ICC.2)
};

enum class ICCColorSpace : uint8_t {
    Unknown       = 0,
    sRGB          = 1,
    AdobeRGB      = 2,
    DisplayP3     = 3,
    ProPhotoRGB   = 4,
    Rec2020       = 5,
    ACES          = 6,
    ACEScg        = 7,
    LinearSRGB    = 8,
    CMYK          = 9,
    Lab           = 10,
    XYZ           = 11,
};

struct ICCProfileInfo {
    ICCProfileVersion version     = ICCProfileVersion::Unknown;
    ICCColorSpace     colorSpace  = ICCColorSpace::Unknown;
    std::string       description;
    bool              hasLUT      = false;  // true if profile uses LUT-based transform
    bool              isV5Matrix  = false;  // true if v5 parametric matrix/TRC
    uint32_t          dataSize    = 0;
};

struct ICCTransformResult {
    uint8_t* pixelsBGRA = nullptr;  // Caller-owned; width × height × 4 bytes
    uint32_t width      = 0;
    uint32_t height     = 0;
    bool     success    = false;
    float    processMs  = 0.0f;
    ICCColorSpace sourceSpace = ICCColorSpace::Unknown;
};

class ICCv5ProfileEngine {
public:
    ICCv5ProfileEngine();
    ~ICCv5ProfileEngine();

    // Parse an ICC profile from raw bytes.
    bool LoadProfile(const uint8_t* profileData, size_t profileSize) noexcept;

    // Load a built-in profile by color space name.
    bool LoadBuiltIn(ICCColorSpace space) noexcept;

    // Get metadata about the loaded profile.
    ICCProfileInfo GetInfo() const noexcept;

    // Apply the loaded source profile → sRGB transform to BGRA8 pixels in-place.
    ICCTransformResult TransformToSRGB(
        const uint8_t* srcBGRA,
        uint32_t       width,
        uint32_t       height) const noexcept;

    // Quick probe: detect ICC profile color space from embedded JPEG/PNG/TIFF bytes.
    static ICCColorSpace DetectEmbeddedColorSpace(
        const uint8_t* fileData, size_t fileSize) noexcept;

    bool IsLoaded() const noexcept { return m_loaded; }

private:
    bool m_loaded = false;
    ICCProfileInfo m_info;
    std::vector<uint8_t> m_profileData;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}} // namespace ExplorerLens::Engine
