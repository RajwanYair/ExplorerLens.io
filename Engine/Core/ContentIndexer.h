#pragma once
// =============================================================================
// ContentIndexer.h — File Content Indexing for Search
// ExplorerLens Engine — Core Module
// =============================================================================

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace ExplorerLens {

/// Content type classification
enum class ContentType : uint32_t {
 Image = 0,
 Archive = 1,
 Document = 2,
 Video = 3,
 Audio = 4,
 Model3D = 5,
 Font = 6,
 Unknown = 7,
 Count = 8
};

/// Index entry state
enum class IndexState : uint32_t {
 Pending = 0, ///< Queued for indexing
 Indexed = 1, ///< Successfully indexed
 Failed = 2, ///< Indexing failed
 Stale = 3, ///< File modified since last index
 Removed = 4 ///< File no longer exists
};

/// A single indexed file entry
struct ContentIndexEntry {
 uint64_t id = 0;
 std::wstring filePath;
 std::wstring fileName;
 std::wstring extension;
 ContentType contentType = ContentType::Unknown;
 IndexState state = IndexState::Pending;
 uint64_t fileSize = 0;
 uint64_t modifiedAt = 0;
 uint64_t indexedAt = 0;
 uint32_t width = 0; ///< For images
 uint32_t height = 0; ///< For images
};

/// Index statistics
struct ContentIndexStats {
 uint32_t totalEntries = 0;
 uint32_t indexedCount = 0;
 uint32_t pendingCount = 0;
 uint32_t failedCount = 0;
 uint32_t staleCount = 0;
 uint64_t totalSizeBytes = 0;
 uint32_t contentTypeCounts[static_cast<uint32_t>(ContentType::Count)] = {};
};

/// ContentIndexer — indexes files for fast content-based search
class ContentIndexer {
public:
 ContentIndexer();

 // Indexing
 uint64_t AddFile(const std::wstring& filePath);
 bool RemoveFile(uint64_t entryId);
 bool UpdateFile(uint64_t entryId);
 uint32_t IndexAll();

 // Search
 std::vector<ContentIndexEntry> SearchByName(const std::wstring& pattern) const;
 std::vector<ContentIndexEntry> SearchByType(ContentType type) const;
 std::vector<ContentIndexEntry> SearchByExtension(const std::wstring& ext) const;

 // Queries
 const ContentIndexEntry* GetEntry(uint64_t id) const;
 ContentIndexStats GetStats() const;
 uint32_t GetTotalCount() const { return static_cast<uint32_t>(m_entries.size()); }

 // Maintenance
 uint32_t PurgeRemoved();
 void Clear();

 // Static
 static ContentType ClassifyExtension(const std::wstring& ext);
 static const wchar_t* GetContentTypeName(ContentType type);
 static const wchar_t* GetIndexStateName(IndexState state);
 static constexpr uint32_t GetContentTypeCount() { return static_cast<uint32_t>(ContentType::Count); }

private:
 std::vector<ContentIndexEntry> m_entries;
 uint64_t m_nextId = 1;

 std::wstring ExtractExtension(const std::wstring& path) const;
 std::wstring ExtractFileName(const std::wstring& path) const;
};

} // namespace ExplorerLens

