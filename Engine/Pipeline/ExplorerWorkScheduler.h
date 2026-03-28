#pragma once
// Explorer Work Scheduler
// Viewport-aware thumbnail request prioritization with cancel-on-scroll
// and adaptive concurrency for Explorer batch operations.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <functional>
#include <queue>

namespace ExplorerLens::Pipeline {

// ─── Work item priority ────────────────────────────────────────────
enum class WorkPriority : uint8_t {
 Critical = 0, // Visible in viewport, user waiting
 High = 1, // Near viewport (prefetch zone)
 Normal = 2, // Queued but not yet visible
 Low = 3, // Background prefetch
 Cancelled = 4 // Marked for skip
};

inline const char* PriorityName(WorkPriority p) {
 switch (p) {
 case WorkPriority::Critical: return "Critical";
 case WorkPriority::High: return "High";
 case WorkPriority::Normal: return "Normal";
 case WorkPriority::Low: return "Low";
 case WorkPriority::Cancelled: return "Cancelled";
 default: return "Unknown";
 }
}

// ─── Work item ─────────────────────────────────────────────────────
struct ThumbnailWorkItem {
 uint64_t id = 0;
 std::string filePath;
 uint32_t requestedWidth = 256;
 uint32_t requestedHeight = 256;
 WorkPriority priority = WorkPriority::Normal;
 int32_t viewportIndex = -1; // position in current viewport (-1 = not visible)
 bool cancelled = false;

 using TimePoint = std::chrono::steady_clock::time_point;
 TimePoint submittedAt;
 TimePoint startedAt;
 TimePoint completedAt;

 double LatencyMs() const {
 if (startedAt == TimePoint{} || completedAt == TimePoint{})
 return 0.0;
 return std::chrono::duration<double, std::milli>(completedAt - startedAt).count();
 }

 double QueueWaitMs() const {
 if (submittedAt == TimePoint{} || startedAt == TimePoint{})
 return 0.0;
 return std::chrono::duration<double, std::milli>(startedAt - submittedAt).count();
 }

 bool IsVisible() const { return viewportIndex >= 0; }
 bool IsCancelled() const { return cancelled || priority == WorkPriority::Cancelled; }
};

// ─── Viewport state ───────────────────────────────────────────────
struct WorkSchedulerViewport {
 int32_t firstVisibleIndex = 0;
 int32_t lastVisibleIndex = 0;
 int32_t prefetchBefore = 10; // items to prefetch before viewport
 int32_t prefetchAfter = 20; // items to prefetch after viewport

 int32_t VisibleCount() const { return lastVisibleIndex - firstVisibleIndex + 1; }

 bool IsInViewport(int32_t index) const {
 return index >= firstVisibleIndex && index <= lastVisibleIndex;
 }

 bool IsInPrefetchZone(int32_t index) const {
 return index >= (firstVisibleIndex - prefetchBefore) &&
 index <= (lastVisibleIndex + prefetchAfter) &&
 !IsInViewport(index);
 }

 WorkPriority PriorityForIndex(int32_t index) const {
 if (IsInViewport(index)) return WorkPriority::Critical;
 if (IsInPrefetchZone(index)) return WorkPriority::High;
 return WorkPriority::Low;
 }
};

// ─── Scheduler statistics ──────────────────────────────────────────
struct WorkSchedulerStats {
 uint64_t totalSubmitted = 0;
 uint64_t totalCompleted = 0;
 uint64_t totalCancelled = 0;
 uint64_t totalSkipped = 0;
 uint64_t currentQueueSize = 0;
 uint64_t scrollEvents = 0;
 double avgLatencyMs = 0.0;
 double avgQueueWaitMs = 0.0;
 size_t activeConcurrency = 0;

 double CompletionRate() const {
 return totalSubmitted > 0
 ? static_cast<double>(totalCompleted) / totalSubmitted
 : 0.0;
 }

 double CancellationRate() const {
 return totalSubmitted > 0
 ? static_cast<double>(totalCancelled) / totalSubmitted
 : 0.0;
 }
};

// ─── Scheduler configuration ──────────────────────────────────────
struct SchedulerConfig {
 size_t maxConcurrency = 4;
 size_t maxQueueSize = 200;
 bool cancelOnScroll = true;
 bool prioritizeViewport = true;
 uint32_t scrollDebounceMs = 50;

 static SchedulerConfig Default() { return {}; }
 static SchedulerConfig LowLatency() {
 SchedulerConfig c;
 c.maxConcurrency = 2;
 c.maxQueueSize = 50;
 c.scrollDebounceMs = 30;
 return c;
 }
 static SchedulerConfig HighThroughput() {
 SchedulerConfig c;
 c.maxConcurrency = 8;
 c.maxQueueSize = 500;
 c.cancelOnScroll = false;
 return c;
 }
};

// ─── Priority comparator for queue ────────────────────────────────
struct WorkItemComparator {
 bool operator()(const ThumbnailWorkItem& a, const ThumbnailWorkItem& b) const {
 // Lower priority value = higher urgency, so reverse
 if (a.priority != b.priority)
 return static_cast<uint8_t>(a.priority) > static_cast<uint8_t>(b.priority);
 // Among same priority, prefer items closer to viewport center
 return a.viewportIndex > b.viewportIndex;
 }
};

// ─── Explorer Work Scheduler ──────────────────────────────────────
class ExplorerWorkScheduler {
public:
 explicit ExplorerWorkScheduler(SchedulerConfig config = SchedulerConfig::Default())
 : m_config(config) {}

