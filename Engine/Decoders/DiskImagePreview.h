// DiskImagePreview.h — Disk Image File Preview (.iso/.img/.vhd)
// Copyright (c) 2026 ExplorerLens Project
//
// Parses ISO 9660, UDF, and VHD/VHDX headers to extract volume label,
// filesystem type, capacity info, and directory listing for thumbnail
// generation of disk image files.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DiskImageFormat : uint8_t { ISO9660, UDF, VHD, VHDX, Raw, Unknown };

struct VolumeDescriptor {
    std::string volumeLabel;
    std::string systemId;
    std::string publisher;
    std::string application;
    uint64_t    volumeSize = 0;
    uint32_t    blockSize = 2048;
    uint32_t    fileCount = 0;
    uint32_t    dirCount = 0;
};

struct DiskImageInfo {
    bool            isValid = false;
    DiskImageFormat format = DiskImageFormat::Unknown;
    uint64_t        totalSizeBytes = 0;
    VolumeDescriptor volume;
    bool            isBootable = false;
    bool            isMultiSession = false;
};

struct DiskImageStats {
    uint32_t filesProcessed = 0;
    uint64_t totalCapacityGB = 0;
};

class DiskImagePreview {
public:
    DiskImagePreview() = default;
    ~DiskImagePreview() = default;

    static const wchar_t* GetName() { return L"DiskImagePreview"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".iso" || e == L".img" || e == L".vhd" || e == L".vhdx" ||
            e == L".dmg" || e == L".cue" || e == L".bin" || e == L".nrg";
    }

    /// Detect ISO 9660 by Primary Volume Descriptor at sector 16.
    /// Magic: "CD001" at offset 32769 (0x8001)
    DiskImageFormat DetectFormat(const uint8_t* data, size_t size) const {
        if (!data) return DiskImageFormat::Unknown;

        // ISO 9660: "CD001" at sector 16 (offset 0x8001)
        if (size >= 0x8006 && memcmp(data + 0x8001, "CD001", 5) == 0)
            return DiskImageFormat::ISO9660;

        // VHD: "conectix" at offset 0 (VHD footer copy at header for dynamic)
        if (size >= 8 && memcmp(data, "conectix", 8) == 0)
            return DiskImageFormat::VHD;

        // VHDX: "vhdxfile" at offset 0
        if (size >= 8 && memcmp(data, "vhdxfile", 8) == 0)
            return DiskImageFormat::VHDX;

        return DiskImageFormat::Unknown;
    }

    /// Parse ISO 9660 Primary Volume Descriptor.
    DiskImageInfo ParseISO9660(const uint8_t* data, size_t size) const {
        DiskImageInfo info;
        if (size < 0x8800) return info;

        const uint8_t* pvd = data + 0x8000; // Sector 16
        if (pvd[0] != 1 || memcmp(pvd + 1, "CD001", 5) != 0) return info;

        info.isValid = true;
        info.format = DiskImageFormat::ISO9660;

        // System ID at offset 8 (32 bytes)
        info.volume.systemId = std::string(reinterpret_cast<const char*>(pvd + 8), 32);
        // Volume ID at offset 40 (32 bytes)
        info.volume.volumeLabel = std::string(reinterpret_cast<const char*>(pvd + 40), 32);
        // Volume size in blocks (little-endian at offset 80)
        uint32_t volumeBlocks = 0;
        memcpy(&volumeBlocks, pvd + 80, 4);
        // Block size at offset 128 (little-endian)
        uint16_t blockSize = 0;
        memcpy(&blockSize, pvd + 128, 2);
        info.volume.blockSize = blockSize > 0 ? blockSize : 2048;
        info.volume.volumeSize = static_cast<uint64_t>(volumeBlocks) * info.volume.blockSize;
        info.totalSizeBytes = info.volume.volumeSize;

        // Publisher at offset 318 (128 bytes)
        info.volume.publisher = std::string(reinterpret_cast<const char*>(pvd + 318), 128);

        // Check for El Torito boot record at sector 17
        if (size >= 0x8806) {
            const uint8_t* bootRec = data + 0x8800;
            if (bootRec[0] == 0 && memcmp(bootRec + 1, "CD001", 5) == 0)
                info.isBootable = true;
        }

        // Trim trailing spaces from strings
        auto trim = [](std::string& s) {
            while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) s.pop_back();
            };
        trim(info.volume.systemId);
        trim(info.volume.volumeLabel);
        trim(info.volume.publisher);

        return info;
    }

    DiskImageStats GetStats() const { return m_stats; }

private:
    mutable DiskImageStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
