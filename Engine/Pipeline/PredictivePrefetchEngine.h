#pragma once
// ============================================================================
// PredictivePrefetchEngine.h — Predictive File Prefetch
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE:
//   Predicts which files will be requested next based on access patterns and
//   pre-decodes their thumbnails in the background. Tracks a sliding window
//   of recent file accesses and applies three prediction strategies:
//     1. DirectoryLocality — if >=3 recent accesses are from the same
//        directory, prefetch all sibling files in that directory.
//     2. SequentialDetection — if files are accessed in alphabetical or
//        numeric order, predict the next files in sequence.
//     3. FileTypeAffinity — if the user opens files of one type (e.g. JPG),
//        predict other files of the same type in the same directory.
//
// CLASSES:
//   - PrefetchStats: Counters for predictions made, hits, misses, and
//     average prediction latency.
//   - PredictivePrefetchEngine: Full prediction engine with background
//     prefetch thread, rate limiter (10 files/sec), and directory
//     enumeration via FindFirstFileExW / FindNextFileW.
//
// INPUTS:
//   - File access notifications via RecordAccess()
//   - A prefetch callback function for StartPrefetching()
//
// OUTPUTS:
//   - Predicted file paths via PredictNext()
//   - PrefetchStats via GetStats()
//
// THREAD SAFETY:
//   All public methods are protected by std::mutex. The background prefetch
//   thread is managed internally with an atomic stop flag.
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Prefetch strategy
enum class PrefetchStrategy : uint8_t {
    None = 0,
    LinearAhead,
    BidirectionalWindow,
    AccessFrequency,
    MLPredicted,
    COUNT
};

/// Scroll direction
enum class ScrollDirection : uint8_t {
    Unknown = 0,
    Down,
    Up,
    PageDown,
    PageUp,
    JumpToEnd,
    COUNT
};

struct PrefetchConfig
{
    PrefetchStrategy strategy = PrefetchStrategy::LinearAhead;
    uint32_t windowSize = 32;
    uint32_t lookAhead = 16;
    uint32_t lookBehind = 8;
    uint64_t memoryBudgetBytes = 32u * 1024u * 1024u;
    bool cancelOnDirectionChange = true;
};

/// Prefetch statistics (predictions, hits, misses, timings).
struct PrefetchStats
{
    uint64_t prefetchHits = 0;
    uint64_t prefetchMisses = 0;
    uint64_t bytesTotalPrefetched = 0;
    float hitRate = 0.0f;
    double avgPrefetchMs = 0.0;
    uint64_t predictionsMade = 0;
    uint64_t prefetchesIssued = 0;
    double avgPredictionLatencyUs = 0.0;
};

class PredictivePrefetchEngine
{
  public:
    // ====================================================================
    // Backward-compatible static API (v14)
    // ====================================================================

    static constexpr size_t StrategyCount()
    {
        return static_cast<size_t>(PrefetchStrategy::COUNT);
    }

    static constexpr size_t DirectionCount()
    {
        return static_cast<size_t>(ScrollDirection::COUNT);
    }

    static inline const wchar_t* StrategyName(PrefetchStrategy s)
    {
        switch (s) {
            case PrefetchStrategy::None:
                return L"Disabled";
            case PrefetchStrategy::LinearAhead:
                return L"Linear Ahead";
            case PrefetchStrategy::BidirectionalWindow:
                return L"Bidirectional Window";
            case PrefetchStrategy::AccessFrequency:
                return L"Access Frequency";
            case PrefetchStrategy::MLPredicted:
                return L"ML Predicted";
            default:
                return L"Unknown";
        }
    }

    static inline const wchar_t* DirectionName(ScrollDirection d)
    {
        switch (d) {
            case ScrollDirection::Unknown:
                return L"Unknown";
            case ScrollDirection::Down:
                return L"Down";
            case ScrollDirection::Up:
                return L"Up";
            case ScrollDirection::PageDown:
                return L"Page Down";
            case ScrollDirection::PageUp:
                return L"Page Up";
            case ScrollDirection::JumpToEnd:
                return L"Jump to End";
            default:
                return L"Unknown";
        }
    }

    /// Calculate prefetch range based on viewport and scroll direction.
    static inline void CalcPrefetchRange(uint32_t viewportStart, uint32_t viewportSize, uint32_t totalItems,
                                         ScrollDirection dir, uint32_t lookAhead, uint32_t lookBehind,
                                         uint32_t& outStart, uint32_t& outEnd)
    {
        int32_t start = static_cast<int32_t>(viewportStart);
        int32_t end = static_cast<int32_t>(viewportStart + viewportSize);
        if (dir == ScrollDirection::Down || dir == ScrollDirection::PageDown) {
            end += static_cast<int32_t>(lookAhead);
            start -= static_cast<int32_t>(lookBehind);
        } else {
            start -= static_cast<int32_t>(lookAhead);
            end += static_cast<int32_t>(lookBehind);
        }
        outStart = static_cast<uint32_t>((start < 0) ? 0 : start);
        outEnd =
            static_cast<uint32_t>((end > static_cast<int32_t>(totalItems)) ? totalItems : static_cast<uint32_t>(end));
    }

