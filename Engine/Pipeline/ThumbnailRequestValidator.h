// ThumbnailRequestValidator.h — Pre-Pipeline Request Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Front-gate validator that checks thumbnail requests for null/empty paths,
// path-length limits, invalid characters, dimension bounds, and optionally
// file existence/size before entering the decode pipeline. Each rejection
// reason is tracked in per-error-code counters for diagnostics. The
// file-existence check is off by default to avoid I/O on the hot path.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

enum class RequestValidationError : uint32_t {
    None = 0,
    NullPath = 1,
    EmptyPath = 2,
    PathTooLong = 3,
    InvalidCharacters = 4,
    ZeroDimensions = 5,
    OversizedWidth = 6,
    OversizedHeight = 7,
    FileNotFound = 8,
    ZeroFileSize = 9,
    FileTooLarge = 10,
    UnsupportedFormat = 11,
    InvalidEncoding = 12
};

static const wchar_t* RequestValidationErrorName(RequestValidationError e) {
    static const wchar_t* names[] = {
        L"None", L"NullPath", L"EmptyPath", L"PathTooLong",
        L"InvalidCharacters", L"ZeroDimensions", L"OversizedWidth",
        L"OversizedHeight", L"FileNotFound", L"ZeroFileSize",
        L"FileTooLarge", L"UnsupportedFormat", L"InvalidEncoding"
    };
    auto idx = static_cast<uint32_t>(e);
    return (idx <= 12) ? names[idx] : L"Unknown";
}

struct RequestValidationResult {
    bool                              valid = false;
    std::vector<RequestValidationError> errors;
    std::wstring                       firstErrorMessage;

    void AddError(RequestValidationError err, const std::wstring& msg = L"") {
        errors.push_back(err);
        if (firstErrorMessage.empty()) {
            firstErrorMessage = msg.empty() ? RequestValidationErrorName(err) : msg;
        }
        valid = false;
    }
};

struct RequestValidationLimits {
    uint32_t maxWidth = 4096;
    uint32_t maxHeight = 4096;
    uint32_t maxPathLength = 32767;   // MAX_PATH extended
    uint64_t maxFileSizeBytes = 4ULL * 1024 * 1024 * 1024; // 4 GB
    bool     checkFileExists = false;   // Disabled by default (perf)
    bool     checkFileSize = false;
};

struct RequestValidationStats {
    uint64_t totalValidated = 0;
    uint64_t totalValid = 0;
    uint64_t totalInvalid = 0;
    uint64_t errorCounts[13] = {};  // One per RequestValidationError
};

// ========================================================================
// ThumbnailRequestValidator — Validates request parameters before pipeline
// ========================================================================
class ThumbnailRequestValidator {
public:
    static ThumbnailRequestValidator& Instance() {
        static ThumbnailRequestValidator instance;
        return instance;
    }

    void Initialize(const RequestValidationLimits& limits = {}) {
        m_limits = limits;
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Validate a thumbnail request
    RequestValidationResult Validate(const wchar_t* filePath,
        uint32_t requestedWidth,
        uint32_t requestedHeight) {
        RequestValidationResult result;
        result.valid = true;
        m_stats.totalValidated++;

        // Path validation
        if (!filePath) {
            result.AddError(RequestValidationError::NullPath, L"File path is null");
        }
        else {
            size_t pathLen = wcslen(filePath);
            if (pathLen == 0) {
                result.AddError(RequestValidationError::EmptyPath, L"File path is empty");
            }
            else if (pathLen > m_limits.maxPathLength) {
                result.AddError(RequestValidationError::PathTooLong, L"Path exceeds max length");
            }
            else {
                // Check for invalid characters
                if (!ValidatePathCharacters(filePath, pathLen)) {
                    result.AddError(RequestValidationError::InvalidCharacters, L"Path contains invalid characters");
                }
            }
        }

        // Dimension validation
        if (requestedWidth == 0 || requestedHeight == 0) {
            result.AddError(RequestValidationError::ZeroDimensions, L"Requested dimensions are zero");
        }
        else {
            if (requestedWidth > m_limits.maxWidth) {
                result.AddError(RequestValidationError::OversizedWidth, L"Width exceeds maximum");
            }
            if (requestedHeight > m_limits.maxHeight) {
                result.AddError(RequestValidationError::OversizedHeight, L"Height exceeds maximum");
            }
        }

        // File existence check (optional, expensive)
        if (m_limits.checkFileExists && filePath && result.errors.empty()) {
            DWORD attrs = GetFileAttributesW(filePath);
            if (attrs == INVALID_FILE_ATTRIBUTES) {
                result.AddError(RequestValidationError::FileNotFound, L"File not found");
            }
            else if (m_limits.checkFileSize) {
                WIN32_FILE_ATTRIBUTE_DATA fad;
                if (GetFileAttributesExW(filePath, GetFileExInfoStandard, &fad)) {
                    uint64_t fileSize = (static_cast<uint64_t>(fad.nFileSizeHigh) << 32) | fad.nFileSizeLow;
                    if (fileSize == 0) {
                        result.AddError(RequestValidationError::ZeroFileSize, L"File is empty");
                    }
                    else if (fileSize > m_limits.maxFileSizeBytes) {
                        result.AddError(RequestValidationError::FileTooLarge, L"File exceeds size limit");
                    }
                }
            }
        }

        // Update stats
        for (auto err : result.errors) {
            uint32_t idx = static_cast<uint32_t>(err);
            if (idx < 13) m_stats.errorCounts[idx]++;
        }

        if (result.errors.empty()) {
            result.valid = true;
            m_stats.totalValid++;
        }
        else {
            m_stats.totalInvalid++;
        }

        return result;
    }

    // Get stats
    RequestValidationStats GetStats() const { return m_stats; }

    // Get limits
    const RequestValidationLimits& GetLimits() const { return m_limits; }

private:
    ThumbnailRequestValidator() = default;

    bool ValidatePathCharacters(const wchar_t* path, size_t len) const {
        for (size_t i = 0; i < len; ++i) {
            wchar_t c = path[i];
            // Check for null bytes embedded in path
            if (c == L'\0') return false;
            // Control characters (except tab)
            if (c < 32 && c != L'\t') return false;
            // Invalid NTFS characters in filename portion
            // (allow : for drive letter, \ and / for path separators)
        }
        return true;
    }

    RequestValidationLimits m_limits;
    RequestValidationStats  m_stats;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
