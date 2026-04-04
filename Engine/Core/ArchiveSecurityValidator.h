// ArchiveSecurityValidator.h — ZIP / RAR / 7z / TAR Hardening
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 14 (v15.3.0 "Zenith-T"): Provides pre-decode security checks for all
// archive decoders.  Must be called before any archive entry is enumerated or
// extracted.  Detects ZIP bombs (compression ratio attack), path traversal
// sequences, and symlink attacks.
//
#pragma once

#include <cstdint>
#include <string_view>
#include "DecodeErrorCategory.h"

namespace ExplorerLens {
namespace Engine {

class ArchiveSecurityValidator
{
  public:
    static constexpr uint64_t MAX_UNCOMPRESSED_RATIO = 100u;
    static constexpr uint64_t MAX_UNCOMPRESSED_BYTES = 2ull * 1024 * 1024 * 1024;
    static constexpr uint64_t MAX_ENTRY_COUNT = 65536u;

    static DecodeErrorCategory CheckCompressionRatio(uint64_t compressedBytes, uint64_t uncompressedBytes) noexcept
    {
        if (compressedBytes == 0)
            return DecodeErrorCategory::CorruptedData;
        if (uncompressedBytes > MAX_UNCOMPRESSED_BYTES)
            return DecodeErrorCategory::ZipBombDetected;
        if (uncompressedBytes / compressedBytes > MAX_UNCOMPRESSED_RATIO)
            return DecodeErrorCategory::ZipBombDetected;
        return DecodeErrorCategory::None;
    }

    static DecodeErrorCategory CheckEntryPath(std::string_view entryPath) noexcept
    {
        if (entryPath.empty())
            return DecodeErrorCategory::CorruptedData;

        if (entryPath.find("..") != std::string_view::npos)
            return DecodeErrorCategory::PathTraversalDetected;

        if (entryPath.front() == '/' || entryPath.front() == '\\')
            return DecodeErrorCategory::PathTraversalDetected;

        if (entryPath.size() >= 2 && entryPath[1] == ':')
            return DecodeErrorCategory::PathTraversalDetected;

        return DecodeErrorCategory::None;
    }

    static DecodeErrorCategory CheckEntryCount(uint64_t entryCount) noexcept
    {
        if (entryCount > MAX_ENTRY_COUNT)
            return DecodeErrorCategory::ZipBombDetected;
        return DecodeErrorCategory::None;
    }

    static DecodeErrorCategory CheckSymlink(bool isSymlink) noexcept
    {
        if (isSymlink)
            return DecodeErrorCategory::SymlinkAttackDetected;
        return DecodeErrorCategory::None;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
