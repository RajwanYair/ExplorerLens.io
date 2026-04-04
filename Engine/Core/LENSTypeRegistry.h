// LENSTypeRegistry.h — Centralized LENSTYPE Registry
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a unified registry for all supported LENSTYPE entries,
// enabling dynamic format registration, lookup, and enumeration.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LENSRegistryCategory : uint8_t {
    Archive = 0,
    Image = 1,
    Document = 2,
    Video = 3,
    Audio = 4,
    CAD = 5,
    Scientific = 6,
    Font = 7,
    Executable = 8,
    Unknown = 255
};

struct TypeRegistryEntry
{
    uint32_t typeId = 0;
    std::string extension;
    std::string mimeType;
    std::string description;
    LENSRegistryCategory category = LENSRegistryCategory::Unknown;
    bool gpuAccel = false;
    bool pluginBased = false;
    uint32_t priority = 100;
};

class LENSTypeRegistry
{
  public:
    static LENSTypeRegistry& Instance()
    {
        static LENSTypeRegistry s;
        return s;
    }

    bool Register(const TypeRegistryEntry& entry)
    {
        if (entry.extension.empty())
            return false;
        if (m_byId.count(entry.typeId) > 0)
            return false;
        m_entries.push_back(entry);
        size_t idx = m_entries.size() - 1;
        m_byId[entry.typeId] = idx;
        std::string extLower = ToLower(entry.extension);
        m_byExtension[extLower] = idx;
        if (!entry.mimeType.empty()) {
            m_byMime[entry.mimeType] = idx;
        }
        return true;
    }

    const TypeRegistryEntry* Lookup(uint32_t typeId) const
    {
        auto it = m_byId.find(typeId);
        if (it == m_byId.end())
            return nullptr;
        return &m_entries[it->second];
    }

    const TypeRegistryEntry* LookupByExtension(const std::string& ext) const
    {
        auto it = m_byExtension.find(ToLower(ext));
        if (it == m_byExtension.end())
            return nullptr;
        return &m_entries[it->second];
    }

    const TypeRegistryEntry* LookupByMime(const std::string& mime) const
    {
        auto it = m_byMime.find(mime);
        if (it == m_byMime.end())
            return nullptr;
        return &m_entries[it->second];
    }

    std::vector<const TypeRegistryEntry*> GetAllTypes() const
    {
        std::vector<const TypeRegistryEntry*> result;
        result.reserve(m_entries.size());
        for (const auto& e : m_entries)
            result.push_back(&e);
        return result;
    }

    std::vector<const TypeRegistryEntry*> GetByCategory(LENSRegistryCategory cat) const
    {
        std::vector<const TypeRegistryEntry*> result;
        for (const auto& e : m_entries) {
            if (e.category == cat)
                result.push_back(&e);
        }
        return result;
    }

    size_t Count() const
    {
        return m_entries.size();
    }

    void Clear()
    {
        m_entries.clear();
        m_byId.clear();
        m_byExtension.clear();
        m_byMime.clear();
    }

    bool Contains(uint32_t typeId) const
    {
        return m_byId.count(typeId) > 0;
    }

    bool Validate() const
    {
        if (m_entries.size() != m_byId.size())
            return false;
        for (const auto& entry : m_entries) {
            if (entry.extension.empty())
                return false;
            if (entry.category == LENSRegistryCategory::Unknown && entry.typeId != 0) {
                // Type with non-zero ID should have a category
            }
        }
        // Verify no duplicate IDs
        std::unordered_map<uint32_t, int> idCount;
        for (const auto& e : m_entries) {
            if (++idCount[e.typeId] > 1)
                return false;
        }
        return true;
    }

  private:
    LENSTypeRegistry() = default;
    ~LENSTypeRegistry() = default;
    LENSTypeRegistry(const LENSTypeRegistry&) = delete;
    LENSTypeRegistry& operator=(const LENSTypeRegistry&) = delete;

    static std::string ToLower(const std::string& s)
    {
        std::string r = s;
        std::transform(r.begin(), r.end(), r.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return r;
    }

    std::vector<TypeRegistryEntry> m_entries;
    std::unordered_map<uint32_t, size_t> m_byId;
    std::unordered_map<std::string, size_t> m_byExtension;
    std::unordered_map<std::string, size_t> m_byMime;
};

}  // namespace Engine
}  // namespace ExplorerLens
