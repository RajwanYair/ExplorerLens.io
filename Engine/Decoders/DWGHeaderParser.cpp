// DWGHeaderParser.cpp — DWG/DXF Header Magic & Version Detection
// Copyright (c) 2026 ExplorerLens Project
//
#include "DWGHeaderParser.h"
#include <cstring>
#include <cstdio>

namespace ExplorerLens {
namespace Engine {

bool DWGHeaderParser::IsDWG(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 7) return false;
    return data[0] == 'A' && data[1] == 'C';
}

bool DWGHeaderParser::IsDXF(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 4) return false;
    // DXF files start with group code "  0\r\n" or "  0\n"
    const char* p = reinterpret_cast<const char*>(data);
    return (p[0] == ' ' && p[1] == ' ' && p[2] == '0' && (p[3] == '\r' || p[3] == '\n'));
}

DWGHeaderInfo DWGHeaderParser::Parse(const uint8_t* data, size_t size) noexcept
{
    DWGHeaderInfo info{};
    info.isValid = false;
    if (!IsDWG(data, size)) return info;

    memcpy(info.versionString, data, 6);
    info.versionString[6] = '\0';
    info.version   = StringToVersion(info.versionString);
    info.isValid   = (info.version != DWGVersion::Unknown);

    // Preview image offset is at byte offset 13 in DWG R14+ (AC1014..AC1032)
    if (size >= 16 && info.version >= DWGVersion::R14)
    {
        uint32_t off = 0;
        memcpy(&off, data + 13, sizeof(uint32_t));
        info.imageDataOffset = off;
    }
    return info;
}

DWGVersion DWGHeaderParser::StringToVersion(const char* v) noexcept
{
    if (!v) return DWGVersion::Unknown;
    if (memcmp(v, "AC1002", 6) == 0) return DWGVersion::R1_2;
    if (memcmp(v, "AC1006", 6) == 0) return DWGVersion::R2_0;
    if (memcmp(v, "AC1009", 6) == 0) return DWGVersion::R10;
    if (memcmp(v, "AC1011", 6) == 0) return DWGVersion::R11_12;
    if (memcmp(v, "AC1012", 6) == 0) return DWGVersion::R11_12;
    if (memcmp(v, "AC1013", 6) == 0) return DWGVersion::R13;
    if (memcmp(v, "AC1014", 6) == 0) return DWGVersion::R14;
    if (memcmp(v, "AC1015", 6) == 0) return DWGVersion::R2000;
    if (memcmp(v, "AC1018", 6) == 0) return DWGVersion::R2004;
    if (memcmp(v, "AC1021", 6) == 0) return DWGVersion::R2007;
    if (memcmp(v, "AC1024", 6) == 0) return DWGVersion::R2010;
    if (memcmp(v, "AC1027", 6) == 0) return DWGVersion::R2013;
    if (memcmp(v, "AC1032", 6) == 0) return DWGVersion::R2018;
    return DWGVersion::Unknown;
}

std::string DWGHeaderParser::VersionLabel(DWGVersion v) noexcept
{
    switch (v)
    {
        case DWGVersion::R1_2:    return "AutoCAD R1.2 (AC1002)";
        case DWGVersion::R2_0:    return "AutoCAD R2.0 (AC1006)";
        case DWGVersion::R10:     return "AutoCAD R10 (AC1009)";
        case DWGVersion::R11_12:  return "AutoCAD R11/R12 (AC1011/12)";
        case DWGVersion::R13:     return "AutoCAD R13 (AC1013)";
        case DWGVersion::R14:     return "AutoCAD R14 (AC1014)";
        case DWGVersion::R2000:   return "AutoCAD 2000 (AC1015)";
        case DWGVersion::R2004:   return "AutoCAD 2004 (AC1018)";
        case DWGVersion::R2007:   return "AutoCAD 2007 (AC1021)";
        case DWGVersion::R2010:   return "AutoCAD 2010 (AC1024)";
        case DWGVersion::R2013:   return "AutoCAD 2013 (AC1027)";
        case DWGVersion::R2018:   return "AutoCAD 2018 (AC1032)";
        default:                  return "Unknown DWG version";
    }
}

std::vector<uint8_t> DWGHeaderParser::RenderVersionChip(
    const uint8_t* data, size_t size, uint32_t width, uint32_t height)
{
    const auto info = Parse(data, size);
    const uint32_t pixCount = width * height;
    std::vector<uint8_t> bgra(static_cast<size_t>(pixCount) * 4, 0);

    // Dark grey background
    for (uint32_t i = 0; i < pixCount; ++i)
    {
        bgra[i * 4 + 0] = 0x30; // B
        bgra[i * 4 + 1] = 0x30; // G
        bgra[i * 4 + 2] = 0x30; // R
        bgra[i * 4 + 3] = 0xFF; // A
    }

    if (!info.isValid) return bgra;

    // Blue accent band across the lower quarter (version chip)
    const uint32_t bandY = height * 3 / 4;
    for (uint32_t y = bandY; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            const uint32_t idx = (y * width + x) * 4;
            bgra[idx + 0] = 0x80; // B
            bgra[idx + 1] = 0x40; // G
            bgra[idx + 2] = 0x10; // R
            bgra[idx + 3] = 0xFF;
        }
    }
    return bgra;
}

} // namespace Engine
} // namespace ExplorerLens