 uint64_t Submit(const std::string& filePath, int32_t viewportIndex = -1) {
 std::lock_guard<std::mutex> lock(m_mutex);
 if (m_queue.size() >= m_config.maxQueueSize) return 0;

 ThumbnailWorkItem item;
 item.id = ++m_nextId;
 item.filePath = filePath;
 item.viewportIndex = viewportIndex;
 item.submittedAt = std::chrono::steady_clock::now();

 if (m_config.prioritizeViewport) {
 item.priority = m_viewport.PriorityForIndex(viewportIndex);
 }

 m_queue.push(item);
 m_stats.totalSubmitted++;
 m_stats.currentQueueSize = m_queue.size();
 return item.id;
 }

 bool Cancel(uint64_t itemId) {
 // Mark as cancelled — actual removal happens at dequeue time
 std::lock_guard<std::mutex> lock(m_mutex);
 m_cancelledIds.push_back(itemId);
 m_stats.totalCancelled++;
 return true;
 }

 ThumbnailWorkItem Dequeue() {
 std::lock_guard<std::mutex> lock(m_mutex);
 while (!m_queue.empty()) {
 auto item = m_queue.top();
 m_queue.pop();

 // Skip cancelled items
 auto it = std::find(m_cancelledIds.begin(), m_cancelledIds.end(), item.id);
 if (it != m_cancelledIds.end()) {
 m_cancelledIds.erase(it);
 m_stats.totalSkipped++;
 continue;
 }

 item.startedAt = std::chrono::steady_clock::now();
 m_stats.currentQueueSize = m_queue.size();
 return item;
 }

 m_stats.currentQueueSize = 0;
 return {}; // empty item
 }

 void Complete(ThumbnailWorkItem& item) {
 std::lock_guard<std::mutex> lock(m_mutex);
 item.completedAt = std::chrono::steady_clock::now();
 m_stats.totalCompleted++;

 // Update rolling average latency
 double lat = item.LatencyMs();
 if (m_stats.totalCompleted == 1) {
 m_stats.avgLatencyMs = lat;
 } else {
 m_stats.avgLatencyMs = m_stats.avgLatencyMs * 0.9 + lat * 0.1;
 }
 }

 void OnScroll(int32_t newFirstVisible, int32_t newLastVisible) {
 std::lock_guard<std::mutex> lock(m_mutex);
 m_viewport.firstVisibleIndex = newFirstVisible;
 m_viewport.lastVisibleIndex = newLastVisible;
 m_stats.scrollEvents++;

 if (m_config.cancelOnScroll) {
 // Cancel all items outside new prefetch zone
 CancelOutOfViewportLocked();
 }
 }

 void UpdateViewport(const WorkSchedulerViewport& vs) {
 std::lock_guard<std::mutex> lock(m_mutex);
 m_viewport = vs;
 }

 WorkSchedulerStats GetStats() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 return m_stats;
 }

 WorkSchedulerViewport GetViewport() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 return m_viewport;
 }

 SchedulerConfig GetConfig() const { return m_config; }

 size_t QueueSize() const {
 std::lock_guard<std::mutex> lock(m_mutex);
 return m_queue.size();
 }

 void ClearQueue() {
 std::lock_guard<std::mutex> lock(m_mutex);
 while (!m_queue.empty()) m_queue.pop();
 m_cancelledIds.clear();
 m_stats.currentQueueSize = 0;
 }

 static ExplorerWorkScheduler Create(SchedulerConfig config = SchedulerConfig::Default()) {
 return ExplorerWorkScheduler(config);
 }

private:
 void CancelOutOfViewportLocked() {
 // Rebuild queue keeping only viewport + prefetch items
 std::priority_queue<ThumbnailWorkItem, std::vector<ThumbnailWorkItem>, WorkItemComparator> newQueue;

 while (!m_queue.empty()) {
 auto item = m_queue.top();
 m_queue.pop();
 if (item.viewportIndex < 0 ||
 m_viewport.IsInViewport(item.viewportIndex) ||
 m_viewport.IsInPrefetchZone(item.viewportIndex)) {
 newQueue.push(item);
 } else {
 m_stats.totalCancelled++;
 }
 }

 m_queue = std::move(newQueue);
 m_stats.currentQueueSize = m_queue.size();
 }

 SchedulerConfig m_config;
 WorkSchedulerViewport m_viewport;
 WorkSchedulerStats m_stats;
 std::priority_queue<ThumbnailWorkItem, std::vector<ThumbnailWorkItem>, WorkItemComparator> m_queue;
 std::vector<uint64_t> m_cancelledIds;
 uint64_t m_nextId = 0;
 mutable std::mutex m_mutex;
};

} // namespace ExplorerLens::Pipeline

