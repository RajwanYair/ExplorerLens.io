// ArchivePeekDecoder.h — Archive Content Preview Without Full Extract
// Copyright (c) 2026 ExplorerLens Project
//
// Reads archive metadata and first few entries to generate a preview
// thumbnail without extracting all contents, optimizing for large archives.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ArchivePeekFormat : uint8_t {
    ZIP = 0,
    RAR = 1,
    SevenZip = 2,
    TAR = 3,
    TarGz = 4,
    TarBz2 = 5,
    TarXz = 6,
    Unknown = 255
};

struct ArchiveEntry
{
    std::wstring name;
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    bool isImageFile = false;
    bool isDirectory = false;
    float priorityScore = 0.0f;
};

struct ArchivePeekResult
{
    bool success = false;
    ArchivePeekFormat format = ArchivePeekFormat::Unknown;
    uint32_t totalEntries = 0;
    uint32_t imageEntries = 0;
    uint64_t totalUncompressedSize = 0;
    std::vector<ArchiveEntry> topEntries;  // Best candidates for thumbnail
    double peekTimeMs = 0.0;
};

struct ArchivePeekConfig
{
    uint32_t maxEntriesToScan = 100;
    uint32_t maxImageCandidates = 4;
    uint64_t maxEntrySize = 50ULL * 1024 * 1024;  // Skip files > 50MB
    bool preferFirstImage = true;
    bool preferLargestImage = false;
};

class ArchivePeekDecoder
{
  public:
    void Configure(const ArchivePeekConfig& config)
    {
        m_config = config;
    }

    bool IsImageExtension(const std::wstring& name) const
    {
        auto ext = GetExtension(name);
        return ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".bmp" || ext == L".gif"
               || ext == L".webp" || ext == L".tif" || ext == L".tiff";
    }

    float ScoreEntry(const ArchiveEntry& entry) const
    {
        float score = 0.0f;
        if (entry.isImageFile)
            score += 10.0f;
        if (entry.isDirectory)
            return 0.0f;
        if (entry.uncompressedSize > m_config.maxEntrySize)
            return 0.0f;
        // Prefer named images (cover, thumb, preview)
        std::wstring lower = entry.name;
        for (auto& c : lower)
            c = towlower(c);
        if (lower.find(L"cover") != std::wstring::npos)
            score += 5.0f;
        if (lower.find(L"thumb") != std::wstring::npos)
            score += 3.0f;
        if (lower.find(L"preview") != std::wstring::npos)
            score += 3.0f;
        // Prefer reasonable sizes
        if (entry.uncompressedSize > 10000 && entry.uncompressedSize < 10000000)
            score += 2.0f;
        return score;
    }

    ArchivePeekConfig GetConfig() const
    {
        return m_config;
    }

  private:
    static std::wstring GetExtension(const std::wstring& path)
    {
        auto pos = path.find_last_of(L'.');
        if (pos == std::wstring::npos)
            return {};
        std::wstring ext = path.substr(pos);
        for (auto& c : ext)
            c = towlower(c);
        return ext;
    }

    ArchivePeekConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
