// ShellSearchProtocolHandler.h — ISearchProtocolHandler Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Implements Windows Search protocol handler for indexed thumbnail search.
// Enables type-ahead search of archive contents via Windows Desktop Search.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Search index entry — metadata for a single indexed item
// ============================================================================

struct SearchIndexEntry {
    std::wstring filePath;            // Full path to archive/file
    std::wstring displayName;         // User-visible name
    std::wstring archiveEntryPath;    // Path within archive (empty for raw files)
    std::wstring formatType;          // e.g., L"ZIP", L"RAR", L"7Z", L"RAW"
    uint64_t     fileSize      = 0;   // Original file size in bytes
    uint64_t     compressedSize = 0;  // Compressed size (for archives)
    uint32_t     width         = 0;   // Image width (if known)
    uint32_t     height        = 0;   // Image height (if known)
    uint64_t     modifiedTime  = 0;   // FILETIME as uint64
    uint32_t     entryCount    = 0;   // Number of items in archive
    bool         hasThumbnail  = false;
};

// ============================================================================
// Search query parameters
// ============================================================================

struct SearchQuery {
    std::wstring queryText;           // User's search string
    std::wstring extensionFilter;     // e.g., L".zip" (empty = all)
    std::wstring formatFilter;        // e.g., L"RAW" (empty = all)
    uint32_t     maxResults    = 50;  // Limit results
    bool         includeArchiveContents = true;
    bool         caseSensitive = false;

    enum class SortBy : uint8_t {
        Relevance,
        FileName,
        FileSize,
        ModifiedDate,
        Format
    };
    SortBy sortBy = SortBy::Relevance;
    bool   sortDescending = false;
};

// ============================================================================
// Search result set
// ============================================================================

struct SearchResult {
    SearchIndexEntry entry;
    float            relevanceScore = 0.0f;  // 0.0 .. 1.0
    std::wstring     highlightedName;         // Name with match highlights
};

struct SearchResultSet {
    std::vector<SearchResult> results;
    uint32_t totalMatches = 0;       // Total matches (may exceed results.size())
    double   searchTimeMs = 0.0;     // Query execution time
    bool     truncated    = false;   // More results available
};

// ============================================================================
// Protocol handler state
// ============================================================================

enum class SearchHandlerState : uint8_t {
    Uninitialized,
    Initializing,
    Ready,
    Indexing,
    Searching,
    Error
};

// ============================================================================
// ShellSearchProtocolHandler — search indexer & query engine
// ============================================================================

class ShellSearchProtocolHandler {
public:
    /// Protocol name registered with Windows Search
    static constexpr const wchar_t* PROTOCOL_NAME = L"explorlens";

    /// Maximum index entries before automatic compaction
    static constexpr uint32_t MAX_INDEX_SIZE = 100000;

    /// Indexing batch size
    static constexpr uint32_t INDEX_BATCH_SIZE = 256;

    ShellSearchProtocolHandler() = default;
    ~ShellSearchProtocolHandler() = default;

