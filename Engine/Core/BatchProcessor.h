#pragma once
/**
 * @file BatchProcessor.h
 * @brief Sprint 573 — Processes multiple thumbnail requests in batch for maximum throughput.
 *
 * Purpose:
 *   Accepts batches of thumbnail decode requests, sorts them by directory for I/O
 *   locality, dispatches them across a configurable thread pool, and collects results.
 *   Supports progress tracking, cancellation, rate limiting, and progress callbacks.
 *
 * Classes:
 *   - BatchProcessor: Batch thumbnail processing engine.
 *
 * Key types:
 *   - BatchItem: Input descriptor (file path, target dimensions).
 *   - BatchResult: Output per item (success, RGBA data, actual dimensions, timing, error).
 *   - BatchStats: Aggregate throughput, timing, and failure rate statistics.
 *
 * Inputs:
 *   - Vectors of BatchItem, a decode function, concurrency/rate settings.
 * Outputs:
 *   - Vectors of BatchResult per batch, progress (0.0–1.0), statistics.
 *
 * Thread safety:
 *   All public methods are thread-safe. Protected by SRWLOCK.
 *
 * Dependencies: Windows API + C++ standard library only.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

    struct BatchItem {
        std::wstring filePath;
        uint32_t targetWidth = 256;
        uint32_t targetHeight = 256;
    };

    struct BatchResult {
        std::wstring filePath;
        bool success = false;
        std::vector<uint8_t> rgbaData;
        uint32_t actualWidth = 0;
        uint32_t actualHeight = 0;
        uint32_t decodeTimeUs = 0;
        std::string error;
    };

    struct BatchStats {
        uint64_t batchesCompleted = 0;
        uint64_t itemsProcessed = 0;
        double avgItemsPerSecond = 0.0;
        double avgDecodeTimeUs = 0.0;
        double failureRate = 0.0;
        uint64_t totalDecodeTimeUs = 0;
        uint64_t totalSuccesses = 0;
        uint64_t totalFailures = 0;
    };

    class BatchProcessor {
    public:
        inline BatchProcessor() noexcept {
            InitializeSRWLock(&m_lock);
            m_nextBatchId.store(1);
            m_maxConcurrency = (std::max)(std::thread::hardware_concurrency(), 1u);
            m_maxThroughput = 0; // unlimited
            m_running.store(false);
        }

        inline ~BatchProcessor() noexcept {
            StopInternal();
        }

        BatchProcessor(const BatchProcessor&) = delete;
        BatchProcessor& operator=(const BatchProcessor&) = delete;

        /// Register the decode function that processes individual items.
        inline void SetDecodeFunction(std::function<BatchResult(const BatchItem&)> fn) {
            AcquireSRWLockExclusive(&m_lock);
            m_decodeFn = std::move(fn);
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Set maximum concurrency (number of worker threads). Default: hardware_concurrency.
        inline void SetMaxConcurrency(uint32_t threads) {
            AcquireSRWLockExclusive(&m_lock);
            m_maxConcurrency = (std::max)(threads, 1u);
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Set max throughput in items per second (0 = unlimited).
        inline void SetMaxThroughput(uint32_t itemsPerSecond) {
            AcquireSRWLockExclusive(&m_lock);
            m_maxThroughput = itemsPerSecond;
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Set progress callback, called after each item completes.
        inline void SetProgressCallback(std::function<void(uint64_t batchId, float progress)> fn) {
            AcquireSRWLockExclusive(&m_lock);
            m_progressCallback = std::move(fn);
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Submit a batch of items. Returns batch ID.
        inline uint64_t SubmitBatch(std::vector<BatchItem>&& items) {
            uint64_t batchId = m_nextBatchId.fetch_add(1);

            // Sort items by directory for I/O locality.
            SortByDirectory(items);

            AcquireSRWLockExclusive(&m_lock);
            auto& batch = m_batches[batchId];
            batch.totalItems = static_cast<uint32_t>(items.size());
            batch.completedItems.store(0);
            batch.cancelled.store(false);
            batch.items = std::move(items);
            batch.results.resize(batch.totalItems);
            ReleaseSRWLockExclusive(&m_lock);

            // Start processing in background.
            EnsureWorkersRunning();
            {
                std::lock_guard<std::mutex> lk(m_queueMutex);
                m_pendingBatches.push(batchId);
            }
            m_cv.notify_one();

            return batchId;
        }

        /// Check if a batch is complete.
        inline bool IsBatchComplete(uint64_t batchId) const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            auto it = m_batches.find(batchId);
            bool complete = false;
            if (it != m_batches.end()) {
                complete = it->second.completedItems.load() >= it->second.totalItems;
            }
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return complete;
        }

        /// Get batch results. Returns empty if batch not found or not complete.
        inline std::vector<BatchResult> GetBatchResults(uint64_t batchId) const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            auto it = m_batches.find(batchId);
            std::vector<BatchResult> results;
            if (it != m_batches.end() &&
                it->second.completedItems.load() >= it->second.totalItems) {
                results = it->second.results;
            }
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return results;
        }

        /// Cancel a batch. Already-processing items will complete but remaining are skipped.
        inline void CancelBatch(uint64_t batchId) {
            AcquireSRWLockExclusive(&m_lock);
            auto it = m_batches.find(batchId);
            if (it != m_batches.end()) {
                it->second.cancelled.store(true);
            }
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Get batch progress (0.0 to 1.0).
        inline float GetBatchProgress(uint64_t batchId) const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            auto it = m_batches.find(batchId);
            float progress = 0.0f;
            if (it != m_batches.end() && it->second.totalItems > 0) {
                progress = static_cast<float>(it->second.completedItems.load()) /
                           static_cast<float>(it->second.totalItems);
            }
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return progress;
        }

        /// Get aggregate statistics across all batches.
        inline BatchStats GetStats() const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            BatchStats result = m_globalStats;
            if (result.itemsProcessed > 0) {
                result.avgDecodeTimeUs =
                    static_cast<double>(result.totalDecodeTimeUs) /
                    static_cast<double>(result.itemsProcessed);
                result.failureRate =
                    static_cast<double>(result.totalFailures) /
                    static_cast<double>(result.itemsProcessed);
            }
            // Compute items/sec from total time.
            if (result.totalDecodeTimeUs > 0) {
                double totalSec = static_cast<double>(result.totalDecodeTimeUs) / 1000000.0;
                result.avgItemsPerSecond =
                    static_cast<double>(result.itemsProcessed) / totalSec;
            }
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return result;
        }

    private:
        struct BatchData {
            uint32_t totalItems = 0;
            std::atomic<uint32_t> completedItems{0};
            std::atomic<bool> cancelled{false};
            std::vector<BatchItem> items;
            std::vector<BatchResult> results;
        };

        mutable SRWLOCK m_lock{};
        std::mutex m_queueMutex;
        std::condition_variable m_cv;
        std::atomic<uint64_t> m_nextBatchId{1};
        std::atomic<bool> m_running{false};

        uint32_t m_maxConcurrency = 4;
        uint32_t m_maxThroughput = 0;

        std::function<BatchResult(const BatchItem&)> m_decodeFn;
        std::function<void(uint64_t, float)> m_progressCallback;

        std::unordered_map<uint64_t, BatchData> m_batches;
        std::queue<uint64_t> m_pendingBatches;
        std::vector<std::thread> m_workers;
        BatchStats m_globalStats{};

        /// Sort items by directory path for I/O locality.
        inline static void SortByDirectory(std::vector<BatchItem>& items) {
            std::sort(items.begin(), items.end(),
                      [](const BatchItem& a, const BatchItem& b) {
                          // Extract directory: everything up to last backslash or slash.
                          auto dirA = ExtractDirectory(a.filePath);
                          auto dirB = ExtractDirectory(b.filePath);
                          if (dirA != dirB) return dirA < dirB;
                          return a.filePath < b.filePath;
                      });
        }

        inline static std::wstring ExtractDirectory(const std::wstring& path) {
            auto pos = path.find_last_of(L"\\/");
            if (pos == std::wstring::npos) return L"";
            return path.substr(0, pos);
        }

        inline void EnsureWorkersRunning() {
            if (m_running.load()) return;
            m_running.store(true);
            uint32_t numWorkers = m_maxConcurrency;
            m_workers.reserve(numWorkers);
            for (uint32_t i = 0; i < numWorkers; ++i) {
                m_workers.emplace_back([this]() { DispatcherLoop(); });
            }
        }

        inline void StopInternal() {
            if (!m_running.exchange(false)) return;
            m_cv.notify_all();
            for (auto& w : m_workers) {
                if (w.joinable()) w.join();
            }
            m_workers.clear();
        }

        inline void DispatcherLoop() {
            while (m_running.load()) {
                uint64_t batchId = 0;
                {
                    std::unique_lock<std::mutex> lk(m_queueMutex);
                    m_cv.wait_for(lk, std::chrono::milliseconds(200), [this]() {
                        return !m_running.load() || !m_pendingBatches.empty();
                    });
                    if (!m_running.load()) break;
                    if (m_pendingBatches.empty()) continue;
                    batchId = m_pendingBatches.front();
                    m_pendingBatches.pop();
                }
                ProcessBatch(batchId);
            }
        }

        inline void ProcessBatch(uint64_t batchId) {
            AcquireSRWLockShared(&m_lock);
            auto it = m_batches.find(batchId);
            if (it == m_batches.end()) {
                ReleaseSRWLockShared(&m_lock);
                return;
            }
            uint32_t total = it->second.totalItems;
            ReleaseSRWLockShared(&m_lock);

            // Rate limiting: compute minimum interval between items.
            uint32_t rateLimitUs = 0;
            {
                AcquireSRWLockShared(&m_lock);
                if (m_maxThroughput > 0) {
                    rateLimitUs = 1000000 / m_maxThroughput;
                }
                ReleaseSRWLockShared(&m_lock);
            }

            for (uint32_t idx = 0; idx < total; ++idx) {
                // Check cancellation.
                AcquireSRWLockShared(&m_lock);
                auto batchIt = m_batches.find(batchId);
                if (batchIt == m_batches.end() || batchIt->second.cancelled.load()) {
                    ReleaseSRWLockShared(&m_lock);
                    // Mark remaining as cancelled.
                    AcquireSRWLockExclusive(&m_lock);
                    auto bm = m_batches.find(batchId);
                    if (bm != m_batches.end()) {
                        for (uint32_t r = idx; r < total; ++r) {
                            bm->second.results[r].success = false;
                            bm->second.results[r].error = "Cancelled";
                            bm->second.completedItems.fetch_add(1);
                        }
                    }
                    ReleaseSRWLockExclusive(&m_lock);
                    break;
                }
                BatchItem item = batchIt->second.items[idx];
                ReleaseSRWLockShared(&m_lock);

                // Rate limiting.
                if (rateLimitUs > 0) {
                    Sleep(rateLimitUs / 1000 + 1);
                }

                // Decode the item.
                LARGE_INTEGER startTime{}, endTime{}, freq{};
                QueryPerformanceCounter(&startTime);
                QueryPerformanceFrequency(&freq);

                BatchResult result;
                result.filePath = item.filePath;

                AcquireSRWLockShared(&m_lock);
                auto decodeFnCopy = m_decodeFn;
                ReleaseSRWLockShared(&m_lock);

                if (decodeFnCopy) {
                    result = decodeFnCopy(item);
                } else {
                    result.success = false;
                    result.error = "No decode function registered";
                }

                QueryPerformanceCounter(&endTime);
                if (freq.QuadPart > 0) {
                    result.decodeTimeUs = static_cast<uint32_t>(
                        (endTime.QuadPart - startTime.QuadPart) * 1000000 / freq.QuadPart);
                }

                // Store result.
                AcquireSRWLockExclusive(&m_lock);
                auto bm2 = m_batches.find(batchId);
                if (bm2 != m_batches.end()) {
                    bm2->second.results[idx] = std::move(result);
                    uint32_t completed = bm2->second.completedItems.fetch_add(1) + 1;

                    // Update global stats.
                    auto& r = bm2->second.results[idx];
                    m_globalStats.itemsProcessed++;
                    m_globalStats.totalDecodeTimeUs += r.decodeTimeUs;
                    if (r.success) {
                        m_globalStats.totalSuccesses++;
                    } else {
                        m_globalStats.totalFailures++;
                    }

                    if (completed >= bm2->second.totalItems) {
                        m_globalStats.batchesCompleted++;
                    }

                    // Progress callback.
                    float progress = static_cast<float>(completed) /
                                     static_cast<float>(bm2->second.totalItems);
                    auto progressCb = m_progressCallback;
                    ReleaseSRWLockExclusive(&m_lock);

                    if (progressCb) {
                        progressCb(batchId, progress);
                    }
                } else {
                    ReleaseSRWLockExclusive(&m_lock);
                }
            }
        }
    };

} // namespace Engine
} // namespace ExplorerLens
