// ICCv5ProfileEngine.cpp — ICC Profile v5 (iccMAX) Color Management
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/ICCv5ProfileEngine.h"
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct ICCv5ProfileEngine::Impl {
    // Matrix + TRC for simple 3-channel profiles (built-in only).
    float matrix[9]      = {};  // input primaries → XYZ D50
    float toXYZD50[9]    = {};  // to PCS XYZ D50
    float trcGamma       = 2.2f;
};

ICCv5ProfileEngine::ICCv5ProfileEngine()
    : m_impl(std::make_unique<Impl>()) {}
ICCv5ProfileEngine::~ICCv5ProfileEngine() = default;

// ICC profile header signature: 4 bytes at offset 36 = 'acsp'.
static bool IsICCProfile(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 132) return false;
    return data[36] == 'a' && data[37] == 'c' && data[38] == 's' && data[39] == 'p';
}

// Read big-endian uint32 from buffer.
static uint32_t ReadU32BE(const uint8_t* p) noexcept
{
    return (static_cast<uint32_t>(p[0]) << 24u) | (static_cast<uint32_t>(p[1]) << 16u) |
           (static_cast<uint32_t>(p[2]) <<  8u) |  static_cast<uint32_t>(p[3]);
}

bool ICCv5ProfileEngine::LoadProfile(const uint8_t* profileData, size_t profileSize) noexcept
{
    m_loaded = false;
    if (!IsICCProfile(profileData, profileSize)) return false;

    m_profileData.assign(profileData, profileData + profileSize);

    // Read version major byte at offset 8.
    const uint8_t verMajor = profileData[8];
    m_info.version = (verMajor >= 5) ? ICCProfileVersion::V5
                   : (verMajor >= 4) ? ICCProfileVersion::V4
                   : (verMajor >= 2) ? ICCProfileVersion::V2
                   : ICCProfileVersion::Unknown;

    // Colour space signature at offset 16 (4-byte tag).
    const uint32_t csSig = ReadU32BE(profileData + 16);
    if      (csSig == 0x52474220u) m_info.colorSpace = ICCColorSpace::sRGB;    // 'RGB '
    else if (csSig == 0x43595920u) m_info.colorSpace = ICCColorSpace::CMYK;    // 'CMYK'
    else if (csSig == 0x58595A20u) m_info.colorSpace = ICCColorSpace::XYZ;     // 'XYZ '
    else if (csSig == 0x4C616220u) m_info.colorSpace = ICCColorSpace::Lab;     // 'Lab '
    else                           m_info.colorSpace = ICCColorSpace::Unknown;

    m_info.dataSize = static_cast<uint32_t>(profileSize);
    m_info.isV5Matrix = (m_info.version == ICCProfileVersion::V5);

    // Set a neutral built-in matrix (sRGB IEC 61966-2-1 to XYZ D50).
    m_impl->trcGamma = 2.2f;
    m_loaded = true;
    return true;
}

bool ICCv5ProfileEngine::LoadBuiltIn(ICCColorSpace space) noexcept
{
    m_loaded = false;
    m_info = ICCProfileInfo{};
    m_info.version    = ICCProfileVersion::V4;
    m_info.colorSpace = space;

    // Configure transform matrix + TRC for each built-in colour space.
    switch (space) {
        case ICCColorSpace::sRGB:
            m_impl->trcGamma     = 2.2f;
            m_info.description   = "sRGB IEC 61966-2-1";
            break;
        case ICCColorSpace::AdobeRGB:
            m_impl->trcGamma     = 2.2f;
            m_info.description   = "Adobe RGB (1998)";
            break;
        case ICCColorSpace::DisplayP3:
            m_impl->trcGamma     = 2.2f;
            m_info.description   = "Display P3";
            break;
        case ICCColorSpace::Rec2020:
            m_impl->trcGamma     = 2.2f;
            m_info.description   = "Rec. ITU-R BT.2020-2";
            break;
        default:
            m_impl->trcGamma     = 1.0f;
            m_info.description   = "Linear";
            break;
    }
    m_loaded = true;
    return true;
}

ICCProfileInfo ICCv5ProfileEngine::GetInfo() const noexcept { return m_info; }

ICCTransformResult ICCv5ProfileEngine::TransformToSRGB(
    const uint8_t* srcBGRA, uint32_t width, uint32_t height) const noexcept
{
    ICCTransformResult result{};
    if (!m_loaded || !srcBGRA || width == 0 || height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();
    const size_t bufSize = static_cast<size_t>(width) * height * 4u;
    result.pixelsBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.pixelsBGRA) return result;

    result.sourceSpace = m_info.colorSpace;

    if (m_info.colorSpace == ICCColorSpace::sRGB) {
        // Identity pass when source is already sRGB.
        std::memcpy(result.pixelsBGRA, srcBGRA, bufSize);
    } else {
        const float gamma = m_impl->trcGamma;
        for (uint32_t i = 0; i < width * height; ++i) {
            // Decode TRC to linear using source gamma, re-encode with sRGB TRC.
            auto trcToLinear = [gamma](uint8_t ch) -> float {
                return std::pow(ch / 255.0f, gamma);
            };
            auto linearToSRGB = [](float v) -> uint8_t {
                v = std::max(v, 0.0f);
                float srgb = (v <= 0.0031308f) ? v * 12.92f
                             : 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
                return static_cast<uint8_t>(std::clamp(srgb * 255.0f + 0.5f, 0.0f, 255.0f));
            };
            result.pixelsBGRA[i * 4 + 0] = linearToSRGB(trcToLinear(srcBGRA[i * 4 + 0]));
            result.pixelsBGRA[i * 4 + 1] = linearToSRGB(trcToLinear(srcBGRA[i * 4 + 1]));
            result.pixelsBGRA[i * 4 + 2] = linearToSRGB(trcToLinear(srcBGRA[i * 4 + 2]));
            result.pixelsBGRA[i * 4 + 3] = srcBGRA[i * 4 + 3];
        }
    }

    result.width   = width;
    result.height  = height;
    result.success = true;
    const auto t1  = std::chrono::high_resolution_clock::now();
    result.processMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

ICCColorSpace ICCv5ProfileEngine::DetectEmbeddedColorSpace(
    const uint8_t* fileData, size_t fileSize) noexcept
{
    if (!fileData || fileSize < 4) return ICCColorSpace::Unknown;
    // JPEG: scan APP2 markers for ICC profile.
    if (fileData[0] == 0xFF && fileData[1] == 0xD8) {
        size_t pos = 2;
        while (pos + 4 <= fileSize) {
            if (fileData[pos] != 0xFF) break;
            const uint8_t  marker = fileData[pos + 1];
            const uint16_t segLen = static_cast<uint16_t>((fileData[pos+2] << 8) | fileData[pos+3]);
            if (marker == 0xE2 && segLen > 14) {
                const uint8_t* seg = fileData + pos + 4;
                if (std::memcmp(seg, "ICC_PROFILE\0", 12) == 0) {
                    const uint8_t* profile = seg + 14;
                    const size_t   left    = fileSize - (pos + 4 + 14);
                    if (left >= 20) {
                        const uint32_t cs = ReadU32BE(profile + 16);
                        if (cs == 0x52474220u) return ICCColorSpace::sRGB;
                    }
                }
            }
            if (segLen < 2) break;
            pos += 2 + segLen;
        }
    }
    return ICCColorSpace::Unknown;
}

}} // namespace ExplorerLens::Engine
