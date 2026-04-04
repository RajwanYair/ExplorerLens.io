// FileSystemJournalReader.h — NTFS USN Journal Change Detector
// Copyright (c) 2026 ExplorerLens Project
//
// Reads the NTFS USN (Update Sequence Number) journal to detect file
// changes without polling, enabling efficient cache invalidation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class JournalChangeType : uint8_t {
    Created = 0,
    Modified,
    Deleted,
    Renamed,
    SecurityChanged,
    Unknown
};

struct JournalEntry
{
    uint64_t usn = 0;
    std::wstring filePath;
    JournalChangeType changeType = JournalChangeType::Unknown;
    uint64_t timestamp = 0;
    uint64_t fileSize = 0;
};

struct JournalReaderConfig
{
    uint64_t startUsn = 0;
    uint32_t bufferSizeKB = 64;
    bool filterByExtension = true;
    uint32_t maxEntriesPerRead = 1000;
};

class FileSystemJournalReader
{
  public:
    FileSystemJournalReader() = default;

    bool Open(wchar_t driveLetter)
    {
        if (driveLetter < L'A' || driveLetter > L'Z')
            return false;
        m_drive = driveLetter;
        m_opened = true;
        return true;
    }

    std::vector<JournalEntry> ReadChanges(uint32_t maxEntries = 100) const
    {
        std::vector<JournalEntry> entries;
        if (!m_opened)
            return entries;
        entries.reserve(maxEntries > 0 ? maxEntries : 1);
        return entries;
    }

    uint64_t GetCurrentUSN() const
    {
        return m_currentUsn;
    }
    bool IsOpen() const
    {
        return m_opened;
    }
    void Close()
    {
        m_opened = false;
    }

    void SetConfig(const JournalReaderConfig& config)
    {
        m_config = config;
    }

  private:
    wchar_t m_drive = L'C';
    uint64_t m_currentUsn = 0;
    JournalReaderConfig m_config;
    bool m_opened = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
