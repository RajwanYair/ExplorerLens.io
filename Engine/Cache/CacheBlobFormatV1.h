//==============================================================================
// ExplorerLens Engine — Cache blob format v1 (Sprint S237)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 ADR-022: structured cache blob v1 — versioned header enabling
//                       zero-copy mmap reads and forward-compatible migration
//==============================================================================
//
// Every cached thumbnail stored on disk (SQLite blob column or future raw
// file) begins with a CacheBlobHeaderV1. Loaders validate magic + version
// before touching the pixel payload. Mismatches are treated as a cache miss.
//
// On-disk layout (little-endian):
//
//   offset  size  field
//   ------  ----  -----
//   0       4     magic      = 'L','E','N','B'   (0x424E454C)
//   4       1     version    = 1
//   5       1     pixelFmt   — 0=BGRA8, 1=RGBA16F (future)
//   6       2     flags
//   8       4     width
//   12      4     height
//   16      4     stride     (bytes per row)
//   20      4     payloadLen (bytes of compressed pixel data that follow)
//   24      8     decodeEpochNs (time_since_epoch ns — for debugging)
//   32      -     payload ...
//==============================================================================
#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {
namespace Cache {

inline constexpr std::uint32_t kCacheBlobMagic   = 0x424E454CU; // "LENB"
inline constexpr std::uint8_t  kCacheBlobVersion = 1;

enum class CacheBlobPixelFormat : std::uint8_t
{
    BGRA8    = 0,
    RGBA16F  = 1,
    P010     = 2,
    Reserved = 0xFF,
};

enum class CacheBlobFlags : std::uint16_t
{
    None            = 0,
    Compressed      = 1 << 0,  // payload is LZ4-compressed
    Premultiplied   = 1 << 1,
    HDR             = 1 << 2,
    GeneratedByGPU  = 1 << 3,
};

#pragma pack(push, 1)
struct CacheBlobHeaderV1
{
    std::uint32_t        magic         = kCacheBlobMagic;
    std::uint8_t         version       = kCacheBlobVersion;
    std::uint8_t         pixelFormat   = 0;
    std::uint16_t        flags         = 0;
    std::uint32_t        width         = 0;
    std::uint32_t        height        = 0;
    std::uint32_t        stride        = 0;
    std::uint32_t        payloadLength = 0;
    std::uint64_t        decodeEpochNs = 0;
};
#pragma pack(pop)

static_assert(sizeof(CacheBlobHeaderV1) == 32,
    "CacheBlobHeaderV1 must be exactly 32 bytes for stable on-disk layout.");
static_assert(std::is_trivially_copyable_v<CacheBlobHeaderV1>,
    "CacheBlobHeaderV1 must be trivially copyable for mmap zero-copy.");

/// <summary>
/// Validate a header read from disk. Returns true if safe to decode the
/// payload that follows.
/// </summary>
[[nodiscard]] inline bool IsValidCacheBlobHeader(const CacheBlobHeaderV1& h) noexcept
{
    if (h.magic != kCacheBlobMagic)       return false;
    if (h.version != kCacheBlobVersion)   return false;
    if (h.width == 0 || h.height == 0)    return false;
    if (h.stride < h.width)               return false;
    if (h.payloadLength == 0)             return false;
    // Cap at 32 MB — larger is suspicious for a thumbnail
    if (h.payloadLength > 32u * 1024u * 1024u) return false;
    return true;
}

/// <summary>
/// Populate a header for a BGRA8 thumbnail before writing to cache.
/// </summary>
[[nodiscard]] inline CacheBlobHeaderV1 MakeBgra8Header(std::uint32_t w,
                                                      std::uint32_t h,
                                                      std::uint32_t payloadLen,
                                                      std::uint64_t epochNs,
                                                      bool compressed = false) noexcept
{
    CacheBlobHeaderV1 hdr{};
    hdr.magic         = kCacheBlobMagic;
    hdr.version       = kCacheBlobVersion;
    hdr.pixelFormat   = static_cast<std::uint8_t>(CacheBlobPixelFormat::BGRA8);
    hdr.flags         = compressed
                            ? static_cast<std::uint16_t>(CacheBlobFlags::Compressed)
                            : 0;
    hdr.width         = w;
    hdr.height        = h;
    hdr.stride        = w * 4u;
    hdr.payloadLength = payloadLen;
    hdr.decodeEpochNs = epochNs;
    return hdr;
}

} // namespace Cache
} // namespace Engine
} // namespace ExplorerLens
