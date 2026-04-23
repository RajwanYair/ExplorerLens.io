// ErrorCategorizationEngine.cpp — HRESULT / errno → DecodeErrorCategory mapping
// Copyright (c) 2026 ExplorerLens Project
//
#include "ErrorCategorizationEngine.h"

#include <cerrno>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// DecodeErrorRecord::Summary
// ---------------------------------------------------------------------------

std::string DecodeErrorRecord::Summary() const {
    std::ostringstream ss;
    ss << DecodeErrorCategoryString(category);
    if (!format.empty())   { ss << " [" << format << "]"; }
    if (hresult != 0)      { ss << " hr=0x" << std::hex << hresult << std::dec; }
    if (code != 0)         { ss << " code=" << code; }
    if (!message.empty())  { ss << " — " << message; }
    return ss.str();
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::FromHresult
// ---------------------------------------------------------------------------

DecodeErrorCategory ErrorCategorizationEngine::FromHresult(
    uint32_t hr, std::string_view /*format*/) noexcept {
    // Success
    if (hr == 0x00000000u) return DecodeErrorCategory::None;            // S_OK
    // Memory
    if (hr == 0x8007000Eu) return DecodeErrorCategory::OutOfMemory;     // E_OUTOFMEMORY
    // Access / I/O
    if (hr == 0x80070005u) return DecodeErrorCategory::PermissionDenied; // E_ACCESSDENIED
    if (hr == 0x80070002u) return DecodeErrorCategory::FileNotFound;    // E_FILENOTFOUND
    if (hr == 0x80070003u) return DecodeErrorCategory::FileNotFound;    // E_PATHNOTFOUND
    // Argument validation
    if (hr == 0x80070057u) return DecodeErrorCategory::InvalidFormat;   // E_INVALIDARG
    // Timeout
    if (hr == 0x80070102u) return DecodeErrorCategory::Timeout;         // E_TIMEOUT
    // WIC codec errors
    if (hr == 0x88982F80u) return DecodeErrorCategory::UnsupportedVersion;  // WINCODEC_UNSUPPORTED_PIXEL
    if (hr == 0x88982F61u) return DecodeErrorCategory::CorruptedData;       // WINCODEC_FRAMEMISSING
    if (hr == 0x88982F20u) return DecodeErrorCategory::CorruptedData;       // WINCODEC_INVALIDJPEGSCANTYPE
    if (hr == 0x88982F04u) return DecodeErrorCategory::DecoderInitFailed;   // WINCODEC_WRONGSTATE
    if ((hr & 0xFFFF0000u) == 0x88980000u) return DecodeErrorCategory::LibraryError; // WINCODEC_ERR_BASE
    // GPU errors
    if (hr == 0x887A0005u) return DecodeErrorCategory::GPUResourceUnavailable; // DXGI_ERROR_DEVICE_REMOVED
    if (hr == 0x887A0007u) return DecodeErrorCategory::GPUResourceUnavailable; // DXGI_ERROR_DEVICE_RESET
    if (hr == 0x887A0020u) return DecodeErrorCategory::GPUResourceUnavailable; // DXGI_ERROR_DRIVER_INTERNAL_ERROR
    if (hr == 0x887A0002u) return DecodeErrorCategory::GPUResourceUnavailable; // DXGI_ERROR_NOT_FOUND
    if (hr == 0x887A0004u) return DecodeErrorCategory::DecoderUnsupported;     // DXGI_ERROR_UNSUPPORTED
    if (hr == 0x887A0001u) return DecodeErrorCategory::DecoderInitFailed;      // DXGI_ERROR_INVALID_CALL
    if (hr == 0x8899000Cu) return DecodeErrorCategory::GPUResourceUnavailable; // D2DERR_RECREATE_TARGET
    // Not-implemented
    if (hr == 0x80004001u) return DecodeErrorCategory::DecoderUnsupported;     // E_NOTIMPL
    // Generic failure
    return DecodeErrorCategory::IOError;
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::FromErrno
// ---------------------------------------------------------------------------

DecodeErrorCategory ErrorCategorizationEngine::FromErrno(int errnoCode) noexcept {
    switch (errnoCode) {
        case 0:              return DecodeErrorCategory::None;
        case ENOENT:
        case ENODEV:         return DecodeErrorCategory::FileNotFound;
        case EACCES:
        case EPERM:          return DecodeErrorCategory::PermissionDenied;
        case ENOMEM:         return DecodeErrorCategory::OutOfMemory;
        case ENOSPC:         return DecodeErrorCategory::IOError;
        case ETIMEDOUT:      return DecodeErrorCategory::Timeout;
        case EINVAL:         return DecodeErrorCategory::InvalidFormat;
        case ERANGE:         return DecodeErrorCategory::DimensionsTooLarge;
        case EIO:            return DecodeErrorCategory::IOError;
        default:             return DecodeErrorCategory::IOError;
    }
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::FromMessage (keyword heuristic)
// ---------------------------------------------------------------------------

DecodeErrorCategory ErrorCategorizationEngine::FromMessage(std::string_view message) noexcept {
    // Search for well-known substrings (case-insensitive approximation)
    auto contains = [&](std::string_view needle) {
        return message.find(needle) != std::string_view::npos;
    };
    if (contains("out of memory") || contains("allocation failed") || contains("bad_alloc"))
        return DecodeErrorCategory::OutOfMemory;
    if (contains("permission") || contains("access denied") || contains("unauthorized"))
        return DecodeErrorCategory::PermissionDenied;
    if (contains("file not found") || contains("no such file"))
        return DecodeErrorCategory::FileNotFound;
    if (contains("truncated") || contains("unexpected end"))
        return DecodeErrorCategory::TruncatedStream;
    if (contains("corrupt") || contains("invalid signature") || contains("bad magic"))
        return DecodeErrorCategory::CorruptedData;
    if (contains("zip bomb") || contains("decompression ratio"))
        return DecodeErrorCategory::ZipBombDetected;
    if (contains("path traversal") || contains("directory escape"))
        return DecodeErrorCategory::PathTraversalDetected;
    if (contains("unsupported") || contains("not supported") || contains("unknown format"))
        return DecodeErrorCategory::DecoderUnsupported;
    if (contains("timeout") || contains("timed out"))
        return DecodeErrorCategory::Timeout;
    if (contains("gpu") || contains("device removed") || contains("dxgi"))
        return DecodeErrorCategory::GPUResourceUnavailable;
    if (contains("too large") || contains("dimensions exceed") || contains("max size"))
        return DecodeErrorCategory::DimensionsTooLarge;
    return DecodeErrorCategory::IOError;
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::MakeRecord
// ---------------------------------------------------------------------------

DecodeErrorRecord ErrorCategorizationEngine::MakeRecord(
    uint32_t hr,
    std::string_view format,
    std::string_view filePath,
    std::string_view detail) noexcept {
    DecodeErrorRecord rec;
    rec.category = FromHresult(hr);
    rec.hresult  = hr;
    rec.format   = std::string(format);
    rec.filePath = std::string(filePath);
    rec.message  = std::string(detail);
    return rec;
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::MakeErrnoRecord
// ---------------------------------------------------------------------------

DecodeErrorRecord ErrorCategorizationEngine::MakeErrnoRecord(
    int errnoCode,
    std::string_view format,
    std::string_view filePath) noexcept {
    DecodeErrorRecord rec;
    rec.category = FromErrno(errnoCode);
    rec.code     = errnoCode;
    rec.format   = std::string(format);
    rec.filePath = std::string(filePath);
    return rec;
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::MakeCategoryRecord
// ---------------------------------------------------------------------------

DecodeErrorRecord ErrorCategorizationEngine::MakeCategoryRecord(
    DecodeErrorCategory category,
    std::string_view format,
    std::string_view filePath,
    std::string_view detail) noexcept {
    DecodeErrorRecord rec;
    rec.category = category;
    rec.format   = std::string(format);
    rec.filePath = std::string(filePath);
    rec.message  = std::string(detail);
    return rec;
}

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine::Upgrade
// ---------------------------------------------------------------------------

DecodeErrorCategory ErrorCategorizationEngine::Upgrade(
    DecodeErrorCategory base,
    DecodeErrorCategory context) noexcept {
    // Security categories always win over I/O categories
    if (IsSecurityError(context)) return context;
    if (IsSecurityError(base))    return base;
    // More specific wins over generic
    if (base == DecodeErrorCategory::IOError && context != DecodeErrorCategory::None)
        return context;
    return base;
}

} // namespace Engine
} // namespace ExplorerLens
