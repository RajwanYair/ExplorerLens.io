// Expected.h — Decode-Specific Error Types Built on ResultType.h
// Copyright (c) 2026 ExplorerLens Project
//
// Extends the generic Result<T, E> with decode-specific error categories
// and EngineError for the thumbnail pipeline. COM boundary uses HRESULT;
// internal pipeline uses DecodeResult<T> for structured error propagation.
//
#pragma once

#include <cstdint>
#include <source_location>
#include <string>
#include "ResultType.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Error Categories for the Decode Pipeline
//==============================================================================

enum class DecodeErrorCategory : uint32_t {
    None = 0,
    // Original pipeline codes — ordinals are stable (tests check these values)
    FormatUnsupported,
    DecoderNotFound,
    InvalidImageData,
    FileTooLarge,
    FileNotFound,
    AccessDenied,
    ArchiveCorrupt,
    DecoderCrashed,
    GPUDeviceLost,
    GPUOutOfMemory,
    CacheCorrupt,
    Timeout,
    OutOfMemory,
    PluginError,
    PipelineAborted,
    InternalError,
    // Extended taxonomy (v15.3.0 "Zenith-T" Sprint 10):
    InvalidFormat,
    UnsupportedVersion,
    CorruptedData,
    TruncatedStream,
    MissingRequiredChunk,
    DimensionsTooLarge,
    ZipBombDetected,
    PathTraversalDetected,
    SymlinkAttackDetected,
    EncryptedAndLocked,
    PolicyViolation,
    DecoderInitFailed,
    DecoderUnsupported,
    LibraryError,
    PermissionDenied,
    IOError,
    GPUResourceUnavailable,
    Unknown = 0xFFFFFFFFu,
};

//==============================================================================
// Structured Error for the Decode Pipeline
//==============================================================================

struct EngineError
{
    DecodeErrorCategory category{DecodeErrorCategory::InternalError};
    uint32_t code{0};
    std::string message;
    std::string file;
    int line{0};

    static EngineError Make(DecodeErrorCategory cat, std::string msg,
                            std::source_location loc = std::source_location::current())
    {
        EngineError e;
        e.category = cat;
        e.code = static_cast<uint32_t>(cat);
        e.message = std::move(msg);
        e.file = loc.file_name();
        e.line = static_cast<int>(loc.line());
        return e;
    }

    long ToHResult() const noexcept
    {
        switch (category) {
            case DecodeErrorCategory::None:
                return 0;
            case DecodeErrorCategory::FormatUnsupported:
                return static_cast<long>(0x80040201);
            case DecodeErrorCategory::DecoderNotFound:
                return static_cast<long>(0x80040202);
            case DecodeErrorCategory::FileNotFound:
                return static_cast<long>(0x80070002);
            case DecodeErrorCategory::AccessDenied:
                return static_cast<long>(0x80070005);
            case DecodeErrorCategory::OutOfMemory:
                return static_cast<long>(0x8007000E);
            case DecodeErrorCategory::Timeout:
                return static_cast<long>(0x80070102);
            default:
                return static_cast<long>(0x80004005);
        }
    }

    bool IsRetryable() const noexcept
    {
        return category == DecodeErrorCategory::GPUDeviceLost || category == DecodeErrorCategory::Timeout
               || category == DecodeErrorCategory::GPUOutOfMemory;
    }
};

//==============================================================================
// Type Aliases — DecodeResult uses Result<T, EngineError>
//==============================================================================

template <typename T>
using DecodeResult = Result<T, EngineError>;

using VoidDecodeResult = Result<void, EngineError>;

//==============================================================================
// Decode-Specific Factories
//==============================================================================

template <typename T>
[[nodiscard]] DecodeResult<T> DecodeOk(T value)
{
    return DecodeResult<T>(OkTag{}, std::move(value));
}

[[nodiscard]] inline VoidDecodeResult DecodeOkVoid()
{
    return VoidDecodeResult(OkTag{});
}

template <typename T = void>
[[nodiscard]] Result<T, EngineError> DecodeErr(DecodeErrorCategory cat, std::string msg,
                                               std::source_location loc = std::source_location::current())
{
    return Result<T, EngineError>(ErrTag{}, EngineError::Make(cat, std::move(msg), loc));
}

//==============================================================================
// HRESULT Conversion (COM boundary)
//==============================================================================

[[nodiscard]] inline VoidDecodeResult FromHResult(long hr)
{
    if (hr >= 0)
        return VoidDecodeResult(OkTag{});
    EngineError e;
    e.category = DecodeErrorCategory::InternalError;
    e.code = static_cast<uint32_t>(hr);
    e.message = "HRESULT error: " + std::to_string(hr);
    return VoidDecodeResult(ErrTag{}, std::move(e));
}

}  // namespace Engine
}  // namespace ExplorerLens
