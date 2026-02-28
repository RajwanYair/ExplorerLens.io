// ============================================================================
// JumpListIntegration.h — Windows Jump List for Recently Browsed Folders
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Manages Windows 10/11 Jump List entries for recently accessed archives
// and folders. Integrates with ICustomDestinationList COM interface.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Jump List entry representing a recently accessed item
// ============================================================================

struct JumpListEntry {
    std::wstring filePath;          // Full path to archive or folder
    std::wstring displayName;       // User-visible title
    std::wstring formatType;        // e.g., L"ZIP", L"RAR", L"RAW"
    std::wstring iconPath;          // Custom icon path (empty = default)
    int32_t      iconIndex = 0;     // Icon resource index
    uint64_t     lastAccessTime = 0; // FILETIME as uint64
    uint32_t     accessCount = 0; // Number of times accessed
    uint32_t     entryCount = 0; // Number of items in archive
    uint64_t     fileSize = 0; // Archive size in bytes
    bool         pinned = false;
};

// ============================================================================
// Jump List category types
// ============================================================================

enum class JumpListCategory : uint8_t {
    Recent,          // Recently opened archives
    Frequent,        // Most frequently accessed
    Pinned,          // User-pinned entries
    Tasks            // Custom action tasks
};

inline const char* JumpListCategoryToString(JumpListCategory cat) {
    static const char* names[] = { "Recent", "Frequent", "Pinned", "Tasks" };
    return names[static_cast<uint8_t>(cat)];
}

// ============================================================================
// Jump List task (custom action in Tasks category)
// ============================================================================

struct JumpListTask {
    std::wstring title;             // Task display name
    std::wstring description;       // Tooltip text
    std::wstring commandPath;       // Executable path
    std::wstring arguments;         // Command-line arguments
    std::wstring iconPath;          // Icon path
    int32_t      iconIndex = 0;
};

// ============================================================================
// JumpListIntegration — manages Jump List entries for ExplorerLens
// ============================================================================

class JumpListIntegration {
public:
    /// Maximum entries per category
    static constexpr uint32_t MAX_RECENT_ENTRIES = 20;
    static constexpr uint32_t MAX_FREQUENT_ENTRIES = 10;
    static constexpr uint32_t MAX_PINNED_ENTRIES = 10;
    static constexpr uint32_t MAX_TASKS = 5;

    /// Minimum time between Jump List COM updates (avoid excessive Explorer refresh)
    static constexpr uint32_t MIN_UPDATE_INTERVAL_MS = 2000;

    JumpListIntegration() = default;
    ~JumpListIntegration() = default;

    // Non-copyable, movable
    JumpListIntegration(const JumpListIntegration&) = delete;
    JumpListIntegration& operator=(const JumpListIntegration&) = delete;
    JumpListIntegration(JumpListIntegration&&) noexcept = default;
    JumpListIntegration& operator=(JumpListIntegration&&) noexcept = default;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize Jump List integration with an App User Model ID
    bool Initialize(const std::wstring& appUserModelId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_appUserModelId = appUserModelId;
        m_initialized = true;

        // Add default tasks
        AddDefaultTasksLocked();
        return true;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_recentEntries.clear();
        m_frequentEntries.clear();
        m_pinnedEntries.clear();
        m_tasks.clear();
        m_initialized = false;
    }

    // ========================================================================
    // Recent / Frequent tracking
    // ========================================================================

