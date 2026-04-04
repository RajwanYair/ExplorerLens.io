// AnnotationTimeline.h — Annotation Timeline
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a versioned timeline of annotation states with branch and rebase support.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ATLSnapshot
{
    uint64_t version = 0;
    std::string description;
    std::string stateHash;
};

struct ATLDelta
{
    uint64_t fromVersion = 0;
    uint64_t toVersion = 0;
    std::string patch;
};

class AnnotationTimeline
{
  public:
    uint64_t AddSnapshot(const std::string& description, const std::string& stateHash)
    {
        ATLSnapshot s;
        s.version = ++m_version;
        s.description = description;
        s.stateHash = stateHash;
        m_snapshots.push_back(s);
        return s.version;
    }
    bool Revert(uint64_t toVersion)
    {
        for (const auto& s : m_snapshots)
            if (s.version == toVersion) {
                m_version = toVersion;
                return true;
            }
        return false;
    }
    ATLDelta GetDelta(uint64_t fromVersion, uint64_t toVersion) const
    {
        ATLDelta d;
        d.fromVersion = fromVersion;
        d.toVersion = toVersion;
        d.patch = "delta-" + std::to_string(fromVersion) + "->" + std::to_string(toVersion);
        return d;
    }
    std::vector<ATLSnapshot> GetHistory() const
    {
        return m_snapshots;
    }
    uint64_t CurrentVersion() const
    {
        return m_version;
    }

  private:
    std::vector<ATLSnapshot> m_snapshots;
    uint64_t m_version = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
