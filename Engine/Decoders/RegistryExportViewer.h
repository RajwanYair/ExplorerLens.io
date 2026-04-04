// RegistryExportViewer.h — Windows Registry Export (.reg) Viewer
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Windows Registry export files (.reg) to extract key paths,
// value names/types, and generate a tree-view thumbnail showing
// the registry structure.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RegExportValueType : uint8_t {
    Sz,
    Dword,
    Qword,
    Binary,
    ExpandSz,
    MultiSz,
    None,
    Unknown
};

struct RegValue
{
    std::wstring name;
    RegExportValueType type = RegExportValueType::Unknown;
    std::wstring data;
};

struct RegKey
{
    std::wstring path;
    std::vector<RegValue> values;
    bool isDelete = false;  // [-HKEY_...] = delete key
};

struct RegistryExportInfo
{
    bool isValid = false;
    uint32_t version = 0;  // 4 = REGEDIT4, 5 = Windows Registry Editor 5.00
    uint32_t keyCount = 0;
    uint32_t valueCount = 0;
    uint32_t deleteCount = 0;
    std::vector<RegKey> topKeys;  // First 10 keys for preview
};

struct RegistryExportStats
{
    uint32_t filesProcessed = 0;
    uint64_t totalKeys = 0;
    uint64_t totalValues = 0;
};

class RegistryExportViewer
{
  public:
    RegistryExportViewer() = default;
    ~RegistryExportViewer() = default;

    static const wchar_t* GetName()
    {
        return L"RegistryExportViewer";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".reg";
    }

    /// Detect file version from header line.
    uint32_t DetectVersion(const std::string& firstLine) const
    {
        if (firstLine.find("Windows Registry Editor Version 5.00") != std::string::npos)
            return 5;
        if (firstLine.find("REGEDIT4") != std::string::npos)
            return 4;
        return 0;
    }

    /// Parse .reg file content.
    RegistryExportInfo Parse(const std::string& content) const
    {
        RegistryExportInfo info;
        if (content.size() < 10)
            return info;

        size_t pos = 0;
        size_t lineEnd = content.find('\n', pos);
        if (lineEnd == std::string::npos)
            return info;

        std::string firstLine = content.substr(0, lineEnd);
        info.version = DetectVersion(firstLine);
        if (info.version == 0)
            return info;
        info.isValid = true;

        pos = lineEnd + 1;
        while (pos < content.size()) {
            // Skip blank lines
            while (pos < content.size() && (content[pos] == '\n' || content[pos] == '\r'))
                pos++;
            if (pos >= content.size())
                break;

            // Check for key header [HKEY_...] or [-HKEY_...]
            if (content[pos] == '[') {
                lineEnd = content.find(']', pos);
                if (lineEnd == std::string::npos)
                    break;

                RegKey key;
                std::string keyPath = content.substr(pos + 1, lineEnd - pos - 1);
                if (!keyPath.empty() && keyPath[0] == '-') {
                    key.isDelete = true;
                    keyPath = keyPath.substr(1);
                    info.deleteCount++;
                }
                key.path = std::wstring(keyPath.begin(), keyPath.end());
                info.keyCount++;

                if (info.topKeys.size() < 10)
                    info.topKeys.push_back(key);

                pos = lineEnd + 1;
            } else {
                // Value line: "name"=type:data
                lineEnd = content.find('\n', pos);
                if (lineEnd == std::string::npos)
                    lineEnd = content.size();
                info.valueCount++;
                pos = lineEnd + 1;
            }
        }
        return info;
    }

    RegistryExportStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable RegistryExportStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
