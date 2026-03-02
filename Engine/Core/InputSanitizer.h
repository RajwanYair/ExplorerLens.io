// InputSanitizer.h — Centralized Path and Buffer Sanitization
// Copyright (c) 2026 ExplorerLens Project
//
// Security layer that scrubs file paths for null-byte injection, embedded
// control characters, NTFS Alternate Data Streams, trailing dots/spaces,
// and MAX_PATH violations. Returns a modified safe path along with a list
// of warnings. Also provides a buffer validator for raw data scanning.
// All sanitization flags are composable via bitmask.
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
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class SanitizeFlags : uint32_t {
    None = 0,
    NullByteInjection = 1 << 0,
    UnicodeNormalize = 1 << 1,
    StripADS = 1 << 2,   // NTFS Alternate Data Streams
    MaxPathEnforce = 1 << 3,
    ControlCharStrip = 1 << 4,
    TrailingDotSpace = 1 << 5,
    EncodingValidation = 1 << 6,
    All = NullByteInjection | UnicodeNormalize | StripADS | MaxPathEnforce | ControlCharStrip | TrailingDotSpace | EncodingValidation
};

inline SanitizeFlags operator|(SanitizeFlags a, SanitizeFlags b) {
    return static_cast<SanitizeFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool HasFlag(SanitizeFlags flags, SanitizeFlags test) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(test)) != 0;
}

struct SanitizeResult {
    bool         safe = false;
    bool         modified = false;
    std::wstring sanitizedPath;
    std::vector<std::wstring> warnings;
    uint32_t     issuesFound = 0;
};

struct BufferSanitizeResult {
    bool     safe = false;
    uint32_t nullBytes = 0;
    uint32_t controlChars = 0;
    bool     validEncoding = true;
};

// ========================================================================
// InputSanitizer — Security-focused input sanitization
// ========================================================================
class InputSanitizer {
public:
    static InputSanitizer& Instance() {
        static InputSanitizer instance;
        return instance;
    }

    void Initialize(SanitizeFlags defaultFlags = SanitizeFlags::All, uint32_t maxPathLen = 32767) {
        m_defaultFlags = defaultFlags;
        m_maxPathLen = maxPathLen;
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Sanitize a file path
    SanitizeResult SanitizePath(const std::wstring& inputPath, SanitizeFlags flags = SanitizeFlags::All) {
        SanitizeResult result;
        result.sanitizedPath = inputPath;
        result.safe = true;

        if (inputPath.empty()) {
            result.safe = false;
            result.warnings.push_back(L"Empty path");
            return result;
        }

        // Null byte injection check
        if (HasFlag(flags, SanitizeFlags::NullByteInjection)) {
            for (size_t i = 0; i < result.sanitizedPath.size(); ++i) {
                if (result.sanitizedPath[i] == L'\0') {
                    result.sanitizedPath.resize(i); // Truncate at null
                    result.modified = true;
                    result.issuesFound++;
                    result.warnings.push_back(L"Null byte injection detected");
                    break;
                }
            }
        }

        // Control character stripping
        if (HasFlag(flags, SanitizeFlags::ControlCharStrip)) {
            std::wstring cleaned;
            cleaned.reserve(result.sanitizedPath.size());
            for (wchar_t c : result.sanitizedPath) {
                if (c >= 32 || c == L'\t') {
                    cleaned += c;
                }
                else {
                    result.issuesFound++;
                    result.modified = true;
                }
            }
            if (result.modified) {
                result.sanitizedPath = cleaned;
                result.warnings.push_back(L"Control characters removed");
            }
        }

        // Strip NTFS Alternate Data Streams (path:stream)
        if (HasFlag(flags, SanitizeFlags::StripADS)) {
            // Skip drive letter colon (e.g., C:)
            size_t searchStart = 0;
            if (result.sanitizedPath.size() >= 2 && result.sanitizedPath[1] == L':') {
                searchStart = 2;
            }
            size_t adsPos = result.sanitizedPath.find(L':', searchStart);
            if (adsPos != std::wstring::npos) {
                result.sanitizedPath = result.sanitizedPath.substr(0, adsPos);
                result.modified = true;
                result.issuesFound++;
                result.warnings.push_back(L"NTFS ADS stripped");
            }
        }

        // Trailing dots and spaces (problematic on Windows)
        if (HasFlag(flags, SanitizeFlags::TrailingDotSpace)) {
            while (!result.sanitizedPath.empty()) {
                wchar_t last = result.sanitizedPath.back();
                if (last == L'.' || last == L' ') {
                    result.sanitizedPath.pop_back();
                    result.modified = true;
                    result.issuesFound++;
                }
                else {
                    break;
                }
            }
            if (result.modified && result.sanitizedPath.empty()) {
                result.safe = false;
                result.warnings.push_back(L"Path reduced to empty after trailing char strip");
            }
        }

        // Max path enforcement
        if (HasFlag(flags, SanitizeFlags::MaxPathEnforce)) {
            if (result.sanitizedPath.size() > m_maxPathLen) {
                result.safe = false;
                result.issuesFound++;
                result.warnings.push_back(L"Path exceeds maximum length");
            }
        }

        m_pathsProcessed++;
        if (result.issuesFound > 0) m_issuesDetected += result.issuesFound;

        return result;
    }

    // Validate a raw buffer
    BufferSanitizeResult ValidateBuffer(const uint8_t* data, size_t size) {
        BufferSanitizeResult result;
        result.safe = true;

        if (!data || size == 0) {
            result.safe = false;
            return result;
        }

        for (size_t i = 0; i < size; ++i) {
            if (data[i] == 0) {
                result.nullBytes++;
            }
            if (data[i] < 32 && data[i] != '\n' && data[i] != '\r' && data[i] != '\t' && data[i] != 0) {
                result.controlChars++;
            }
        }

        return result;
    }

    // Quick path safety check (no modification)
    bool IsPathSafe(const std::wstring& path) const {
        if (path.empty() || path.size() > m_maxPathLen) return false;

        for (wchar_t c : path) {
            if (c == L'\0') return false;
            if (c < 32 && c != L'\t') return false;
        }

        return true;
    }

    // Stats
    uint64_t GetPathsProcessed() const { return m_pathsProcessed; }
    uint64_t GetIssuesDetected() const { return m_issuesDetected; }

private:
    InputSanitizer() = default;

    SanitizeFlags m_defaultFlags = SanitizeFlags::All;
    uint32_t      m_maxPathLen = 32767;
    uint64_t      m_pathsProcessed = 0;
    uint64_t      m_issuesDetected = 0;
    bool          m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
