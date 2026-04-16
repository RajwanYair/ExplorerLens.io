// ErrorCategorizationEngine.h — Structured decode error categorization and HRESULT mapping
// Copyright (c) 2026 ExplorerLens Project
//
// Maps raw decode failures (HRESULT, exception codes, library error codes) to
// the unified DecodeErrorCategory taxonomy and provides structured error records
// for logging, ETW tracing, and user-facing diagnostics.
//
#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <optional>

#include "DecodeErrorCategory.h"

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// DecodeErrorRecord — structured error with full context
// ---------------------------------------------------------------------------

struct DecodeErrorRecord {
    DecodeErrorCategory category = DecodeErrorCategory::None;
    uint32_t            hresult  = 0;    // Win32 HRESULT (0 = no HRESULT)
    int                 code     = 0;    // Library-specific return code
    std::string         format;          // File extension / format name
    std::string         filePath;        // Affected file path (may be empty)
    std::string         message;         // Human-readable detail

    bool IsFailure() const noexcept {
        return category != DecodeErrorCategory::None;
    }

    bool IsSecurity() const noexcept {
        return IsSecurityError(category);
    }

    bool IsRecoverableError() const noexcept {
        return IsRecoverable(category);
    }

    // Returns a brief one-line description for logging / ETW events
    std::string Summary() const;
};

// ---------------------------------------------------------------------------
// ErrorCategorizationEngine — the categorization service
// ---------------------------------------------------------------------------

class ErrorCategorizationEngine {
public:
    ErrorCategorizationEngine() = default;

    // Map a Win32 HRESULT to the nearest DecodeErrorCategory.
    // Uses the canonical HRESULT → category mapping defined in the .cpp.
    static DecodeErrorCategory FromHresult(uint32_t hr, std::string_view format = {}) noexcept;

    // Map a C errno code to a DecodeErrorCategory.
    static DecodeErrorCategory FromErrno(int errnoCode) noexcept;

    // Categorize based on a free-form message (keyword matching — last resort).
    static DecodeErrorCategory FromMessage(std::string_view message) noexcept;

    // Build a full DecodeErrorRecord from HRESULT + context.
    static DecodeErrorRecord MakeRecord(
        uint32_t hr,
        std::string_view format,
        std::string_view filePath = {},
        std::string_view detail   = {}) noexcept;

    // Build a DecodeErrorRecord from errno + context.
    static DecodeErrorRecord MakeErrnoRecord(
        int errnoCode,
        std::string_view format,
        std::string_view filePath = {}) noexcept;

    // Build a DecodeErrorRecord from a raw category + optional detail.
    static DecodeErrorRecord MakeCategoryRecord(
        DecodeErrorCategory category,
        std::string_view format,
        std::string_view filePath = {},
        std::string_view detail   = {}) noexcept;

    // Upgrade an existing category when additional context is known.
    // E.g., if a TruncatedStream error is found via security scan → ZipBombDetected.
    static DecodeErrorCategory Upgrade(
        DecodeErrorCategory base,
        DecodeErrorCategory context) noexcept;
};

// ---------------------------------------------------------------------------
// Common HRESULT constants (avoids winerror.h dependency)
// ---------------------------------------------------------------------------

namespace HResult {
    inline constexpr uint32_t S_OK               = 0x00000000;
    inline constexpr uint32_t E_FAIL             = 0x80004005;
    inline constexpr uint32_t E_OUTOFMEMORY      = 0x8007000E;
    inline constexpr uint32_t E_INVALIDARG       = 0x80070057;
    inline constexpr uint32_t E_ACCESSDENIED     = 0x80070005;
    inline constexpr uint32_t E_NOTIMPL          = 0x80004001;
    inline constexpr uint32_t E_FILENOTFOUND     = 0x80070002;
    inline constexpr uint32_t E_PATHNOTFOUND     = 0x80070003;
    inline constexpr uint32_t E_TIMEOUT          = 0x80070102;
    inline constexpr uint32_t E_INSUFFICIENT_BUF = 0x8007007A;
    inline constexpr uint32_t WINCODEC_ERR_BASE  = 0x88982F00;
    inline constexpr uint32_t WINCODEC_UNSUPPORTED_PIXEL  = 0x88982F80;
    inline constexpr uint32_t WINCODEC_WRONGSTATE          = 0x88982F04;
    inline constexpr uint32_t WINCODEC_FRAMEMISSING        = 0x88982F61;
    inline constexpr uint32_t WINCODEC_INVALIDJPEGSCANTYPE = 0x88982F20;
    inline constexpr uint32_t D2DERR_RECREATE_TARGET       = 0x8899000C;
    inline constexpr uint32_t DXGI_ERROR_DEVICE_REMOVED    = 0x887A0005;
    inline constexpr uint32_t DXGI_ERROR_DEVICE_RESET      = 0x887A0007;
    inline constexpr uint32_t DXGI_ERROR_DRIVER_INTERNAL_ERROR = 0x887A0020;
    inline constexpr uint32_t DXGI_ERROR_INVALID_CALL      = 0x887A0001;
    inline constexpr uint32_t DXGI_ERROR_NOT_FOUND         = 0x887A0002;
    inline constexpr uint32_t DXGI_ERROR_UNSUPPORTED       = 0x887A0004;
} // namespace HResult

} // namespace Engine
} // namespace ExplorerLens