    // Non-copyable
    ShellSearchProtocolHandler(const ShellSearchProtocolHandler&) = delete;
    ShellSearchProtocolHandler& operator=(const ShellSearchProtocolHandler&) = delete;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    bool Initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = SearchHandlerState::Initializing;
        m_index.clear();
        m_extensionIndex.clear();
        m_state = SearchHandlerState::Ready;
        return true;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_index.clear();
        m_extensionIndex.clear();
        m_state = SearchHandlerState::Uninitialized;
    }

    // ========================================================================
    // Indexing
    // ========================================================================

    /// Add a single entry to the search index
    bool AddEntry(const SearchIndexEntry& entry) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_index.size() >= MAX_INDEX_SIZE) {
            CompactIndexLocked();
        }

        m_index.push_back(entry);
        uint32_t idx = static_cast<uint32_t>(m_index.size() - 1);

        // Build extension reverse index
        std::wstring ext = ExtractExtension(entry.filePath);
        if (!ext.empty()) {
            m_extensionIndex[ext].push_back(idx);
        }

        m_totalIndexed.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    /// Add multiple entries in batch
    uint32_t AddEntries(const std::vector<SearchIndexEntry>& entries) {
        uint32_t added = 0;
        for (const auto& entry : entries) {
            if (AddEntry(entry)) added++;
        }
        return added;
    }

    /// Remove entries matching a file path prefix
    uint32_t RemoveByPath(const std::wstring& pathPrefix) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t removed = 0;

        auto it = m_index.begin();
        while (it != m_index.end()) {
            if (it->filePath.find(pathPrefix) == 0) {
                it = m_index.erase(it);
                removed++;
            } else {
                ++it;
            }
        }

        if (removed > 0) {
            RebuildExtensionIndexLocked();
        }
        return removed;
    }

    /// Clear the entire index
    void ClearIndex() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_index.clear();
        m_extensionIndex.clear();
        m_totalIndexed.store(0, std::memory_order_relaxed);
    }

    // ========================================================================
    // Search / Query
    // ========================================================================

    /// Execute a search query
    SearchResultSet Search(const SearchQuery& query) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto startTime = std::chrono::steady_clock::now();

        SearchResultSet results;
        m_state = SearchHandlerState::Searching;

        // Get candidate set (extension filter narrows scope)
        const auto& candidates = GetCandidateSetLocked(query);

        for (uint32_t idx : candidates) {
            if (idx >= m_index.size()) continue;
            const auto& entry = m_index[idx];

            // Apply format filter
            if (!query.formatFilter.empty() && entry.formatType != query.formatFilter) {
                continue;
            }

            // Compute relevance score
            float score = ComputeRelevance(entry, query);
            if (score > 0.0f) {
                results.totalMatches++;

                if (results.results.size() < query.maxResults) {
                    SearchResult result;
                    result.entry = entry;
                    result.relevanceScore = score;
                    result.highlightedName = HighlightMatches(entry.displayName, query.queryText);
                    results.results.push_back(std::move(result));
                } else {
                    results.truncated = true;
                }
            }
        }

        // Sort results
        SortResults(results.results, query.sortBy, query.sortDescending);

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        results.searchTimeMs = std::chrono::duration<double, std::milli>(elapsed).count();

        m_state = SearchHandlerState::Ready;
        m_totalSearches.fetch_add(1, std::memory_order_relaxed);
        return results;
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    uint32_t GetIndexSize() const {
        return static_cast<uint32_t>(m_index.size());
    }

    uint64_t GetTotalIndexed() const {
        return m_totalIndexed.load(std::memory_order_relaxed);
    }

    uint64_t GetTotalSearches() const {
        return m_totalSearches.load(std::memory_order_relaxed);
    }

    SearchHandlerState GetState() const { return m_state; }

