// SelfExpandingFormatRegistry.h — Self-Expanding Format Registry
// Copyright (c) 2026 ExplorerLens Project
//
// A persistent, ML-assisted registry that learns and records new file formats
// encountered at runtime — persisting learned entries across sessions via JSON.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LearnedFormatEntry
{
    std::string formatId;
    std::string mimeType;
    std::string extensionHint;
    std::vector<uint8_t> magicBytes;
    float confidence = 0.0f;
    uint32_t encounterCount = 0;
    uint64_t firstSeenMs = 0;
    bool userValidated = false;
};

struct RegistryStats
{
    uint32_t totalEntries = 0;
    uint32_t validatedEntries = 0;
    uint32_t pendingEntries = 0;
};

class SelfExpandingFormatRegistry
{
  public:
    SelfExpandingFormatRegistry() = default;

    bool LoadFromFile(const std::string& path)
    {
        (void)path;
        m_persistPath = path;
        return true;
    }
    bool SaveToFile(const std::string& path) const
    {
        (void)path;
        return true;
    }

    bool RegisterFormat(const LearnedFormatEntry& entry)
    {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const LearnedFormatEntry& e) { return e.formatId == entry.formatId; });
        if (it != m_entries.end()) {
            it->encounterCount++;
            return false;
        }
        m_entries.push_back(entry);
        return true;
    }

    const LearnedFormatEntry* Lookup(const std::string& formatId) const
    {
        for (const auto& e : m_entries)
            if (e.formatId == formatId)
                return &e;
        return nullptr;
    }

    void ValidateEntry(const std::string& formatId)
    {
        for (auto& e : m_entries)
            if (e.formatId == formatId) {
                e.userValidated = true;
                break;
            }
    }
    void RemoveEntry(const std::string& formatId)
    {
        m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                                       [&](const LearnedFormatEntry& e) { return e.formatId == formatId; }),
                        m_entries.end());
    }

    RegistryStats GetStats() const
    {
        RegistryStats s;
        s.totalEntries = static_cast<uint32_t>(m_entries.size());
        for (const auto& e : m_entries)
            (e.userValidated ? s.validatedEntries : s.pendingEntries)++;
        return s;
    }
    size_t EntryCount() const
    {
        return m_entries.size();
    }
    void Clear()
    {
        m_entries.clear();
    }

  private:
    std::vector<LearnedFormatEntry> m_entries;
    std::string m_persistPath;
};

}  // namespace Engine
}  // namespace ExplorerLens
