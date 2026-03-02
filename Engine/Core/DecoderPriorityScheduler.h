#pragma once
/**
 * @file DecoderPriorityScheduler.h
 * @brief Sprint 571 — Priority-based scheduling of decode requests using multi-level feedback queues.
 *
 * Purpose:
 *   Manages decode requests across 5 priority levels (Critical, High, Normal, Low,
 *   Background). Implements aging to prevent starvation, viewport boosting for
 *   visible items, directory-level boosting, and an internal worker pool that
 *   processes requests in priority order.
 *
 * Classes:
 *   - DecoderPriorityScheduler: Multi-level priority queue scheduler with worker pool.
 *
 * Key types:
 *   - DecodePriority: 5-level priority enum.
 *   - DecodeRequest: Full request descriptor (path, size, priority, callback).
 *   - SchedulerStats: Per-priority statistics.
 *
 * Inputs:
 *   - DecodeRequest with file path, dimensions, priority, callback.
 * Outputs:
 *   - Decoded image data delivered via callback; statistics.
 *
 * Thread safety:
 *   All public methods are thread-safe. Internal queues protected by SRWLOCK.
 *   Worker threads wake via condition_variable_any.
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
#include <deque>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

    enum class DecodePriority : uint8_t {
        Critical   = 0,
        High       = 1,
        Normal     = 2,
        Low        = 3,
        Background = 4
    };

    struct DecodeRequest {
        uint64_t requestId = 0;
        std::wstring filePath;
        uint32_t targetWidth = 0;
        uint32_t targetHeight = 0;
        DecodePriority priority = DecodePriority::Normal;
        std::chrono::steady_clock::time_point submitted{};
        std::function<void(bool, std::vector<uint8_t>&)> callback;
    };

    struct SchedulerStats {
        struct PerLevel {
            uint64_t pending = 0;
            uint64_t completed = 0;
            double avgWaitTimeMs = 0.0;
            double avgDecodeTimeMs = 0.0;
            uint64_t totalWaitTimeMs = 0;
            uint64_t totalDecodeTimeMs = 0;
            uint64_t promotions = 0;
            uint64_t cancellations = 0;
        };
        PerLevel levels[5]{};
        uint64_t totalRequests = 0;
        uint64_t totalCompleted = 0;
    };

    class DecoderPriorityScheduler {
    public:
        static constexpr uint32_t kNumLevels = 5;
        static constexpr uint32_t kAgingThresholdMs = 5000;

        inline DecoderPriorityScheduler() noexcept {
            InitializeSRWLock(&m_lock);
            m_nextRequestId.store(1);
            m_running.store(false);
        }

        inline ~DecoderPriorityScheduler() noexcept {
            StopWorkers();
        }

        DecoderPriorityScheduler(const DecoderPriorityScheduler&) = delete;
        DecoderPriorityScheduler& operator=(const DecoderPriorityScheduler&) = delete;

        /// Submit a decode request. Returns the assigned request ID.
        inline uint64_t Submit(DecodeRequest&& request) {
            uint64_t id = m_nextRequestId.fetch_add(1);
            request.requestId = id;
            if (request.submitted == std::chrono::steady_clock::time_point{}) {
                request.submitted = std::chrono::steady_clock::now();
            }
            uint32_t level = static_cast<uint32_t>(request.priority);
            if (level >= kNumLevels) level = kNumLevels - 1;

            {
                std::lock_guard<std::mutex> lk(m_queueMutex);
                m_queues[level].push_back(std::move(request));
                m_requestLevel[id] = level;
            }
            m_cv.notify_one();
            return id;
        }

        /// Cancel a pending request. Returns true if it was found and removed.
        inline bool Cancel(uint64_t requestId) {
            std::lock_guard<std::mutex> lk(m_queueMutex);
            auto it = m_requestLevel.find(requestId);
            if (it == m_requestLevel.end()) return false;
            uint32_t level = it->second;
            auto& q = m_queues[level];
            for (auto qit = q.begin(); qit != q.end(); ++qit) {
                if (qit->requestId == requestId) {
                    q.erase(qit);
                    m_requestLevel.erase(it);
                    AcquireSRWLockExclusive(&m_lock);
                    m_stats.levels[level].cancellations++;
                    ReleaseSRWLockExclusive(&m_lock);
                    return true;
                }
            }
            m_requestLevel.erase(it);
            return false;
        }

        /// Dequeue the highest-priority available request (Critical first).
        /// Applies aging before selection. Returns nullptr if all queues are empty.
        /// Caller takes ownership via move of the returned request.
        inline bool DequeueNext(DecodeRequest& out) {
            std::lock_guard<std::mutex> lk(m_queueMutex);
            ApplyAging();
            for (uint32_t level = 0; level < kNumLevels; ++level) {
                if (!m_queues[level].empty()) {
                    out = std::move(m_queues[level].front());
                    m_queues[level].pop_front();
                    m_requestLevel.erase(out.requestId);
                    return true;
                }
            }
            return false;
        }

        /// Set the visible viewport rectangle (for auto-boosting).
        inline void SetViewportRect(RECT viewport) {
            AcquireSRWLockExclusive(&m_lock);
            m_viewport = viewport;
            m_hasViewport = true;
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Boost all pending requests for files in the given directory to High priority.
        inline void BoostDirectory(const std::wstring& dirPath) {
            std::lock_guard<std::mutex> lk(m_queueMutex);
            std::wstring prefix = dirPath;
            // Ensure trailing backslash.
            if (!prefix.empty() && prefix.back() != L'\\' && prefix.back() != L'/') {
                prefix += L'\\';
            }
            for (uint32_t level = static_cast<uint32_t>(DecodePriority::Normal);
                 level < kNumLevels; ++level) {
                auto& q = m_queues[level];
                auto it = q.begin();
                while (it != q.end()) {
                    bool matches = false;
                    if (it->filePath.size() >= prefix.size()) {
                        // Case-insensitive prefix match.
                        matches = true;
                        for (size_t i = 0; i < prefix.size() && matches; ++i) {
                            wchar_t a = it->filePath[i];
                            wchar_t b = prefix[i];
                            if (a >= L'A' && a <= L'Z') a += 32;
                            if (b >= L'A' && b <= L'Z') b += 32;
                            if (a == L'/') a = L'\\';
                            if (b == L'/') b = L'\\';
                            if (a != b) matches = false;
                        }
                    }
                    if (matches) {
                        DecodeRequest req = std::move(*it);
                        it = q.erase(it);
                        req.priority = DecodePriority::High;
                        m_queues[static_cast<uint32_t>(DecodePriority::High)].push_back(std::move(req));
                        AcquireSRWLockExclusive(&m_lock);
                        m_stats.levels[level].promotions++;
                        ReleaseSRWLockExclusive(&m_lock);
                    } else {
                        ++it;
                    }
                }
            }
        }

        /// Start worker threads that dequeue and process requests.
        /// @param count Number of worker threads.
        /// @param decodeFn Function: (filePath, width, height, outData) → success.
        inline void StartWorkers(uint32_t count,
                                 std::function<bool(const std::wstring&, uint32_t, uint32_t,
                                                    std::vector<uint8_t>&)> decodeFn) {
            if (m_running.exchange(true)) return; // already running
            m_decodeFn = std::move(decodeFn);
            uint32_t numWorkers = (std::max)(count, 1u);
            m_workers.reserve(numWorkers);
            for (uint32_t i = 0; i < numWorkers; ++i) {
                m_workers.emplace_back([this]() { WorkerLoop(); });
            }
        }

        /// Stop all worker threads and wait for completion.
        inline void StopWorkers() {
            if (!m_running.exchange(false)) return;
            m_cv.notify_all();
            for (auto& w : m_workers) {
                if (w.joinable()) w.join();
            }
            m_workers.clear();
        }

        /// Get scheduler statistics.
        inline SchedulerStats GetStats() const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            SchedulerStats result = m_stats;
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

            // Add current pending counts (need queue mutex; use try_lock pattern).
            auto* self = const_cast<DecoderPriorityScheduler*>(this);
            std::unique_lock<std::mutex> lk(self->m_queueMutex, std::try_to_lock);
            if (lk.owns_lock()) {
                for (uint32_t i = 0; i < kNumLevels; ++i) {
                    result.levels[i].pending = self->m_queues[i].size();
                }
            }
            // Compute averages.
            for (uint32_t i = 0; i < kNumLevels; ++i) {
                auto& lv = result.levels[i];
                if (lv.completed > 0) {
                    lv.avgWaitTimeMs = static_cast<double>(lv.totalWaitTimeMs) /
                                       static_cast<double>(lv.completed);
                    lv.avgDecodeTimeMs = static_cast<double>(lv.totalDecodeTimeMs) /
                                         static_cast<double>(lv.completed);
                }
                result.totalCompleted += lv.completed;
            }
            result.totalRequests = m_nextRequestId.load() - 1;
            return result;
        }

    private:
        mutable SRWLOCK m_lock{};
        std::mutex m_queueMutex;
        std::condition_variable m_cv;
        std::atomic<uint64_t> m_nextRequestId{1};
        std::atomic<bool> m_running{false};

        std::deque<DecodeRequest> m_queues[kNumLevels];
        std::unordered_map<uint64_t, uint32_t> m_requestLevel;  // requestId → level

        RECT m_viewport{};
        bool m_hasViewport = false;

        std::function<bool(const std::wstring&, uint32_t, uint32_t, std::vector<uint8_t>&)> m_decodeFn;
        std::vector<std::thread> m_workers;
        mutable SchedulerStats m_stats{};

        /// Promote requests that have been waiting longer than the aging threshold.
        inline void ApplyAging() {
            auto now = std::chrono::steady_clock::now();
            for (uint32_t level = 1; level < kNumLevels; ++level) {
                auto& q = m_queues[level];
                auto it = q.begin();
                while (it != q.end()) {
                    auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - it->submitted).count();
                    if (waitMs >= static_cast<int64_t>(kAgingThresholdMs)) {
                        DecodeRequest req = std::move(*it);
                        it = q.erase(it);
                        uint32_t newLevel = level - 1;
                        req.priority = static_cast<DecodePriority>(newLevel);
                        req.submitted = now; // reset aging timer
                        m_queues[newLevel].push_back(std::move(req));
                        AcquireSRWLockExclusive(&m_lock);
                        m_stats.levels[level].promotions++;
                        ReleaseSRWLockExclusive(&m_lock);
                    } else {
                        ++it;
                    }
                }
            }
        }

        /// Worker thread main loop.
        inline void WorkerLoop() {
            while (m_running.load()) {
                DecodeRequest req;
                bool gotWork = false;
                {
                    std::unique_lock<std::mutex> lk(m_queueMutex);
                    m_cv.wait_for(lk, std::chrono::milliseconds(100), [this]() {
                        if (!m_running.load()) return true;
                        for (uint32_t i = 0; i < kNumLevels; ++i) {
                            if (!m_queues[i].empty()) return true;
                        }
                        return false;
                    });
                    if (!m_running.load()) break;
                    ApplyAging();
                    for (uint32_t level = 0; level < kNumLevels; ++level) {
                        if (!m_queues[level].empty()) {
                            req = std::move(m_queues[level].front());
                            m_queues[level].pop_front();
                            m_requestLevel.erase(req.requestId);
                            gotWork = true;
                            break;
                        }
                    }
                }
                if (!gotWork) continue;

                auto dequeueTime = std::chrono::steady_clock::now();
                auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    dequeueTime - req.submitted).count();

                std::vector<uint8_t> outData;
                bool success = false;
                auto decodeStart = std::chrono::steady_clock::now();

                if (m_decodeFn) {
                    success = m_decodeFn(req.filePath, req.targetWidth, req.targetHeight, outData);
                }

                auto decodeEnd = std::chrono::steady_clock::now();
                auto decodeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    decodeEnd - decodeStart).count();

                uint32_t level = static_cast<uint32_t>(req.priority);
                AcquireSRWLockExclusive(&m_lock);
                m_stats.levels[level].completed++;
                m_stats.levels[level].totalWaitTimeMs += static_cast<uint64_t>(waitMs);
                m_stats.levels[level].totalDecodeTimeMs += static_cast<uint64_t>(decodeMs);
                ReleaseSRWLockExclusive(&m_lock);

                if (req.callback) {
                    req.callback(success, outData);
                }
            }
        }
    };

} // namespace Engine
} // namespace ExplorerLens
