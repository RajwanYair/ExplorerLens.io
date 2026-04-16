// ArchiveCoverExtractor.cpp — First-image cover extraction via libarchive
// Copyright (c) 2026 ExplorerLens Project
//
#include "ArchiveCoverExtractor.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#if __has_include(<archive.h>)
#  include <archive.h>
#  include <archive_entry.h>
#  define HAVE_LIBARCHIVE 1
#else
#  define HAVE_LIBARCHIVE 0
#endif

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string ToLower(std::string_view s)
{
    std::string r(s);
    for (char& c : r) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return r;
}

bool ArchiveCoverExtractor::IsImageEntry(std::string_view name)
{
    std::string lo = ToLower(name);
    return lo.ends_with(".jpg")  || lo.ends_with(".jpeg") ||
           lo.ends_with(".png")  || lo.ends_with(".webp") ||
           lo.ends_with(".gif")  || lo.ends_with(".bmp")  ||
           lo.ends_with(".tiff") || lo.ends_with(".tif")  ||
           lo.ends_with(".avif") || lo.ends_with(".heic") ||
           lo.ends_with(".jxl");
}

std::string ArchiveCoverExtractor::InferMimeType(std::string_view name)
{
    std::string lo = ToLower(name);
    if (lo.ends_with(".jpg") || lo.ends_with(".jpeg")) return "image/jpeg";
    if (lo.ends_with(".png"))  return "image/png";
    if (lo.ends_with(".webp")) return "image/webp";
    if (lo.ends_with(".gif"))  return "image/gif";
    if (lo.ends_with(".bmp"))  return "image/bmp";
    if (lo.ends_with(".avif")) return "image/avif";
    if (lo.ends_with(".heic")) return "image/heic";
    if (lo.ends_with(".jxl"))  return "image/jxl";
    return "application/octet-stream";
}

// ---------------------------------------------------------------------------
// libarchive implementation
// ---------------------------------------------------------------------------

#if HAVE_LIBARCHIVE

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::Extract(
    std::string_view filePath) const
{
    std::string fp(filePath);
    archive* a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (archive_read_open_filename(a, fp.c_str(), 65536) != ARCHIVE_OK) {
        archive_read_free(a);
        return std::nullopt;
    }

    auto result = ExtractFirstImage(a);
    archive_read_free(a);
    return result;
}

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::ExtractFromBuffer(
    const uint8_t* data, size_t size, std::string_view) const
{
    archive* a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (archive_read_open_memory(a, data, size) != ARCHIVE_OK) {
        archive_read_free(a);
        return std::nullopt;
    }

    auto result = ExtractFirstImage(a);
    archive_read_free(a);
    return result;
}

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::ExtractFirstImage(
    void* handle) const
{
    archive* a = static_cast<archive*>(handle);

    // Collect all image entries, sort, take first
    struct Candidate {
        std::string name;
        int64_t     offset; // archive offset (unused for streaming; we do two-pass if needed)
        int64_t     size;
    };

    std::vector<Candidate> candidates;
    archive_entry* ent = nullptr;
    while (archive_read_next_header(a, &ent) == ARCHIVE_OK) {
        const char* pname = archive_entry_pathname(ent);
        if (!pname) { archive_read_data_skip(a); continue; }

        std::string entName(pname);
        if (!IsImageEntry(entName)) { archive_read_data_skip(a); continue; }

        int64_t sz = archive_entry_size(ent);
        if (sz <= 0 || static_cast<size_t>(sz) > m_maxImageBytes) {
            archive_read_data_skip(a);
            continue;
        }

        // Read data immediately (streaming archive — can't seek back)
        std::vector<uint8_t> buf(static_cast<size_t>(sz));
        ssize_t r = archive_read_data(a, buf.data(), buf.size());
        if (r < 0) continue;
        buf.resize(static_cast<size_t>(r));

        candidates.push_back({ entName, 0, sz });

        // For streaming: take the first lexicographic match we've already read.
        // We keep reading to find candidates but store data only for the current.
        // Simplified: on first valid image we return immediately.
        // (A proper implementation would do two passes; this is the streaming fast path.)
        ArchiveCoverResult res;
        res.imageData  = std::move(buf);
        res.entryName  = entName;
        res.mimeType   = InferMimeType(entName);
        return res;
    }

    return std::nullopt;
}

#else // !HAVE_LIBARCHIVE — stubs

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::Extract(
    std::string_view) const { return std::nullopt; }

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::ExtractFromBuffer(
    const uint8_t*, size_t, std::string_view) const { return std::nullopt; }

std::optional<ArchiveCoverResult> ArchiveCoverExtractor::ExtractFirstImage(
    void*) const { return std::nullopt; }

#endif // HAVE_LIBARCHIVE

} // namespace Engine
} // namespace ExplorerLens
