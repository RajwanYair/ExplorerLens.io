// FileTypeAssociationBroker.h — System File Association Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages Windows file type associations for ExplorerLens, handling
// registration, conflict detection, and user-choice persistence.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AssociationState : uint8_t {
    NotRegistered,
    Registered,
    ConflictDetected,
    UserOverridden,
    SystemDefault
};

struct FileTypeAssociation
{
    std::wstring extension;
    std::wstring progId;
    std::wstring currentHandler;
    AssociationState state = AssociationState::NotRegistered;
    bool isExplorerLensOwned = false;
};

struct TypeAssociationConflict
{
    std::wstring extension;
    std::wstring competingHandler;
    std::wstring competingAppName;
    uint64_t detectedTimestamp = 0;
};

class FileTypeAssociationBroker
{
  public:
    FileTypeAssociationBroker() = default;

    void RegisterExtension(const std::wstring& extension, const std::wstring& progId)
    {
        FileTypeAssociation assoc;
        assoc.extension = extension;
        assoc.progId = progId;
        assoc.state = AssociationState::Registered;
        assoc.isExplorerLensOwned = true;
        m_associations[extension] = assoc;
    }

    AssociationState GetState(const std::wstring& extension) const
    {
        auto it = m_associations.find(extension);
        if (it != m_associations.end())
            return it->second.state;
        return AssociationState::NotRegistered;
    }

    std::vector<FileTypeAssociation> GetAllAssociations() const
    {
        std::vector<FileTypeAssociation> result;
        result.reserve(m_associations.size());
        for (const auto& [ext, assoc] : m_associations) {
            result.push_back(assoc);
        }
        return result;
    }

    std::vector<TypeAssociationConflict> DetectConflicts() const
    {
        std::vector<TypeAssociationConflict> conflicts;
        for (const auto& [ext, assoc] : m_associations) {
            if (assoc.state == AssociationState::ConflictDetected) {
                conflicts.push_back(TypeAssociationConflict{ext, assoc.currentHandler, L"", 0});
            }
        }
        return conflicts;
    }

    size_t GetRegisteredCount() const
    {
        return m_associations.size();
    }

    void UnregisterExtension(const std::wstring& extension)
    {
        m_associations.erase(extension);
    }

    void Clear()
    {
        m_associations.clear();
    }

  private:
    std::unordered_map<std::wstring, FileTypeAssociation> m_associations;
};

}  // namespace Engine
}  // namespace ExplorerLens
