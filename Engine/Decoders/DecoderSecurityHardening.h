// DecoderSecurityHardening.h — Centralized Security Validation for Decoders
// Copyright (c) 2026 ExplorerLens Project
//
// Provides safe integer arithmetic, dimension validation, file size limits,
// buffer bounds checking, and magic number validation utilities for all
// decoder implementations. Sprint 31-32 hardening.
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {
namespace Security {

// ============================================================================
// Safe Integer Arithmetic
// ============================================================================

/// Check if a * b would overflow uint32_t. Returns true if safe.
inline bool SafeMul32(uint32_t a, uint32_t b, uint32_t& result) noexcept {
    if (a == 0 || b == 0) { result = 0; return true; }
    if (a > (std::numeric_limits<uint32_t>::max)() / b) return false;
    result = a * b;
    return true;
}

/// Check if a * b would overflow uint64_t. Returns true if safe.
inline bool SafeMul64(uint64_t a, uint64_t b, uint64_t& result) noexcept {
    if (a == 0 || b == 0) { result = 0; return true; }
    if (a > (std::numeric_limits<uint64_t>::max)() / b) return false;
    result = a * b;
    return true;
}

/// Check if a + b would overflow uint64_t. Returns true if safe.
inline bool SafeAdd64(uint64_t a, uint64_t b, uint64_t& result) noexcept {
    if (a > (std::numeric_limits<uint64_t>::max)() - b) return false;
    result = a + b;
    return true;
}

/// Check if a * b * c would overflow size_t. Returns true if safe.
inline bool SafeMulTriple(size_t a, size_t b, size_t c, size_t& result) noexcept {
    if (a == 0 || b == 0 || c == 0) { result = 0; return true; }
    if (a > (std::numeric_limits<size_t>::max)() / b) return false;
    size_t ab = a * b;
    if (ab > (std::numeric_limits<size_t>::max)() / c) return false;
    result = ab * c;
    return true;
}

/// Compute width * height * bytesPerPixel safely. Returns 0 on overflow.
inline size_t SafePixelBufferSize(uint32_t width, uint32_t height, uint32_t bytesPerPixel) noexcept {
    size_t result = 0;
    if (!SafeMulTriple(static_cast<size_t>(width), static_cast<size_t>(height),
                       static_cast<size_t>(bytesPerPixel), result)) {
        return 0;
    }
    return result;
}

// ============================================================================
// Dimension Validation
// ============================================================================

/// Maximum image dimension (65536 x 65536)
static constexpr uint32_t MAX_IMAGE_DIMENSION = 65536;

/// Default maximum image dimension for thumbnails (16384 x 16384)
static constexpr uint32_t MAX_THUMBNAIL_DIMENSION = 16384;

/// Maximum pixel count to prevent excessive memory allocation (256 megapixels)
static constexpr uint64_t MAX_PIXEL_COUNT = 256ULL * 1024 * 1024;

/// Maximum allocation size for a single decoded image buffer (2 GB)
static constexpr size_t MAX_DECODE_ALLOCATION = 2ULL * 1024 * 1024 * 1024;

/// Validate image dimensions against security limits.
/// Returns true if dimensions are valid and safe.
inline bool ValidateDimensions(uint32_t width, uint32_t height,
                               uint32_t maxDim = MAX_THUMBNAIL_DIMENSION) noexcept {
    if (width == 0 || height == 0) return false;
    if (width > maxDim || height > maxDim) return false;
    uint64_t pixels = static_cast<uint64_t>(width) * height;
    if (pixels > MAX_PIXEL_COUNT) return false;
    return true;
}

/// Validate that a pixel buffer allocation is safe.
/// Checks dimension limits AND overflow on width * height * bytesPerPixel.
inline bool ValidatePixelAllocation(uint32_t width, uint32_t height,
                                    uint32_t bytesPerPixel, size_t& outSize) noexcept {
    if (!ValidateDimensions(width, height)) return false;
    outSize = SafePixelBufferSize(width, height, bytesPerPixel);
    if (outSize == 0 || outSize > MAX_DECODE_ALLOCATION) return false;
    return true;
}

// ============================================================================
// File Size Validation
// ============================================================================

/// Default file size limits per category (bytes)
static constexpr size_t MAX_IMAGE_FILE_SIZE       = 2ULL * 1024 * 1024 * 1024; // 2 GB
static constexpr size_t MAX_TEXTURE_FILE_SIZE     = 1ULL * 1024 * 1024 * 1024; // 1 GB
static constexpr size_t MAX_ICON_FILE_SIZE        = 64ULL * 1024 * 1024;       // 64 MB
static constexpr size_t MAX_SIMPLE_FORMAT_FILE_SIZE = 512ULL * 1024 * 1024;    // 512 MB
static constexpr size_t MAX_HDR_FILE_SIZE         = 1ULL * 1024 * 1024 * 1024; // 1 GB

/// Validate file size against a maximum.
inline bool ValidateFileSize(size_t fileSize, size_t maxSize) noexcept {
    return fileSize > 0 && fileSize <= maxSize;
}

// ============================================================================
// Buffer Bounds Checking
// ============================================================================

/// Validate that [offset, offset + length) fits within bufferSize.
inline bool ValidateBufferAccess(size_t offset, size_t length, size_t bufferSize) noexcept {
    if (length == 0) return true;
    if (offset > bufferSize) return false;
    if (length > bufferSize - offset) return false;
    return true;
}

/// Validate that reading 'count' items of 'itemSize' bytes at 'offset' is safe.
inline bool ValidateBufferRead(size_t offset, size_t count, size_t itemSize,
                               size_t bufferSize) noexcept {
    size_t totalBytes = 0;
    if (!SafeMul64(count, itemSize, reinterpret_cast<uint64_t&>(totalBytes))) return false;
    return ValidateBufferAccess(offset, totalBytes, bufferSize);
}

// ============================================================================
// Magic Number / Signature Validation
// ============================================================================

/// Check magic bytes at the start of a buffer.
inline bool ValidateMagic(const uint8_t* data, size_t dataSize,
                          const void* magic, size_t magicSize) noexcept {
    if (!data || dataSize < magicSize) return false;
    return memcmp(data, magic, magicSize) == 0;
}

/// Check magic bytes at an arbitrary offset.
inline bool ValidateMagicAt(const uint8_t* data, size_t dataSize,
                            size_t offset, const void* magic, size_t magicSize) noexcept {
    if (!ValidateBufferAccess(offset, magicSize, dataSize)) return false;
    return memcmp(data + offset, magic, magicSize) == 0;
}

// ============================================================================
// Null Pointer Validation
// ============================================================================

/// Validate a pointer is not null.
template<typename T>
inline bool ValidatePtr(const T* ptr) noexcept {
    return ptr != nullptr;
}

/// Validate multiple pointers are not null.
template<typename T, typename... Args>
inline bool ValidatePtr(const T* ptr, const Args*... args) noexcept {
    return ptr != nullptr && ValidatePtr(args...);
}

// ============================================================================
// RLE Decompression Safety
// ============================================================================

/// Safe RLE state tracker to prevent buffer overruns during decompression.
struct SafeRLEReader {
    const uint8_t* src;
    const uint8_t* srcEnd;
    uint8_t* dst;
    uint8_t* dstEnd;
    bool overflow = false;

    SafeRLEReader(const uint8_t* s, size_t sLen, uint8_t* d, size_t dLen) noexcept
        : src(s), srcEnd(s + sLen), dst(d), dstEnd(d + dLen) {}

    bool CanReadSrc(size_t n) const noexcept { return src + n <= srcEnd; }
    bool CanWriteDst(size_t n) const noexcept { return dst + n <= dstEnd; }
    size_t SrcRemaining() const noexcept { return static_cast<size_t>(srcEnd - src); }
    size_t DstRemaining() const noexcept { return static_cast<size_t>(dstEnd - dst); }

    uint8_t ReadByte() noexcept {
        if (src >= srcEnd) { overflow = true; return 0; }
        return *src++;
    }

    void WriteByte(uint8_t val) noexcept {
        if (dst >= dstEnd) { overflow = true; return; }
        *dst++ = val;
    }

    void WriteRepeat(uint8_t val, uint32_t count) noexcept {
        uint32_t safeCount = (std::min)(count, static_cast<uint32_t>(DstRemaining()));
        if (safeCount < count) overflow = true;
        memset(dst, val, safeCount);
        dst += safeCount;
    }

    void CopyFromSrc(uint32_t count) noexcept {
        uint32_t safeSrc = (std::min)(count, static_cast<uint32_t>(SrcRemaining()));
        uint32_t safeDst = (std::min)(count, static_cast<uint32_t>(DstRemaining()));
        uint32_t safe = (std::min)(safeSrc, safeDst);
        if (safe < count) overflow = true;
        memcpy(dst, src, safe);
        src += safe;
        dst += safe;
    }

    bool IsValid() const noexcept { return !overflow; }
};

} // namespace Security

// -- Decoder sandbox policy types (used by tests via 'using namespace Engine') --

enum class DecoderSandboxLevel : uint8_t { None, Standard, Strict, Paranoid };

inline const char* DecoderSandboxLevelName(DecoderSandboxLevel l) noexcept {
    switch (l) {
    case DecoderSandboxLevel::None:     return "None";
    case DecoderSandboxLevel::Standard: return "Standard";
    case DecoderSandboxLevel::Strict:   return "Strict";
    case DecoderSandboxLevel::Paranoid: return "Paranoid";
    default: return "Unknown";
    }
}

enum class SandboxResourceLimit : uint8_t { Memory, CPU, Disk, NetworkBandwidth };

inline const char* SandboxResourceLimitName(SandboxResourceLimit r) noexcept {
    switch (r) {
    case SandboxResourceLimit::Memory:           return "Memory";
    case SandboxResourceLimit::CPU:              return "CPU";
    case SandboxResourceLimit::Disk:             return "Disk";
    case SandboxResourceLimit::NetworkBandwidth: return "NetworkBandwidth";
    default: return "Unknown";
    }
}

struct DecoderSandboxRule {
    DecoderSandboxLevel level = DecoderSandboxLevel::Standard;
};

struct DecoderSandboxViolation {
    SandboxResourceLimit resource = SandboxResourceLimit::Memory;
};

class DecoderSandboxPolicy {
public:
    static constexpr uint32_t MAX_MEMORY_MB = 512;
};

} // namespace Engine
} // namespace ExplorerLens