    /// Calculate hit rate from hit/miss counts.
    static inline float CalcHitRate(uint64_t hits, uint64_t misses)
    {
        uint64_t total = hits + misses;
        return (total > 0) ? static_cast<float>(hits) / static_cast<float>(total) : 0.0f;
    }

    /// Construct with sliding window size (number of recent accesses tracked).
    explicit PredictivePrefetchEngine(uint32_t windowSize = 100) : m_maxHistory(windowSize), m_stopPrefetch(false) {}

    ~PredictivePrefetchEngine()
    {
        StopPrefetching();
    }

    PredictivePrefetchEngine(const PredictivePrefetchEngine&) = delete;
    PredictivePrefetchEngine& operator=(const PredictivePrefetchEngine&) = delete;

    /// Record a file access (adds to sliding window history).
    inline void RecordAccess(const std::wstring& filePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_history.push_back(filePath);
        if (m_history.size() > m_maxHistory) {
            m_history.pop_front();
        }
        m_accessedSet.insert(filePath);
    }

    /// Predict the next files likely to be requested.
    /// Applies DirectoryLocality, SequentialDetection, and FileTypeAffinity
    /// strategies in priority order, returning up to 'count' unique paths.
    inline std::vector<std::wstring> PredictNext(uint32_t count)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto start = std::chrono::steady_clock::now();
        std::vector<std::wstring> predictions;
        std::unordered_set<std::wstring> seen;

        // Copy already-accessed set for filtering
        const auto& accessed = m_accessedSet;

        // Strategy 1: Directory Locality
        // If >= 3 recent accesses in the same directory, prefetch all siblings
        std::unordered_map<std::wstring, uint32_t> dirCounts;
        for (const auto& path : m_history) {
            std::wstring dir = GetDirectory(path);
            dirCounts[dir]++;
        }
        for (const auto& pair : dirCounts) {
            if (pair.second >= 3 && predictions.size() < count) {
                auto siblings = EnumerateDirectory(pair.first);
                for (auto& sib : siblings) {
                    if (predictions.size() >= count)
                        break;
                    if (accessed.find(sib) == accessed.end() && seen.find(sib) == seen.end()) {
                        predictions.push_back(sib);
                        seen.insert(sib);
                    }
                }
            }
        }

        // Strategy 2: Sequential Detection
        // If last K files are in alphabetical order, predict next in sequence
        if (predictions.size() < count && m_history.size() >= 3) {
            bool ascending = true;
            auto it = m_history.rbegin();
            std::wstring prev = *it;
            ++it;
            uint32_t seqCount = 1;
            while (it != m_history.rend() && seqCount < 5) {
                if (*it < prev) {
                    ascending = true;
                } else {
                    ascending = false;
                    break;
                }
                prev = *it;
                ++it;
                ++seqCount;
            }

            if (ascending && seqCount >= 2) {
                std::wstring lastDir = GetDirectory(m_history.back());
                auto siblings = EnumerateDirectory(lastDir);
                std::sort(siblings.begin(), siblings.end());

                // Find position of last accessed file, predict the next ones
                auto pos = std::lower_bound(siblings.begin(), siblings.end(), m_history.back());
                if (pos != siblings.end())
                    ++pos;  // move past current
                while (pos != siblings.end() && predictions.size() < count) {
                    if (accessed.find(*pos) == accessed.end() && seen.find(*pos) == seen.end()) {
                        predictions.push_back(*pos);
                        seen.insert(*pos);
                    }
                    ++pos;
                }
            }
        }

        // Strategy 3: File Type Affinity
        // If recent accesses share the same extension, predict other files
        // with that extension in the same directory
        if (predictions.size() < count && !m_history.empty()) {
            std::unordered_map<std::wstring, uint32_t> extCounts;
            for (const auto& path : m_history) {
                std::wstring ext = GetExtension(path);
                if (!ext.empty())
                    extCounts[ext]++;
            }
            // Find most common extension
            std::wstring topExt;
            uint32_t topCount = 0;
            for (const auto& pair : extCounts) {
                if (pair.second > topCount) {
                    topCount = pair.second;
                    topExt = pair.first;
                }
            }
            if (topCount >= 2 && !topExt.empty()) {
                std::wstring lastDir = GetDirectory(m_history.back());
                auto siblings = EnumerateDirectory(lastDir);
                for (auto& sib : siblings) {
                    if (predictions.size() >= count)
                        break;
                    if (GetExtension(sib) == topExt && accessed.find(sib) == accessed.end()
                        && seen.find(sib) == seen.end()) {
                        predictions.push_back(sib);
                        seen.insert(sib);
                    }
                }
            }
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        double latUs = std::chrono::duration<double, std::micro>(elapsed).count();
        m_stats.predictionsMade++;
        // Running average of prediction latency
        m_stats.avgPredictionLatencyUs = (m_stats.avgPredictionLatencyUs * (m_stats.predictionsMade - 1) + latUs)
                                         / static_cast<double>(m_stats.predictionsMade);

        return predictions;
    }

