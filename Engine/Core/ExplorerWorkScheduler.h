#pragma once
/**
 * @file ExplorerWorkScheduler.h
 * @brief Sprint 572 — Integrates with Windows Explorer's COM threading model for safe thumbnail delivery.
 *
 * Purpose:
 *   Explorer calls IThumbnailProvider on STA/MTA threads. This scheduler manages work items
 *   safely across COM apartment boundaries using an I/O Completion Port (IOCP) for
 *   completion marshaling and an internal configurable thread pool for execution.
 *
 * Classes:
 *   - ExplorerWorkScheduler: Work item scheduler with IOCP-based completion marshaling.
 *
 * Key types:
 *   - WorkItem: Describes a unit of work (work function, completion callback, origin thread).
 *   - WorkState: Lifecycle states — Queued, Processing, Completed, TimedOut, Cancelled.
 *   - WorkStats: Aggregate counters (queued, processing, completed, timed out, cancelled, avg time).
 *
 * Inputs:
 *   - std::function<void()> work and completion callbacks.
 * Outputs:
 *   - Work item IDs (uint64_t), completion status, statistics.
 *
 * Thread safety:
 *   IOCP is inherently thread-safe; internal bookkeeping protected by SRWLOCK.
 *
 * Dependencies: Windows API + C++ standard library only.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <atomic>
#include <chrono>
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

    enum class WorkState : uint8_t {
        Queued,
        Processing,
        Completed,
        TimedOut,
        Cancelled
    };

    struct WorkStats {
        uint64_t queued = 0;
        uint64_t processing = 0;
        uint64_t completed = 0;
        uint64_t timedOut = 0;
        uint64_t cancelled = 0;
        double avgCompletionTimeMs = 0.0;
        uint64_t totalCompletionTimeMs = 0;
    };

    class ExplorerWorkScheduler {
    public:
        inline ExplorerWorkScheduler() noexcept
            : m_hIOCP(nullptr)
            , m_running(false)
            , m_maxConcurrent(4)
            , m_workTimeoutMs(30000)
            , m_nextItemId(1)
        {
            InitializeSRWLock(&m_lock);
        }

        inline ~ExplorerWorkScheduler() noexcept {
            Shutdown();
        }

        ExplorerWorkScheduler(const ExplorerWorkScheduler&) = delete;
        ExplorerWorkScheduler& operator=(const ExplorerWorkScheduler&) = delete;

        /// Initialize the scheduler and start worker threads.
        inline bool Initialize() {
            if (m_running.exchange(true)) return true; // already running

            m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0,
                                             m_maxConcurrent);
            if (!m_hIOCP) {
                m_running.store(false);
                return false;
            }

            uint32_t numWorkers = m_maxConcurrent;
            m_workers.reserve(numWorkers);
            for (uint32_t i = 0; i < numWorkers; ++i) {
                m_workers.emplace_back([this]() { WorkerProc(); });
            }
            return true;
        }

        /// Queue a work item. Returns the work item ID.
        inline uint64_t QueueWork(std::function<void()> work,
                                  std::function<void()> completion = nullptr) {
            if (!m_running.load() || !m_hIOCP) {
                // Auto-initialize if not started.
                if (!Initialize()) return 0;
            }

            uint64_t id = m_nextItemId.fetch_add(1);
            auto now = std::chrono::steady_clock::now();

            auto* ctx = new WorkContext();
            ctx->itemId = id;
            ctx->work = std::move(work);
            ctx->completion = std::move(completion);
            ctx->originThreadId = GetCurrentThreadId();
            ctx->submitTime = now;

            {
                AcquireSRWLockExclusive(&m_lock);
                m_items[id] = WorkState::Queued;
                m_stats.queued++;
                ReleaseSRWLockExclusive(&m_lock);
            }

            // Post to IOCP for worker pickup. Use completion key = 1 for work items.
            BOOL ok = PostQueuedCompletionStatus(m_hIOCP, 0, kWorkKey,
                                                 reinterpret_cast<LPOVERLAPPED>(ctx));
            if (!ok) {
                AcquireSRWLockExclusive(&m_lock);
                m_items.erase(id);
                m_stats.queued--;
                ReleaseSRWLockExclusive(&m_lock);
                delete ctx;
                return 0;
            }
            return id;
        }

        /// Process completed work callbacks for the calling thread.
        /// Call this periodically from the origin thread (e.g., STA pump).
        inline void ProcessCompletions() {
            std::vector<std::function<void()>> toInvoke;
            {
                AcquireSRWLockExclusive(&m_lock);
                DWORD tid = GetCurrentThreadId();
                auto it = m_pendingCompletions.find(tid);
                if (it != m_pendingCompletions.end()) {
                    toInvoke = std::move(it->second);
                    m_pendingCompletions.erase(it);
                }
                ReleaseSRWLockExclusive(&m_lock);
            }
            for (auto& fn : toInvoke) {
                if (fn) fn();
            }
        }

        /// Set maximum concurrent work items (thread pool size).
        /// Must be called before Initialize(), or call Shutdown() then re-Initialize().
        inline void SetMaxConcurrentWork(uint32_t max) {
            m_maxConcurrent = (std::max)(max, 1u);
        }

        /// Set work item timeout in milliseconds (default 30000).
        inline void SetWorkTimeout(uint32_t ms) {
            m_workTimeoutMs = (std::max)(ms, 100u);
        }

        /// Block until all pending work completes or timeout expires. Returns true if all done.
        inline bool WaitForAll(uint32_t timeoutMs) {
            auto deadline = std::chrono::steady_clock::now() +
                            std::chrono::milliseconds(timeoutMs);
            while (std::chrono::steady_clock::now() < deadline) {
                AcquireSRWLockShared(&m_lock);
                bool allDone = true;
                for (auto& [id, state] : m_items) {
                    if (state == WorkState::Queued || state == WorkState::Processing) {
                        allDone = false;
                        break;
                    }
                }
                ReleaseSRWLockShared(&m_lock);
                if (allDone) return true;
                Sleep(1);
            }
            return false;
        }

        /// Cancel all pending work items.
        inline void CancelAll() {
            AcquireSRWLockExclusive(&m_lock);
            for (auto& [id, state] : m_items) {
                if (state == WorkState::Queued) {
                    state = WorkState::Cancelled;
                    m_stats.cancelled++;
                }
            }
            ReleaseSRWLockExclusive(&m_lock);
        }

        /// Get scheduler statistics.
        inline WorkStats GetStats() const {
            AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            WorkStats result = m_stats;
            if (result.completed > 0) {
                result.avgCompletionTimeMs =
                    static_cast<double>(result.totalCompletionTimeMs) /
                    static_cast<double>(result.completed);
            }
            // Count active items.
            result.processing = 0;
            result.queued = 0;
            for (auto& [id, state] : m_items) {
                if (state == WorkState::Queued) result.queued++;
                else if (state == WorkState::Processing) result.processing++;
            }
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return result;
        }

    private:
        static constexpr ULONG_PTR kWorkKey = 1;
        static constexpr ULONG_PTR kShutdownKey = 0;

        struct WorkContext {
            uint64_t itemId = 0;
            std::function<void()> work;
            std::function<void()> completion;
            DWORD originThreadId = 0;
            std::chrono::steady_clock::time_point submitTime{};
        };

        mutable SRWLOCK m_lock{};
        HANDLE m_hIOCP;
        std::atomic<bool> m_running;
        uint32_t m_maxConcurrent;
        uint32_t m_workTimeoutMs;
        std::atomic<uint64_t> m_nextItemId;

        std::vector<std::thread> m_workers;
        std::unordered_map<uint64_t, WorkState> m_items;
        std::unordered_map<DWORD, std::vector<std::function<void()>>> m_pendingCompletions;
        WorkStats m_stats{};

        inline void Shutdown() {
            if (!m_running.exchange(false)) return;
            if (m_hIOCP) {
                // Post shutdown signals.
                for (size_t i = 0; i < m_workers.size(); ++i) {
                    PostQueuedCompletionStatus(m_hIOCP, 0, kShutdownKey, nullptr);
                }
            }
            for (auto& w : m_workers) {
                if (w.joinable()) w.join();
            }
            m_workers.clear();
            if (m_hIOCP) {
                CloseHandle(m_hIOCP);
                m_hIOCP = nullptr;
            }
        }

        inline void WorkerProc() {
            while (m_running.load()) {
                DWORD bytesTransferred = 0;
                ULONG_PTR completionKey = 0;
                LPOVERLAPPED pOverlapped = nullptr;

                BOOL ok = GetQueuedCompletionStatus(
                    m_hIOCP, &bytesTransferred, &completionKey,
                    &pOverlapped, 500); // 500ms timeout for periodic shutdown check

                if (!ok && !pOverlapped) {
                    // Timeout or error — just loop and check running flag.
                    continue;
                }

                if (completionKey == kShutdownKey) {
                    break;
                }

                if (completionKey == kWorkKey && pOverlapped) {
                    auto* ctx = reinterpret_cast<WorkContext*>(pOverlapped);
                    ProcessWorkItem(ctx);
                }
            }
        }

        inline void ProcessWorkItem(WorkContext* ctx) {
            if (!ctx) return;

            uint64_t id = ctx->itemId;
            auto startTime = std::chrono::steady_clock::now();

            // Check if cancelled.
            bool cancelled = false;
            {
                AcquireSRWLockExclusive(&m_lock);
                auto it = m_items.find(id);
                if (it != m_items.end() && it->second == WorkState::Cancelled) {
                    cancelled = true;
                } else if (it != m_items.end()) {
                    it->second = WorkState::Processing;
                }
                ReleaseSRWLockExclusive(&m_lock);
            }

            if (cancelled) {
                delete ctx;
                return;
            }

            // Execute the work function with a timeout check.
            // We run the work synchronously — timeout is advisory (recorded after the fact).
            bool timedOut = false;
            if (ctx->work) {
                ctx->work();
            }
            auto endTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime).count();

            if (static_cast<uint32_t>(elapsedMs) > m_workTimeoutMs) {
                timedOut = true;
            }

            // Update state and stats.
            {
                AcquireSRWLockExclusive(&m_lock);
                auto it = m_items.find(id);
                if (it != m_items.end()) {
                    if (timedOut) {
                        it->second = WorkState::TimedOut;
                        m_stats.timedOut++;
                    } else {
                        it->second = WorkState::Completed;
                        m_stats.completed++;
                        m_stats.totalCompletionTimeMs += static_cast<uint64_t>(elapsedMs);
                    }
                }
                // Marshal completion to origin thread.
                if (ctx->completion) {
                    m_pendingCompletions[ctx->originThreadId].push_back(ctx->completion);
                }
                ReleaseSRWLockExclusive(&m_lock);
            }

            delete ctx;
        }
    };

} // namespace Engine
} // namespace ExplorerLens