    /// Record that a file was accessed (updates both recent and frequent lists)
    void RecordAccess(const std::wstring& filePath,
        const std::wstring& displayName,
        const std::wstring& formatType,
        uint64_t fileSize = 0,
        uint32_t entryCount = 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return;

        uint64_t now = GetCurrentFileTime();

        // Update or insert in recent list
        auto it = FindEntryLocked(m_recentEntries, filePath);
        if (it != m_recentEntries.end()) {
            it->lastAccessTime = now;
            it->accessCount++;
        }
        else {
            JumpListEntry entry;
            entry.filePath = filePath;
            entry.displayName = displayName;
            entry.formatType = formatType;
            entry.fileSize = fileSize;
            entry.entryCount = entryCount;
            entry.lastAccessTime = now;
            entry.accessCount = 1;
            m_recentEntries.push_front(entry);
        }

        // Trim recent list
        while (m_recentEntries.size() > MAX_RECENT_ENTRIES) {
            m_recentEntries.pop_back();
        }

        // Update frequent list (sorted by access count)
        UpdateFrequentListLocked();

        // Throttled COM update
        MaybeUpdateJumpListLocked();
    }

    // ========================================================================
    // Pinning
    // ========================================================================

    /// Pin an entry so it persists across sessions
    bool PinEntry(const std::wstring& filePath) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pinnedEntries.size() >= MAX_PINNED_ENTRIES) return false;

        // Find in recent or frequent
        auto it = FindEntryLocked(m_recentEntries, filePath);
        if (it != m_recentEntries.end()) {
            it->pinned = true;
            JumpListEntry pinned = *it;
            m_pinnedEntries.push_back(std::move(pinned));
            MaybeUpdateJumpListLocked();
            return true;
        }
        return false;
    }

    /// Unpin an entry
    bool UnpinEntry(const std::wstring& filePath) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_pinnedEntries.begin(), m_pinnedEntries.end(),
            [&](const JumpListEntry& e) { return e.filePath == filePath; });
        if (it != m_pinnedEntries.end()) {
            m_pinnedEntries.erase(it);

            // Also unpin in recent list
            auto rit = FindEntryLocked(m_recentEntries, filePath);
            if (rit != m_recentEntries.end()) rit->pinned = false;

            MaybeUpdateJumpListLocked();
            return true;
        }
        return false;
    }

    // ========================================================================
    // Custom tasks
    // ========================================================================

    /// Add a custom task to the Tasks category
    bool AddTask(const JumpListTask& task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_tasks.size() >= MAX_TASKS) return false;
        m_tasks.push_back(task);
        MaybeUpdateJumpListLocked();
        return true;
    }

    // ========================================================================
    // Queries
    // ========================================================================

    /// Get entries for a specific category
    std::vector<JumpListEntry> GetEntries(JumpListCategory category) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        switch (category) {
        case JumpListCategory::Recent:
            return { m_recentEntries.begin(), m_recentEntries.end() };
        case JumpListCategory::Frequent:
            return m_frequentEntries;
        case JumpListCategory::Pinned:
            return m_pinnedEntries;
        default:
            return {};
        }
    }

    std::vector<JumpListTask> GetTasks() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_tasks;
    }

    uint32_t GetRecentCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_recentEntries.size());
    }

    uint32_t GetPinnedCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_pinnedEntries.size());
    }

    bool IsInitialized() const { return m_initialized; }

    // ========================================================================
    // Serialization (for persistence across sessions)
    // ========================================================================

    /// Export entries as a flat list (for JSON/registry serialization)
    std::vector<JumpListEntry> ExportAllEntries() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<JumpListEntry> all;
        all.reserve(m_recentEntries.size() + m_pinnedEntries.size());

        for (const auto& e : m_pinnedEntries) all.push_back(e);
        for (const auto& e : m_recentEntries) {
            if (!e.pinned) all.push_back(e);
        }
        return all;
    }

    /// Import entries (typically from persisted storage)
    void ImportEntries(const std::vector<JumpListEntry>& entries) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_recentEntries.clear();
        m_pinnedEntries.clear();

        for (const auto& e : entries) {
            if (e.pinned) {
                m_pinnedEntries.push_back(e);
            }
            m_recentEntries.push_back(e);
        }

        // Trim
        while (m_recentEntries.size() > MAX_RECENT_ENTRIES) {
            m_recentEntries.pop_back();
        }

        UpdateFrequentListLocked();
    }

    // ========================================================================
    // COM interaction point (called by shell extension)
    // ========================================================================

    /// Force update the Windows Jump List via COM
    /// Returns true if the COM call succeeded
    bool ForceUpdate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return UpdateJumpListCOMLocked();
    }

