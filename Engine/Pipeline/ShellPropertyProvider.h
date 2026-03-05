// ShellPropertyProvider.h — IPropertyStore Metadata for Explorer
// Copyright (c) 2026 ExplorerLens Project
//
// Provides Windows Shell property store integration for exposing file metadata
// (dimensions, format, codec) to Explorer's Details pane and column view.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class ShellPropertyCategory : uint8_t {
    Image = 0,
    Video = 1,
    Document = 2,
    Archive = 3,
    Audio = 4,
    General = 5
};

enum class ShellPropertyType : uint8_t {
    String = 0,
    UInt32 = 1,
    UInt64 = 2,
    Double = 3,
    Boolean = 4,
    DateTime = 5
};

struct ShellPropertyEntry {
    std::wstring          name;
    std::wstring          displayName;
    std::wstring          stringValue;
    uint64_t              numericValue = 0;
    double                doubleValue = 0.0;
    ShellPropertyCategory category = ShellPropertyCategory::General;
    ShellPropertyType     type = ShellPropertyType::String;
    bool                  isReadOnly = true;
};

struct ShellPropertySet {
    std::wstring                    filePath;
    std::vector<ShellPropertyEntry> entries;
    ShellPropertyCategory           primaryCategory = ShellPropertyCategory::General;
};

class ShellPropertyProvider {
public:
    static ShellPropertyProvider& Instance() { static ShellPropertyProvider s; return s; }

    bool SetProperty(const std::wstring& name, const ShellPropertyEntry& entry) {
        if (name.empty()) return false;
        m_properties[name] = entry;
        return true;
    }

    bool SetStringProperty(const std::wstring& name, const std::wstring& displayName,
        const std::wstring& value, ShellPropertyCategory cat = ShellPropertyCategory::General) {
        ShellPropertyEntry entry;
        entry.name = name;
        entry.displayName = displayName;
        entry.stringValue = value;
        entry.category = cat;
        entry.type = ShellPropertyType::String;
        m_properties[name] = entry;
        return true;
    }

    bool SetNumericProperty(const std::wstring& name, const std::wstring& displayName,
        uint64_t value, ShellPropertyCategory cat = ShellPropertyCategory::General) {
        ShellPropertyEntry entry;
        entry.name = name;
        entry.displayName = displayName;
        entry.numericValue = value;
        entry.category = cat;
        entry.type = ShellPropertyType::UInt64;
        m_properties[name] = entry;
        return true;
    }

    ShellPropertySet GetProperties(ShellPropertyCategory filter) const {
        ShellPropertySet set;
        set.primaryCategory = filter;
        for (const auto& [name, entry] : m_properties) {
            if (entry.category == filter || filter == ShellPropertyCategory::General) {
                set.entries.push_back(entry);
            }
        }
        return set;
    }

    ShellPropertySet GetAllProperties() const {
        ShellPropertySet set;
        for (const auto& [name, entry] : m_properties)
            set.entries.push_back(entry);
        return set;
    }

    const ShellPropertyEntry* FindProperty(const std::wstring& name) const {
        auto it = m_properties.find(name);
        if (it != m_properties.end()) return &it->second;
        return nullptr;
    }

    bool PopulateImageProperties(uint32_t width, uint32_t height, uint32_t bpp,
        const std::wstring& format) {
        SetNumericProperty(L"System.Image.HorizontalSize", L"Width", width, ShellPropertyCategory::Image);
        SetNumericProperty(L"System.Image.VerticalSize", L"Height", height, ShellPropertyCategory::Image);
        SetNumericProperty(L"System.Image.BitDepth", L"Bit Depth", bpp, ShellPropertyCategory::Image);
        SetStringProperty(L"System.Image.Format", L"Format", format, ShellPropertyCategory::Image);
        SetStringProperty(L"System.Image.Dimensions", L"Dimensions",
            std::to_wstring(width) + L" x " + std::to_wstring(height), ShellPropertyCategory::Image);
        return true;
    }

    size_t Count() const { return m_properties.size(); }

    void Clear() { m_properties.clear(); }

    bool Validate() const {
        for (const auto& [name, entry] : m_properties) {
            if (entry.name.empty()) return false;
            if (entry.displayName.empty()) return false;
        }
        return true;
    }

private:
    ShellPropertyProvider() = default;
    ~ShellPropertyProvider() = default;
    ShellPropertyProvider(const ShellPropertyProvider&) = delete;
    ShellPropertyProvider& operator=(const ShellPropertyProvider&) = delete;

    std::unordered_map<std::wstring, ShellPropertyEntry> m_properties;
};

} // namespace Engine
} // namespace ExplorerLens
