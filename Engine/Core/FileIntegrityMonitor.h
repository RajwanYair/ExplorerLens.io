#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// FileIntegrityMonitor — NTFS USN journal monitoring for file change detection
// ============================================================================

enum class FileIntegrityCheckType {
    Hash,
    Timestamp,
    Attribute,
    FullScan,
    Quick
};

inline const char* FileIntegrityCheckTypeName(FileIntegrityCheckType value) {
    switch (value) {
    case FileIntegrityCheckType::Hash:      return "Hash";
    case FileIntegrityCheckType::Timestamp: return "Timestamp";
    case FileIntegrityCheckType::Attribute: return "Attribute";
    case FileIntegrityCheckType::FullScan:  return "FullScan";
    case FileIntegrityCheckType::Quick:     return "Quick";
    default:                            return "Unknown";
    }
}

enum class FileIntegrityStatus {
    Valid,
    Modified,
    Corrupted,
    Missing,
    Quarantined
};

inline const char* FileIntegrityStatusName(FileIntegrityStatus value) {
    switch (value) {
    case FileIntegrityStatus::Valid:       return "Valid";
    case FileIntegrityStatus::Modified:    return "Modified";
    case FileIntegrityStatus::Corrupted:   return "Corrupted";
    case FileIntegrityStatus::Missing:     return "Missing";
    case FileIntegrityStatus::Quarantined: return "Quarantined";
    default:                           return "Unknown";
    }
}

struct FileIntegrityRecord {
    std::wstring filePath;
    uint64_t     lastHash = 0;
    uint64_t     lastCheckMs = 0;
    FileIntegrityStatus status = FileIntegrityStatus::Valid;
    uint64_t     fileSizeBytes = 0;
    FileIntegrityCheckType lastCheckType = FileIntegrityCheckType::Quick;

    bool IsHealthy() const {
        return status == FileIntegrityStatus::Valid;
    }
};

class FileIntegrityMonitor {
public:
    static constexpr uint32_t MAX_RECORDS = 100000;
    static constexpr uint32_t HASH_SEED = 0x45584C4E; // "EXLN"
    static constexpr uint32_t JOURNAL_BUFFER_SIZE = 65536;

    FileIntegrityMonitor() = default;
    ~FileIntegrityMonitor() = default;

    FileIntegrityMonitor(const FileIntegrityMonitor&) = delete;
    FileIntegrityMonitor& operator=(const FileIntegrityMonitor&) = delete;

    FileIntegrityStatus CheckFile(const std::wstring& path, FileIntegrityCheckType checkType) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_records.find(path);
        if (it == m_records.end()) {
            FileIntegrityRecord record;
            record.filePath = path;
            record.status = FileIntegrityStatus::Valid;
            record.lastCheckType = checkType;
            record.lastCheckMs = GetCurrentTimeMs();

            if (checkType == FileIntegrityCheckType::Hash || checkType == FileIntegrityCheckType::FullScan) {
                record.lastHash = ComputeSimpleHash(path);
            }

            m_records[path] = record;
            m_totalChecks++;
            return FileIntegrityStatus::Valid;
        }

        FileIntegrityRecord& record = it->second;
        record.lastCheckType = checkType;
        record.lastCheckMs = GetCurrentTimeMs();

        if (checkType == FileIntegrityCheckType::Hash || checkType == FileIntegrityCheckType::FullScan) {
            uint64_t newHash = ComputeSimpleHash(path);
            if (newHash != record.lastHash && record.lastHash != 0) {
                record.status = FileIntegrityStatus::Modified;
            }
            record.lastHash = newHash;
        }

        m_totalChecks++;
        return record.status;
    }

    /// Enumerate all files in the given directory and register/check each one.
    /// Returns the number of files discovered and checked.
    uint32_t ScanDirectory(const std::wstring& directoryPath, FileIntegrityCheckType checkType) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lastScanDirectory = directoryPath;
        m_lastScanType = checkType;
        m_scanCount++;

        // Build search pattern: directoryPath\*
        std::wstring searchPattern = directoryPath;
        if (!searchPattern.empty() && searchPattern.back() != L'\\')
            searchPattern += L'\\';
        searchPattern += L"*";

        WIN32_FIND_DATAW findData{};
        HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE)
            return 0;

        uint32_t fileCount = 0;
        do {
            // Skip "." and ".." directory entries
            if (findData.cFileName[0] == L'.' &&
                (findData.cFileName[1] == L'\0' ||
                 (findData.cFileName[1] == L'.' && findData.cFileName[2] == L'\0')))
                continue;

            // Skip subdirectories — only process regular files
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            std::wstring filePath = directoryPath;
            if (!filePath.empty() && filePath.back() != L'\\')
                filePath += L'\\';
            filePath += findData.cFileName;

            // Register or update each discovered file
            auto it = m_records.find(filePath);
            if (it == m_records.end()) {
                FileIntegrityRecord record{};
                record.filePath = filePath;
                record.status = FileIntegrityStatus::Valid;
                record.lastCheckType = checkType;
                record.lastCheckMs = GetCurrentTimeMs();
                record.fileSizeBytes =
                    (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) |
                    findData.nFileSizeLow;
                if (checkType == FileIntegrityCheckType::Hash ||
                    checkType == FileIntegrityCheckType::FullScan) {
                    record.lastHash = ComputeSimpleHash(filePath);
                }
                m_records[filePath] = record;
            } else {
                it->second.lastCheckType = checkType;
                it->second.lastCheckMs = GetCurrentTimeMs();
                it->second.fileSizeBytes =
                    (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) |
                    findData.nFileSizeLow;
            }
            m_totalChecks++;
            fileCount++;
        } while (FindNextFileW(hFind, &findData));

        FindClose(hFind);
        return fileCount;
    }

    size_t GetRecordCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_records.size();
    }

    uint64_t GetTotalChecks() const { return m_totalChecks; }
    uint32_t GetScanCount() const { return m_scanCount; }

    bool RemoveRecord(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_records.erase(path) > 0;
    }

    void ClearAllRecords() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_records.clear();
    }

private:
    uint64_t ComputeSimpleHash(const std::wstring& path) const {
        uint64_t hash = HASH_SEED;
        for (wchar_t c : path) {
            hash = hash * 31 + static_cast<uint64_t>(c);
        }
        return hash;
    }

    uint64_t GetCurrentTimeMs() const {
        auto now = std::chrono::steady_clock::now();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    mutable std::mutex                              m_mutex;
    std::unordered_map<std::wstring, FileIntegrityRecord> m_records;
    uint64_t                                        m_totalChecks = 0;
    uint32_t                                        m_scanCount = 0;
    std::wstring                                    m_lastScanDirectory;
    FileIntegrityCheckType                              m_lastScanType = FileIntegrityCheckType::Quick;
};

} // namespace Engine
} // namespace ExplorerLens
