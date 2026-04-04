// SharedCollectionBuilder.h — Shared Collection Builder (Folder-Level Annotation Sets)
// Copyright (c) 2026 ExplorerLens Project
//
// Groups file annotations into named collections scoped to a folder. Collections
// can be exported, shared via URL, or merged from remote collaborators.
//
#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CollectionItem
{
    std::wstring filePath;
    std::wstring thumbnailKey;
    std::vector<std::wstring> tags;
    int starRating = 0;
};

struct SharedCollection
{
    std::wstring name;
    std::wstring folderPath;
    std::string shareToken;
    std::vector<CollectionItem> items;
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();
    std::string createdBy;
    int version = 1;
    bool isShared = false;

    int ItemCount() const noexcept
    {
        return (int)items.size();
    }
    bool IsEmpty() const noexcept
    {
        return items.empty();
    }
};

class SharedCollectionBuilder
{
  public:
    explicit SharedCollectionBuilder() = default;

    SharedCollection& CreateCollection(const std::wstring& name, const std::wstring& folder)
    {
        SharedCollection col;
        col.name = name;
        col.folderPath = folder;
        col.shareToken = GenerateToken(name);
        m_collections.push_back(col);
        return m_collections.back();
    }

    bool AddItem(const std::wstring& collectionName, const CollectionItem& item)
    {
        for (auto& col : m_collections)
            if (col.name == collectionName) {
                col.items.push_back(item);
                return true;
            }
        return false;
    }

    const SharedCollection* FindByToken(const std::string& token) const
    {
        for (const auto& col : m_collections)
            if (col.shareToken == token)
                return &col;
        return nullptr;
    }

    bool MergeCollection(const SharedCollection& remote)
    {
        for (auto& local : m_collections) {
            if (local.name == remote.name && local.folderPath == remote.folderPath) {
                for (const auto& item : remote.items)
                    local.items.push_back(item);
                local.version = std::max(local.version, remote.version) + 1;
                return true;
            }
        }
        m_collections.push_back(remote);
        return true;
    }

    int CollectionCount() const noexcept
    {
        return (int)m_collections.size();
    }
    const std::vector<SharedCollection>& All() const noexcept
    {
        return m_collections;
    }

  private:
    static std::string GenerateToken(const std::wstring& name)
    {
        return "tok-" + std::to_string(std::hash<std::wstring>{}(name) % 100000);
    }
    std::vector<SharedCollection> m_collections;
};

}  // namespace Engine
}  // namespace ExplorerLens
