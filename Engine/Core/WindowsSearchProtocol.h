#pragma once
// WindowsSearchProtocol.h — Windows Search Protocol Handler
// IFilter + ISearchProtocol implementation for Windows Desktop Search
// integration, enabling thumbnail metadata to appear in Windows Search results.
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Search index field
enum class SearchIndexField : uint8_t {
 FileName = 0,
 FileExtension,
 Dimensions,
 ColorSpace_,
 CameraModel,
 DateTaken,
 GPSLocation,
 FileSize,
 FormatFamily_,
 ThumbnailHash,
 COUNT
};

/// Search protocol state
enum class SearchProtocolState : uint8_t {
 Unregistered = 0,
 Registered,
 Indexing,
 Idle,
 Error,
 COUNT
};

struct SearchIndexStats {
 uint64_t filesIndexed = 0;
 uint64_t fieldsStored = 0;
 double indexBuildTimeMs = 0.0;
 uint64_t indexSizeBytes = 0;
 SearchProtocolState state = SearchProtocolState::Unregistered;
};

class WindowsSearchProtocol {
public:
 static constexpr size_t FieldCount() {
 return static_cast<size_t>(SearchIndexField::COUNT);
 }
 static constexpr size_t StateCount() {
 return static_cast<size_t>(SearchProtocolState::COUNT);
 }

 static const wchar_t *FieldName(SearchIndexField f) {
 switch (f) {
 case SearchIndexField::FileName:
 return L"File Name";
 case SearchIndexField::FileExtension:
 return L"File Extension";
 case SearchIndexField::Dimensions:
 return L"Dimensions";
 case SearchIndexField::ColorSpace_:
 return L"Color Space";
 case SearchIndexField::CameraModel:
 return L"Camera Model";
 case SearchIndexField::DateTaken:
 return L"Date Taken";
 case SearchIndexField::GPSLocation:
 return L"GPS Location";
 case SearchIndexField::FileSize:
 return L"File Size";
 case SearchIndexField::FormatFamily_:
 return L"Format Family";
 case SearchIndexField::ThumbnailHash:
 return L"Thumbnail Hash";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *StateName(SearchProtocolState s) {
 switch (s) {
 case SearchProtocolState::Unregistered:
 return L"Unregistered";
 case SearchProtocolState::Registered:
 return L"Registered";
 case SearchProtocolState::Indexing:
 return L"Indexing";
 case SearchProtocolState::Idle:
 return L"Idle";
 case SearchProtocolState::Error:
 return L"Error";
 default:
 return L"Unknown";
 }
 }
};

// Shell search protocol handler state
enum class SearchHandlerState : uint8_t {
    Uninitialized = 0,
    Ready,
    Error,
    COUNT
};

// An entry in the shell search index
struct SearchIndexEntry {
    std::wstring filePath;
    std::wstring displayName;
    std::wstring formatType;
    uint64_t     fileSize    = 0;
    bool         hasThumbnail = false;
};

// A shell search result with file entry and relevance score
struct ShellSearchHit {
    SearchIndexEntry entry;
    float            relevanceScore = 0.0f;
};

// Aggregated search results from ShellSearchProtocolHandler::Search
struct ShellSearchResults {
    uint32_t                  totalMatches = 0;
    std::vector<ShellSearchHit> results;
};

// Query parameters for ShellSearchProtocolHandler::Search
struct ShellSearchQuery {
    std::wstring queryText;
    std::wstring extensionFilter;
};

// Alias for backward compat — results-only alias avoids conflict with MarketplaceClientV4::SearchQuery
using SearchResults = ShellSearchResults;

// Full-text and filtered search handler over the dynamic thumbnail index
class ShellSearchProtocolHandler {
public:
    ShellSearchProtocolHandler() = default;

    SearchHandlerState GetState() const noexcept { return m_state; }

    bool Initialize() noexcept {
        m_state = SearchHandlerState::Ready;
        return true;
    }

    void Shutdown() noexcept {
        m_state = SearchHandlerState::Uninitialized;
        m_entries.clear();
    }

    uint32_t GetIndexSize() const noexcept {
        return static_cast<uint32_t>(m_entries.size());
    }

    void AddEntry(SearchIndexEntry const& entry) {
        m_entries.push_back(entry);
    }

    void ClearIndex() noexcept { m_entries.clear(); }

    SearchResults Search(ShellSearchQuery const& query) const {
        SearchResults out;
        for (auto const& e : m_entries) {
            bool matches = e.displayName.find(query.queryText) != std::wstring::npos;
            if (matches && !query.extensionFilter.empty()) {
                auto dot = e.displayName.rfind(L'.');
                std::wstring ext = (dot != std::wstring::npos)
                    ? e.displayName.substr(dot) : std::wstring{};
                matches = (ext == query.extensionFilter);
            }
            if (matches) {
                ShellSearchHit r;
                r.entry          = e;
                r.relevanceScore = 1.0f;
                out.results.push_back(std::move(r));
                ++out.totalMatches;
            }
        }
        return out;
    }

private:
    SearchHandlerState           m_state = SearchHandlerState::Uninitialized;
    std::vector<SearchIndexEntry> m_entries;
};

} // namespace Engine
} // namespace ExplorerLens
