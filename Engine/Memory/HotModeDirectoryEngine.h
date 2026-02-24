#pragma once
// Hot-Mode Directory Engine
// Background index, change notification watcher, pre-warm thumbnail generation
// for directories with > 50 images (Explorer hot paths).

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens::Memory {

// ─── Hot-mode thresholds ─────────────────────────────────────────────────────

struct HotModeThresholds {
  uint32_t minFilesForHotMode{50};  // directory qualifies at >= 50 images
  uint32_t maxConcurrentPreWarm{4}; // pre-warm worker count
  uint32_t prewarmBatchSize{16};    // files per pre-warm batch
  uint32_t indexRefreshSecs{30};    // re-index if directory is older than this
  size_t vramBudgetBytes{128ULL * 1024 * 1024}; // 128 MB hot-mode budget

  static HotModeThresholds Default() { return {}; }

  static HotModeThresholds AggressiveLowPower() {
    HotModeThresholds t;
    t.minFilesForHotMode = 100;
    t.maxConcurrentPreWarm = 2;
    t.prewarmBatchSize = 8;
    return t;
  }
};

// ─── Directory index entry ───────────────────────────────────────────────────

struct DirectoryIndexEntry {
  std::string filePath;
  uint64_t sizeBytes{0};
  uint64_t lastModified{0}; // FILETIME epoch
  bool thumbnailCached{false};
  bool prewarmPending{false};
};

// ─── Directory snapshot
// ───────────────────────────────────────────────────────

struct DirectorySnapshot {
  std::string directoryPath;
  std::vector<DirectoryIndexEntry> entries;
  uint64_t indexTimestamp{0};
  bool hotMode{false};

  uint32_t TotalFiles() const { return static_cast<uint32_t>(entries.size()); }
  uint32_t CachedCount() const {
    uint32_t n = 0;
    for (const auto &e : entries)
      if (e.thumbnailCached)
        ++n;
    return n;
  }

  double CacheHitRate() const {
    return TotalFiles() > 0 ? 100.0 * CachedCount() / TotalFiles() : 0.0;
  }
};

// ─── Change notification ─────────────────────────────────────────────────────

enum class DirChangeType : uint32_t {
  FileAdded = 0,
  FileRemoved = 1,
  FileChanged = 2,
  DirRenamed = 3,
};

struct DirChangeEvent {
  DirChangeType type;
  std::string path;
  uint64_t timestamp{0};
};

// ─── Pre-warm result ─────────────────────────────────────────────────────────

struct PreWarmBatchResult {
  uint32_t filesAttempted{0};
  uint32_t filesSucceeded{0};
  uint32_t filesFailed{0};
  double totalMs{0.0};
  size_t bytesUsed{0};

  double AvgMsPerFile() const {
    return filesAttempted > 0 ? totalMs / filesAttempted : 0.0;
  }
};

// ─── Hot-mode directory engine ───────────────────────────────────────────────

class HotModeDirectoryEngine {
public:
  using ChangeCallback = std::function<void(const DirChangeEvent &)>;

  explicit HotModeDirectoryEngine(
      HotModeThresholds thresholds = HotModeThresholds::Default())
      : m_thresholds(std::move(thresholds)) {}

  bool IsHotModeDirectory(const DirectorySnapshot &snap) const {
    return snap.TotalFiles() >= m_thresholds.minFilesForHotMode;
  }

  DirectorySnapshot IndexDirectory(const std::string &path) const {
    DirectorySnapshot snap;
    snap.directoryPath = path;

    // Convert to wide string for Win32 API
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    std::wstring wpath(static_cast<size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath.data(), wlen);

    // Ensure trailing backslash for search pattern
    if (!wpath.empty() && wpath.back() == L'\0')
      wpath.pop_back();
    if (!wpath.empty() && wpath.back() != L'\\')
      wpath += L'\\';
    std::wstring searchPattern = wpath + L"*";

    // Use FindFirstFileExW with FindExInfoBasic for best performance
    WIN32_FIND_DATAW findData{};
    HANDLE hFind =
        FindFirstFileExW(searchPattern.c_str(),
                         FindExInfoBasic, // Skip short names for speed
                         &findData, FindExSearchNameMatch, nullptr,
                         FIND_FIRST_EX_LARGE_FETCH // Batch fetch optimization
        );

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        // Skip . and ..
        if (findData.cFileName[0] == L'.' &&
            (findData.cFileName[1] == L'\0' ||
             (findData.cFileName[1] == L'.' && findData.cFileName[2] == L'\0')))
          continue;

        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          continue;

        DirectoryIndexEntry entry;
        // Convert filename back to UTF-8
        int mblen = WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1,
                                        nullptr, 0, nullptr, nullptr);
        std::string fileName(static_cast<size_t>(mblen), '\0');
        WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1, fileName.data(),
                            mblen, nullptr, nullptr);
        if (!fileName.empty() && fileName.back() == '\0')
          fileName.pop_back();

        entry.filePath = path + "\\" + fileName;
        entry.sizeBytes =
            (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) |
            findData.nFileSizeLow;
        entry.lastModified =
            (static_cast<uint64_t>(findData.ftLastWriteTime.dwHighDateTime)
             << 32) |
            findData.ftLastWriteTime.dwLowDateTime;
        entry.thumbnailCached = false;
        entry.prewarmPending = true;

        snap.entries.push_back(std::move(entry));
      } while (FindNextFileW(hFind, &findData));
      FindClose(hFind);
    }

    // Sort by filename for deterministic ordering
    std::sort(snap.entries.begin(), snap.entries.end(),
              [](const DirectoryIndexEntry &a, const DirectoryIndexEntry &b) {
                return a.filePath < b.filePath;
              });

    // Set index timestamp (FILETIME now)
    FILETIME ft{};
    GetSystemTimeAsFileTime(&ft);
    snap.indexTimestamp =
        (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    snap.hotMode = IsHotModeDirectory(snap);
    return snap;
  }

  PreWarmBatchResult PreWarmBatch(DirectorySnapshot &snap, uint32_t startIndex,
                                  uint32_t count) const {
    PreWarmBatchResult result;
    uint32_t end = startIndex + count;
    if (end > snap.TotalFiles())
      end = snap.TotalFiles();

    for (uint32_t i = startIndex; i < end; ++i) {
      ++result.filesAttempted;
      // stub: real impl calls ExplorerLensEngine::DecodeThumbnail
      snap.entries[i].thumbnailCached = true;
      snap.entries[i].prewarmPending = false;
      result.bytesUsed += 4 * 256 * 256; // 256 KB per thumbnail BGRA estimate
      ++result.filesSucceeded;
    }
    result.totalMs = result.filesAttempted * 5.0; // 5 ms/file estimate
    return result;
  }

  void RegisterChangeCallback(ChangeCallback cb) {
    m_changeCallback = std::move(cb);
  }

  const HotModeThresholds &Thresholds() const { return m_thresholds; }

private:
  HotModeThresholds m_thresholds;
  ChangeCallback m_changeCallback;
};

} // namespace ExplorerLens::Memory
