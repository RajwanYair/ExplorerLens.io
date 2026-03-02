// ShortcutInspector.h — Windows .lnk Shortcut File Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Windows Shell Link (.lnk) binary format to extract target path,
// arguments, working directory, icon location, and hotkey. Generates
// an info-card thumbnail with target details.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct ShortcutInfo {
    bool         isValid = false;
    uint32_t     flags = 0;
    uint32_t     fileAttributes = 0;
    uint64_t     creationTime = 0;
    uint64_t     accessTime = 0;
    uint64_t     writeTime = 0;
    uint32_t     fileSize = 0;     // Target file size
    uint32_t     iconIndex = 0;
    uint32_t     showCommand = 0;  // SW_NORMAL/SW_MAXIMIZED/etc.
    uint16_t     hotKey = 0;
    std::wstring targetPath;
    std::wstring arguments;
    std::wstring workingDirectory;
    std::wstring iconLocation;
    std::wstring description;
    bool         hasTargetIDList = false;
    bool         pointsToFile = false;
    bool         pointsToDirectory = false;
    bool         isRemoteTarget = false;
};

struct ShortcutStats {
    uint32_t filesProcessed = 0;
    uint32_t pointToFiles = 0;
    uint32_t pointToDirs = 0;
    uint32_t remoteTargets = 0;
};

class ShortcutInspector {
public:
    ShortcutInspector() = default;
    ~ShortcutInspector() = default;

    static const wchar_t* GetName() { return L"ShortcutInspector"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".lnk" || e == L".url";
    }

    /// Detect .lnk magic: 4C 00 00 00 (HeaderSize = 0x0000004C)
    /// followed by CLSID 00021401-0000-0000-C000-000000000046
    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 76) return false;
        static const uint8_t lnkMagic[] = { 0x4C, 0x00, 0x00, 0x00 };
        static const uint8_t lnkCLSID[] = {
            0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46
        };
        return memcmp(data, lnkMagic, 4) == 0 &&
            memcmp(data + 4, lnkCLSID, 16) == 0;
    }

    /// Parse .lnk Shell Link header.
    ShortcutInfo Parse(const uint8_t* data, size_t size) const {
        ShortcutInfo info;
        if (!DetectMagic(data, size)) return info;
        info.isValid = true;

        // Header at offset 0, size 0x4C (76 bytes)
        memcpy(&info.flags, data + 20, 4);
        memcpy(&info.fileAttributes, data + 24, 4);
        memcpy(&info.creationTime, data + 28, 8);
        memcpy(&info.accessTime, data + 36, 8);
        memcpy(&info.writeTime, data + 44, 8);
        memcpy(&info.fileSize, data + 52, 4);
        memcpy(&info.iconIndex, data + 56, 4);
        memcpy(&info.showCommand, data + 60, 4);
        memcpy(&info.hotKey, data + 64, 2);

        // Flags interpretation
        info.hasTargetIDList = (info.flags & 0x01) != 0;
        info.pointsToFile = (info.fileAttributes & 0x20) == 0 &&
            (info.fileAttributes & 0x10) == 0;
        info.pointsToDirectory = (info.fileAttributes & 0x10) != 0;
        info.isRemoteTarget = (info.flags & 0x100) != 0;

        return info;
    }

    ShortcutStats GetStats() const { return m_stats; }

private:
    mutable ShortcutStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
