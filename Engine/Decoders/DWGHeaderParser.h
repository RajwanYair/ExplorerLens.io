// DWGHeaderParser.h — DWG/DXF Header Magic & Version Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Parses AutoCAD DWG binary header to extract version string (AC1032 etc.)
// and ACADVER field. Provides IsD WG/IsDXF probes and a thumbnail-ready
// version-label overlay renderer for the CAD/BIM decoder chain.
//
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// DWG version codes from the 6-byte magic header ("AC" + 4-digit code)
enum class DWGVersion : uint16_t
{
    Unknown  = 0,
    R1_2     = 12,   // AC1002
    R2_0     = 20,   // AC1006
    R10      = 100,  // AC1009 (most common legacy)
    R11_12   = 112,  // AC1011/AC1012
    R13      = 130,  // AC1013
    R14      = 140,  // AC1014
    R2000    = 150,  // AC1015
    R2004    = 160,  // AC1018
    R2007    = 170,  // AC1021
    R2010    = 180,  // AC1024
    R2013    = 190,  // AC1027
    R2018    = 200,  // AC1032  (current)
};

struct DWGHeaderInfo
{
    DWGVersion version;
    char       versionString[7];  // null-terminated "ACxxxx\0"
    bool       isValid;
    uint32_t   imageDataOffset;   // preview image offset in file (0 if none)
};

class DWGHeaderParser
{
public:
    // Returns true if data starts with "AC" DWG magic and has at least 7 bytes.
    static bool IsDWG(const uint8_t* data, size_t size) noexcept;

    // Returns true if data looks like ASCII DXF (starts with "  0\r\n" or "  0\n").
    static bool IsDXF(const uint8_t* data, size_t size) noexcept;

    // Parse the 6-byte DWG header into DWGHeaderInfo.
    // Returns isValid=false when the magic is unrecognised.
    static DWGHeaderInfo Parse(const uint8_t* data, size_t size) noexcept;

    // Map version string like "AC1032" to DWGVersion enum.
    static DWGVersion StringToVersion(const char* versionString) noexcept;

    // Human-readable label e.g. "AutoCAD 2018 (AC1032)".
    static std::string VersionLabel(DWGVersion v) noexcept;

    // Render a 256×256 BGRA32 placeholder thumbnail with version chip overlay.
    // Returns empty vector if data is not a valid DWG.
    static std::vector<uint8_t> RenderVersionChip(const uint8_t* data, size_t size,
                                                   uint32_t width  = 256,
                                                   uint32_t height = 256);
};

} // namespace Engine
} // namespace ExplorerLens
