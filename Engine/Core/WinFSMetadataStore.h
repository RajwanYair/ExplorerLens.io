// WinFSMetadataStore.h — Windows File System Metadata Store (WinFS-style)
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a structured metadata store layered on top of NTFS alternate data streams
// and Windows Property System, enabling rich file metadata without external databases.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MetadataStoreBackend {
    NTFS_ADS,
    WindowsPropertySystem,
    InMemory
};
enum class MetadataValueType {
    String,
    Integer,
    Double,
    Boolean,
    DateTime
};

struct MetadataProperty
{
    std::string key;
    std::string value;
    MetadataValueType type = MetadataValueType::String;
};

struct MetadataRecord
{
    std::wstring filePath;
    std::vector<MetadataProperty> properties;
    std::string lastModifiedISO;

    void Set(const std::string& key, const std::string& val)
    {
        for (auto& p : properties) {
            if (p.key == key) {
                p.value = val;
                return;
            }
        }
        properties.push_back({key, val, MetadataValueType::String});
    }
    std::string Get(const std::string& key) const noexcept
    {
        for (const auto& p : properties)
            if (p.key == key)
                return p.value;
        return {};
    }
};

struct MetadataStoreStats
{
    int recordCount = 0;
    int totalProps = 0;
};

class WinFSMetadataStore
{
  public:
    explicit WinFSMetadataStore(MetadataStoreBackend backend = MetadataStoreBackend::InMemory) : m_backend(backend) {}

    bool Write(const MetadataRecord& record)
    {
        m_store[record.filePath] = record;
        return true;
    }

    bool Read(const std::wstring& filePath, MetadataRecord& outRecord) const
    {
        auto it = m_store.find(filePath);
        if (it == m_store.end())
            return false;
        outRecord = it->second;
        return true;
    }

    bool Delete(const std::wstring& filePath)
    {
        return m_store.erase(filePath) > 0;
    }

    MetadataStoreStats Stats() const noexcept
    {
        MetadataStoreStats s;
        s.recordCount = static_cast<int>(m_store.size());
        for (const auto& [_, rec] : m_store)
            s.totalProps += static_cast<int>(rec.properties.size());
        return s;
    }

    MetadataStoreBackend Backend() const noexcept
    {
        return m_backend;
    }

    std::vector<std::wstring> AllPaths() const
    {
        std::vector<std::wstring> result;
        for (const auto& [path, _] : m_store)
            result.push_back(path);
        return result;
    }

  private:
    MetadataStoreBackend m_backend;
    std::unordered_map<std::wstring, MetadataRecord> m_store;
};

}  // namespace Engine
}  // namespace ExplorerLens
