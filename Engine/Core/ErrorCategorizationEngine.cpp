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
    using H = HResult;
    // Success
    if (hr == H::S_OK) return DecodeErrorCategory::None;
    // Memory
    if (hr == H::E_OUTOFMEMORY) return DecodeErrorCategory::OutOfMemory;
    // Access / I/O
    if (hr == H::E_ACCESSDENIED) return DecodeErrorCategory::PermissionDenied;
    if (hr == H::E_FILENOTFOUND) return DecodeErrorCategory::FileNotFound;
    if (hr == H::E_PATHNOTFOUND) return DecodeErrorCategory::FileNotFound;
    // Argument validation
    if (hr == H::E_INVALIDARG) return DecodeErrorCategory::InvalidFormat;
    // Timeout
    if (hr == H::E_TIMEOUT) return DecodeErrorCategory::Timeout;
    // WIC codec errors
    if (hr == H::WINCODEC_UNSUPPORTED_PIXEL)  return DecodeErrorCategory::UnsupportedVersion;
    if (hr == H::WINCODEC_FRAMEMISSING)        return DecodeErrorCategory::CorruptedData;
    if (hr == H::WINCODEC_INVALIDJPEGSCANTYPE) return DecodeErrorCategory::CorruptedData;
    if (hr == H::WINCODEC_WRONGSTATE)          return DecodeErrorCategory::DecoderInitFailed;
    if ((hr & 0xFFFF0000) == H::WINCODEC_ERR_BASE) return DecodeErrorCategory::LibraryError;
    // GPU errors
    if (hr == H::DXGI_ERROR_DEVICE_REMOVED)        return DecodeErrorCategory::GPUResourceUnavailable;
    if (hr == H::DXGI_ERROR_DEVICE_RESET)          return DecodeErrorCategory::GPUResourceUnavailable;
    if (hr == H::DXGI_ERROR_DRIVER_INTERNAL_ERROR) return DecodeErrorCategory::GPUResourceUnavailable;
    if (hr == H::DXGI_ERROR_NOT_FOUND)             return DecodeErrorCategory::GPUResourceUnavailable;
    if (hr == H::DXGI_ERROR_UNSUPPORTED)           return DecodeErrorCategory::DecoderUnsupported;
    if (hr == H::DXGI_ERROR_INVALID_CALL)          return DecodeErrorCategory::DecoderInitFailed;
    if (hr == H::D2DERR_RECREATE_TARGET)           return DecodeErrorCategory::GPUResourceUnavailable;
    // Not-implemented
    if (hr == H::E_NOTIMPL) return DecodeErrorCategory::DecoderUnsupported;
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
