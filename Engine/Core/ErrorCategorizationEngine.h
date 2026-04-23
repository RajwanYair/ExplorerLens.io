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

} // namespace Engine
} // namespace ExplorerLens