private:
    // ========================================================================
    // Internal helpers
    // ========================================================================

    std::wstring ExtractExtension(const std::wstring& path) const {
        size_t dot = path.rfind(L'.');
        if (dot == std::wstring::npos) return L"";
        std::wstring ext = path.substr(dot);
        // Lowercase
        for (auto& ch : ext) {
            if (ch >= L'A' && ch <= L'Z') ch += (L'a' - L'A');
        }
        return ext;
    }

    const std::vector<uint32_t>& GetCandidateSetLocked(const SearchQuery& query) {
        if (!query.extensionFilter.empty()) {
            auto it = m_extensionIndex.find(query.extensionFilter);
            if (it != m_extensionIndex.end()) {
                return it->second;
            }
            static const std::vector<uint32_t> empty;
            return empty;
        }

        // No filter — return all indices
        m_allIndices.resize(m_index.size());
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_index.size()); i++) {
            m_allIndices[i] = i;
        }
        return m_allIndices;
    }

    float ComputeRelevance(const SearchIndexEntry& entry, const SearchQuery& query) const {
        if (query.queryText.empty()) return 1.0f;  // Match all

        const std::wstring& text = query.caseSensitive ? entry.displayName : ToLower(entry.displayName);
        const std::wstring& queryLower = query.caseSensitive ? query.queryText : ToLower(query.queryText);

        // Exact match
        if (text == queryLower) return 1.0f;

        // Prefix match
        if (text.find(queryLower) == 0) return 0.9f;

        // Substring match
        if (text.find(queryLower) != std::wstring::npos) return 0.7f;

        // Check archive entry path too
        if (entry.archiveEntryPath.find(queryLower) != std::wstring::npos) return 0.5f;

        return 0.0f;  // No match
    }

    std::wstring ToLower(const std::wstring& s) const {
        std::wstring result = s;
        for (auto& ch : result) {
            if (ch >= L'A' && ch <= L'Z') ch += (L'a' - L'A');
        }
        return result;
    }

    std::wstring HighlightMatches(const std::wstring& text, const std::wstring& query) const {
        // Simple highlight — wrap matches in brackets for now
        // In production, this would use rich text or HTML tags
        if (query.empty()) return text;

        std::wstring lower = ToLower(text);
        std::wstring qLower = ToLower(query);
        size_t pos = lower.find(qLower);
        if (pos == std::wstring::npos) return text;

        std::wstring result = text.substr(0, pos) + L"[" +
            text.substr(pos, query.size()) + L"]" +
            text.substr(pos + query.size());
        return result;
    }

    static void SortResults(std::vector<SearchResult>& results,
        SearchQuery::SortBy sortBy, bool descending)
    {
        auto cmp = [sortBy, descending](const SearchResult& a, const SearchResult& b) {
            int order = 0;
            switch (sortBy) {
                case SearchQuery::SortBy::Relevance:
                    order = (a.relevanceScore > b.relevanceScore) ? -1 : 1;
                    break;
                case SearchQuery::SortBy::FileName:
                    order = a.entry.displayName.compare(b.entry.displayName);
                    break;
                case SearchQuery::SortBy::FileSize:
                    order = (a.entry.fileSize < b.entry.fileSize) ? -1 : 1;
                    break;
                case SearchQuery::SortBy::ModifiedDate:
                    order = (a.entry.modifiedTime < b.entry.modifiedTime) ? -1 : 1;
                    break;
                case SearchQuery::SortBy::Format:
                    order = a.entry.formatType.compare(b.entry.formatType);
                    break;
            }
            return descending ? (order > 0) : (order < 0);
        };
        std::sort(results.begin(), results.end(), cmp);
    }

    void CompactIndexLocked() {
        // Remove entries without thumbnails (lowest value)
        auto it = m_index.begin();
        while (it != m_index.end() && m_index.size() > MAX_INDEX_SIZE / 2) {
            if (!it->hasThumbnail) {
                it = m_index.erase(it);
            } else {
                ++it;
            }
        }
        RebuildExtensionIndexLocked();
    }

    void RebuildExtensionIndexLocked() {
        m_extensionIndex.clear();
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_index.size()); i++) {
            std::wstring ext = ExtractExtension(m_index[i].filePath);
            if (!ext.empty()) {
                m_extensionIndex[ext].push_back(i);
            }
        }
    }

    // Index storage
    std::vector<SearchIndexEntry> m_index;
    std::unordered_map<std::wstring, std::vector<uint32_t>> m_extensionIndex;
    std::vector<uint32_t> m_allIndices;  // Scratch buffer for unfiltered queries

    // State
    std::mutex m_mutex;
    SearchHandlerState m_state = SearchHandlerState::Uninitialized;
    std::atomic<uint64_t> m_totalIndexed{0};
    std::atomic<uint64_t> m_totalSearches{0};
};

} // namespace Engine
} // namespace ExplorerLens
