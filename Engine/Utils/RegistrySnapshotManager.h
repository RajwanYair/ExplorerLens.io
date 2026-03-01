#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// RegistrySnapshotManager — Atomic registry backup/restore
// ============================================================================

enum class SnapshotScope {
    CurrentUser,
    LocalMachine,
    ClassesRoot,
    All
};

inline const char* SnapshotScopeName(SnapshotScope value) {
    switch (value) {
    case SnapshotScope::CurrentUser:   return "CurrentUser";
    case SnapshotScope::LocalMachine:  return "LocalMachine";
    case SnapshotScope::ClassesRoot:   return "ClassesRoot";
    case SnapshotScope::All:           return "All";
    default:                           return "Unknown";
    }
}

enum class SnapshotAction {
    Backup,
    Restore,
    Compare,
    Delete
};

inline const char* SnapshotActionName(SnapshotAction value) {
    switch (value) {
    case SnapshotAction::Backup:  return "Backup";
    case SnapshotAction::Restore: return "Restore";
    case SnapshotAction::Compare: return "Compare";
    case SnapshotAction::Delete:  return "Delete";
    default:                      return "Unknown";
    }
}

struct RegistrySnapshot {
    SnapshotScope scope = SnapshotScope::CurrentUser;
    uint64_t      timestampMs = 0;
    uint32_t      keyCount = 0;
    std::wstring  filePath;
    uint64_t      sizeBytes = 0;
    std::string   description;
    uint32_t      snapshotId = 0;

    bool IsValid() const {
        return keyCount > 0 && sizeBytes > 0 && !filePath.empty();
    }
};

struct SnapshotComparisonResult {
    uint32_t addedKeys = 0;
    uint32_t removedKeys = 0;
    uint32_t modifiedKeys = 0;
    bool     identical = true;

    uint32_t GetTotalChanges() const {
        return addedKeys + removedKeys + modifiedKeys;
    }
};

class RegistrySnapshotManager {
public:
    static constexpr uint32_t MAX_SNAPSHOTS = 10;
    static constexpr uint64_t MAX_SNAPSHOT_SIZE = 50 * 1024 * 1024; // 50 MB
    static constexpr uint32_t SNAPSHOT_VERSION = 2;

    RegistrySnapshotManager() = default;
    ~RegistrySnapshotManager() = default;

    RegistrySnapshotManager(const RegistrySnapshotManager&) = delete;
    RegistrySnapshotManager& operator=(const RegistrySnapshotManager&) = delete;

    bool CreateSnapshot(SnapshotScope scope, const std::wstring& outputPath,
        const std::string& description = "") {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_snapshots.size() >= MAX_SNAPSHOTS) {
            // Remove oldest snapshot
            auto oldest = std::min_element(m_snapshots.begin(), m_snapshots.end(),
                [](const RegistrySnapshot& a, const RegistrySnapshot& b) {
                    return a.timestampMs < b.timestampMs;
                });
            if (oldest != m_snapshots.end()) {
                m_snapshots.erase(oldest);
            }
        }

        RegistrySnapshot snapshot;
        snapshot.scope = scope;
        snapshot.filePath = outputPath;
        snapshot.timestampMs = GetCurrentTimeMs();
        snapshot.description = description;
        snapshot.snapshotId = m_nextSnapshotId++;

        // In production: use RegSaveKeyExW to export registry hive
        // Simulate key counts per scope
        switch (scope) {
        case SnapshotScope::CurrentUser:  snapshot.keyCount = 150; snapshot.sizeBytes = 32768; break;
        case SnapshotScope::LocalMachine: snapshot.keyCount = 80;  snapshot.sizeBytes = 16384; break;
        case SnapshotScope::ClassesRoot:  snapshot.keyCount = 200; snapshot.sizeBytes = 65536; break;
        case SnapshotScope::All:          snapshot.keyCount = 430; snapshot.sizeBytes = 114688; break;
        }

        m_snapshots.push_back(snapshot);
        m_totalCreated++;
        return true;
    }

    bool RestoreSnapshot(uint32_t snapshotId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = FindSnapshotById(snapshotId);
        if (it == m_snapshots.end()) {
            return false;
        }

        // In production: use RegRestoreKeyW to import registry hive
        m_lastRestoredId = snapshotId;
        m_totalRestored++;
        return true;
    }

    SnapshotComparisonResult CompareSnapshots(uint32_t snapshotIdA, uint32_t snapshotIdB) {
        std::lock_guard<std::mutex> lock(m_mutex);

        SnapshotComparisonResult result;

        auto itA = FindSnapshotById(snapshotIdA);
        auto itB = FindSnapshotById(snapshotIdB);

        if (itA == m_snapshots.end() || itB == m_snapshots.end()) {
            return result;
        }

        // Simulated comparison
        if (itA->keyCount != itB->keyCount) {
            result.identical = false;
            if (itA->keyCount > itB->keyCount) {
                result.removedKeys = itA->keyCount - itB->keyCount;
            }
            else {
                result.addedKeys = itB->keyCount - itA->keyCount;
            }
        }

        if (itA->sizeBytes != itB->sizeBytes) {
            result.identical = false;
            result.modifiedKeys = 5; // Simulated modification count
        }

        return result;
    }

    bool DeleteSnapshot(uint32_t snapshotId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = FindSnapshotById(snapshotId);
        if (it == m_snapshots.end()) {
            return false;
        }

        m_snapshots.erase(it);
        return true;
    }

    size_t GetSnapshotCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_snapshots.size();
    }

    uint64_t GetTotalCreated() const { return m_totalCreated; }
    uint64_t GetTotalRestored() const { return m_totalRestored; }
    uint32_t GetLastRestoredId() const { return m_lastRestoredId; }

private:
    std::vector<RegistrySnapshot>::iterator FindSnapshotById(uint32_t id) {
        return std::find_if(m_snapshots.begin(), m_snapshots.end(),
            [id](const RegistrySnapshot& s) { return s.snapshotId == id; });
    }

    uint64_t GetCurrentTimeMs() const {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    mutable std::mutex                 m_mutex;
    std::vector<RegistrySnapshot>      m_snapshots;
    uint32_t                           m_nextSnapshotId = 1;
    uint64_t                           m_totalCreated = 0;
    uint64_t                           m_totalRestored = 0;
    uint32_t                           m_lastRestoredId = 0;
};

} // namespace Engine
} // namespace ExplorerLens