private:
    // ========================================================================
    // Internal helpers
    // ========================================================================

    using EntryIterator = std::deque<JumpListEntry>::iterator;

    EntryIterator FindEntryLocked(std::deque<JumpListEntry>& list,
        const std::wstring& filePath) {
        return std::find_if(list.begin(), list.end(),
            [&](const JumpListEntry& e) { return e.filePath == filePath; });
    }

    void UpdateFrequentListLocked() {
        m_frequentEntries.clear();
        m_frequentEntries.reserve(m_recentEntries.size());
        for (const auto& e : m_recentEntries) {
            if (e.accessCount > 1) {
                m_frequentEntries.push_back(e);
            }
        }
        std::sort(m_frequentEntries.begin(), m_frequentEntries.end(),
            [](const JumpListEntry& a, const JumpListEntry& b) {
                return a.accessCount > b.accessCount;
            });
        if (m_frequentEntries.size() > MAX_FREQUENT_ENTRIES) {
            m_frequentEntries.resize(MAX_FREQUENT_ENTRIES);
        }
    }

    void AddDefaultTasksLocked() {
        // "Open ExplorerLens Manager" task
        JumpListTask managerTask;
        managerTask.title = L"Open ExplorerLens Manager";
        managerTask.description = L"Configure ExplorerLens settings and registered formats";
        managerTask.commandPath = L"LENSManager.exe";
        managerTask.iconIndex = 0;
        m_tasks.push_back(std::move(managerTask));

        // "Clear Thumbnail Cache" task
        JumpListTask clearCacheTask;
        clearCacheTask.title = L"Clear Thumbnail Cache";
        clearCacheTask.description = L"Remove all cached thumbnails to free disk space";
        clearCacheTask.commandPath = L"LENSManager.exe";
        clearCacheTask.arguments = L"/clearcache";
        clearCacheTask.iconIndex = 1;
        m_tasks.push_back(std::move(clearCacheTask));
    }

    void MaybeUpdateJumpListLocked() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastUpdateTime).count();
        if (elapsed >= MIN_UPDATE_INTERVAL_MS) {
            UpdateJumpListCOMLocked();
        }
    }

    /// Stub for COM ICustomDestinationList interaction
    /// In production, this calls CoCreateInstance(CLSID_DestinationList)
    bool UpdateJumpListCOMLocked() {
        m_lastUpdateTime = std::chrono::steady_clock::now();
        m_updateCount++;

        // Production implementation would:
        // 1. CoCreateInstance(CLSID_DestinationList, ...)
        // 2. BeginList() to get removed items
        // 3. Add "Recent" category via AppendKnownCategory(KDC_RECENT)
        // 4. Add "Frequent" custom category
        // 5. Add "Pinned" custom category
        // 6. Add Tasks category
        // 7. CommitList()

        return true;  // Stub — always succeeds
    }

    static uint64_t GetCurrentFileTime() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        // Convert to FILETIME (100ns intervals since 1601-01-01)
        return static_cast<uint64_t>((ns / 100) + 116444736000000000ULL);
    }

    // State
    mutable std::mutex m_mutex;
    bool m_initialized = false;
    std::wstring m_appUserModelId;

    // Entry lists
    std::deque<JumpListEntry>  m_recentEntries;
    std::vector<JumpListEntry> m_frequentEntries;
    std::vector<JumpListEntry> m_pinnedEntries;
    std::vector<JumpListTask>  m_tasks;

    // Throttling
    std::chrono::steady_clock::time_point m_lastUpdateTime;
    uint32_t m_updateCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