    /// Start background prefetching. The engine calls PredictNext() periodically
    /// and invokes prefetchFn for each predicted file. Rate-limited to at most
    /// 10 files/second to avoid I/O saturation.
    inline void StartPrefetching(std::function<void(const std::wstring&)> prefetchFn)
    {
        StopPrefetching();  // stop any previous thread
        m_prefetchFn = std::move(prefetchFn);
        m_stopPrefetch.store(false, std::memory_order_release);
        m_prefetchThread = std::thread([this]() { PrefetchWorker(); });
    }

    /// Stop the background prefetch thread.
    inline void StopPrefetching()
    {
        m_stopPrefetch.store(true, std::memory_order_release);
        if (m_prefetchThread.joinable()) {
            m_prefetchThread.join();
        }
    }

    /// Mark a file as actually requested (for hit/miss tracking).
    inline void RecordRequest(const std::wstring& filePath)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_prefetchedSet.find(filePath) != m_prefetchedSet.end()) {
            m_stats.prefetchHits++;
            m_prefetchedSet.erase(filePath);
        } else {
            m_stats.prefetchMisses++;
        }
        m_stats.hitRate = CalcHitRate(m_stats.prefetchHits, m_stats.prefetchMisses);
    }

    /// Get aggregate prefetch statistics.
    inline PrefetchStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    /// Current sliding window size.
    inline size_t HistorySize() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_history.size();
    }

  private:
    /// Extract directory from full path.
    static inline std::wstring GetDirectory(const std::wstring& path)
    {
        auto pos = path.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
            return path.substr(0, pos);
        return L".";
    }

    /// Extract file extension (lowercase, including the dot).
    static inline std::wstring GetExtension(const std::wstring& path)
    {
        auto dotPos = path.find_last_of(L'.');
        auto slashPos = path.find_last_of(L"\\/");
        if (dotPos == std::wstring::npos)
            return {};
        if (slashPos != std::wstring::npos && dotPos < slashPos)
            return {};
        std::wstring ext = path.substr(dotPos);
        // Lowercase for comparison
        for (auto& ch : ext) {
            if (ch >= L'A' && ch <= L'Z')
                ch += 32;
        }
        return ext;
    }

    /// Enumerate all files in a directory using FindFirstFileExW.
    static inline std::vector<std::wstring> EnumerateDirectory(const std::wstring& dirPath)
    {
        std::vector<std::wstring> results;
        WIN32_FIND_DATAW fd;
        std::wstring pattern = dirPath + L"\\*";
        HANDLE hFind = FindFirstFileExW(pattern.c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr,
                                        FIND_FIRST_EX_LARGE_FETCH);
        if (hFind == INVALID_HANDLE_VALUE)
            return results;

        do {
            // Skip directories, ".", and ".."
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            results.push_back(dirPath + L"\\" + fd.cFileName);
        } while (FindNextFileW(hFind, &fd));

        FindClose(hFind);
        return results;
    }

    /// Background prefetch worker thread. Periodically calls PredictNext()
    /// and invokes the prefetch function with rate limiting (max 10/sec).
    inline void PrefetchWorker()
    {
        using Clock = std::chrono::steady_clock;
        const auto minInterval = std::chrono::milliseconds(100);  // 10 files/sec

        while (!m_stopPrefetch.load(std::memory_order_acquire)) {
            std::vector<std::wstring> predictions;
            {
                // Predict next 5 files
                // Note: PredictNext acquires m_mutex internally
                // We must NOT hold m_mutex here
            }
            predictions = PredictNext(5);

            for (const auto& path : predictions) {
                if (m_stopPrefetch.load(std::memory_order_acquire))
                    break;

                auto beforePrefetch = Clock::now();

                if (m_prefetchFn) {
                    m_prefetchFn(path);
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_stats.prefetchesIssued++;
                    m_prefetchedSet.insert(path);
                }

                // Rate limit: wait at least 100ms between prefetches
                auto elapsed = Clock::now() - beforePrefetch;
                if (elapsed < minInterval) {
                    std::this_thread::sleep_for(minInterval - elapsed);
                }
            }

            // If no predictions, sleep longer to avoid busy-looping
            if (predictions.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }

    // Sliding window of recent file accesses
    uint32_t m_maxHistory;
    std::deque<std::wstring> m_history;
    std::unordered_set<std::wstring> m_accessedSet;

    // Prefetch tracking for hit/miss calculation
    std::unordered_set<std::wstring> m_prefetchedSet;

    // Stats and configuration
    mutable std::mutex m_mutex;
    PrefetchStats m_stats{};

    // Background prefetch thread
    std::atomic<bool> m_stopPrefetch;
    std::thread m_prefetchThread;
    std::function<void(const std::wstring&)> m_prefetchFn;
};

}  // namespace Engine
}  // namespace ExplorerLens
