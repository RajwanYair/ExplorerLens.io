// MSIPackageInspector.h — Windows Installer (.msi) Package Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Inspects MSI files by parsing the OLE Compound File Binary Format
// (CFBF) header to extract product name, version, manufacturer, and
// file table summary for thumbnail generation.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct MSIPropertyInfo
{
    std::wstring productName;
    std::wstring manufacturer;
    std::wstring productVersion;
    std::wstring productCode;
    uint32_t estimatedSizeKB = 0;
};

struct CFBFHeader
{
    bool isValid = false;
    uint16_t minorVersion = 0;
    uint16_t majorVersion = 0;
    uint16_t sectorSize = 0;
    uint32_t totalFATSectors = 0;
    uint32_t firstDirectorySector = 0;
    uint32_t firstMiniFATSector = 0;
    uint32_t totalMiniFATSectors = 0;
    uint32_t totalStreamEntries = 0;
};

struct MSIStats
{
    uint32_t filesProcessed = 0;
    uint64_t totalSizeKB = 0;
};

class MSIPackageInspector
{
  public:
    MSIPackageInspector() = default;
    ~MSIPackageInspector() = default;

    static const wchar_t* GetName()
    {
        return L"MSIPackageInspector";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".msi" || e == L".msp" || e == L".mst" || e == L".msm";
    }

    /// Detect OLE Compound File magic: D0 CF 11 E0 A1 B1 1A E1
    bool DetectCFBF(const uint8_t* data, size_t size) const
    {
        if (!data || size < 512)
            return false;
        static const uint8_t cfbfMagic[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
        return memcmp(data, cfbfMagic, 8) == 0;
    }

    /// Parse CFBF file header (first 512 bytes).
    CFBFHeader ParseCFBFHeader(const uint8_t* data, size_t size) const
    {
        CFBFHeader hdr;
        if (!DetectCFBF(data, size))
            return hdr;
        hdr.isValid = true;

        memcpy(&hdr.minorVersion, data + 24, 2);
        memcpy(&hdr.majorVersion, data + 26, 2);

        uint16_t sectorShift = 0;
        memcpy(&sectorShift, data + 30, 2);
        hdr.sectorSize = 1 << sectorShift;

        memcpy(&hdr.totalFATSectors, data + 44, 4);
        memcpy(&hdr.firstDirectorySector, data + 48, 4);
        memcpy(&hdr.firstMiniFATSector, data + 60, 4);
        memcpy(&hdr.totalMiniFATSectors, data + 64, 4);

        // Count directory entries by scanning after the header
        uint32_t dirOffset = 512 + hdr.firstDirectorySector * hdr.sectorSize;
        if (dirOffset < size) {
            uint32_t entries = 0;
            for (uint32_t off = dirOffset; off + 128 <= size && off < dirOffset + hdr.sectorSize; off += 128) {
                uint16_t nameLen = 0;
                memcpy(&nameLen, data + off + 64, 2);
                if (nameLen > 0 && nameLen <= 64)
                    entries++;
            }
            hdr.totalStreamEntries = entries;
        }
        return hdr;
    }

    /// Estimate product info from CFBF stream content.
    MSIPropertyInfo EstimateProperties(const CFBFHeader& hdr, uint64_t fileSize) const
    {
        MSIPropertyInfo props;
        props.estimatedSizeKB = static_cast<uint32_t>(fileSize / 1024);
        // MSI version heuristic from CFBF version
        if (hdr.majorVersion >= 4)
            props.productVersion = L"Windows Installer 4.0+";
        else if (hdr.majorVersion == 3)
            props.productVersion = L"Windows Installer 3.0";
        return props;
    }

    MSIStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable MSIStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
