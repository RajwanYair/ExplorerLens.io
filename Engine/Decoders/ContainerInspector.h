// ContainerInspector.h — Container Format Structure Analyzer
// Copyright (c) 2026 ExplorerLens Project
//
// Inspects container file formats (ISO, VHD, DMG, disk images) to
// enumerate contents and identify thumbnail candidates.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ContainerType : uint8_t {
    ISO9660 = 0,
    UDF = 1,
    VHD = 2,
    VHDX = 3,
    DMG = 4,
    WIM = 5,
    ESD = 6,
    MSI = 7,
    CAB = 8,
    AppX = 9,
    MSIX = 10,
    Unknown = 255
};

struct ContainerEntry
{
    std::wstring path;
    uint64_t size = 0;
    bool isExecutable = false;
    bool isImage = false;
    bool isManifest = false;
};

struct ContainerMetadata
{
    ContainerType type = ContainerType::Unknown;
    std::wstring volumeLabel;
    uint64_t totalSize = 0;
    uint32_t fileCount = 0;
    uint32_t directoryCount = 0;
    bool hasAutorun = false;
    bool hasBrandingAssets = false;
    std::vector<ContainerEntry> thumbnailCandidates;
};

struct InspectorConfig
{
    uint32_t maxEntriesToScan = 500;
    uint32_t maxCandidates = 4;
    bool searchForIcons = true;
    bool searchForBranding = true;
    bool searchForScreenshots = true;
};

class ContainerInspector
{
  public:
    void Configure(const InspectorConfig& config)
    {
        m_config = config;
    }

    ContainerType DetectType(const uint8_t* header, size_t headerSize) const
    {
        if (headerSize < 8)
            return ContainerType::Unknown;
        // ISO: "CD001" at offset 32769
        if (headerSize > 32773 && header[32769] == 'C' && header[32770] == 'D' && header[32771] == '0'
            && header[32772] == '0' && header[32773] == '1')
            return ContainerType::ISO9660;
        // VHD: "conectix" at offset 0
        if (headerSize >= 8 && header[0] == 'c' && header[1] == 'o' && header[2] == 'n' && header[3] == 'e')
            return ContainerType::VHD;
        // VHDX: "vhdxfile" at offset 0
        if (headerSize >= 8 && header[0] == 'v' && header[1] == 'h' && header[2] == 'd' && header[3] == 'x')
            return ContainerType::VHDX;
        // CAB: "MSCF" at offset 0
        if (headerSize >= 4 && header[0] == 'M' && header[1] == 'S' && header[2] == 'C' && header[3] == 'F')
            return ContainerType::CAB;
        return ContainerType::Unknown;
    }

    bool IsBrandingAsset(const std::wstring& name) const
    {
        std::wstring lower = name;
        for (auto& c : lower)
            c = towlower(c);
        return lower.find(L"icon") != std::wstring::npos || lower.find(L"logo") != std::wstring::npos
               || lower.find(L"banner") != std::wstring::npos || lower.find(L"appxmanifest") != std::wstring::npos;
    }

    InspectorConfig GetConfig() const
    {
        return m_config;
    }

  private:
    InspectorConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
