// InputValidator.h — Path Traversal Prevention and Input Sanitization
// Copyright (c) 2026 ExplorerLens Project
//
// Validates all user/shell-supplied input at the engine boundary:
// file paths (traversal, UNC, device names), registry key names,
// plugin identifiers, and thumbnail size parameters.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <regex>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ValidationError {
    Ok,
    PathTraversal,      // ".." components
    UNCPath,            // \\server\share
    DevicePath,         // \\.\COM1 etc.
    JunctionLoop,       // Reparse point depth exceeded
    NullByte,           // Embedded null character
    ExcessiveLength,    // > MAX_PATH * 2
    InvalidChars,       // Control characters or reserved Win32 names
    ThumbnailSizeRange, // Outside 16–2048
    RegistryKeyInvalid,
    PluginIdInvalid,
    EmptyInput
};

struct ValidationResult {
    ValidationError error    = ValidationError::Ok;
    std::wstring    canonical; // Normalised path on success
    bool            ok() const { return error == ValidationError::Ok; }
    const wchar_t*  ErrorMessage() const {
        switch (error) {
        case ValidationError::Ok:               return L"OK";
        case ValidationError::PathTraversal:    return L"Path traversal detected";
        case ValidationError::UNCPath:          return L"UNC paths not allowed";
        case ValidationError::DevicePath:       return L"Device paths not allowed";
        case ValidationError::NullByte:         return L"Embedded null character";
        case ValidationError::ExcessiveLength:  return L"Path exceeds maximum length";
        case ValidationError::InvalidChars:     return L"Invalid characters in path";
        case ValidationError::ThumbnailSizeRange: return L"Thumbnail size out of range (16–2048)";
        case ValidationError::RegistryKeyInvalid: return L"Invalid registry key name";
        case ValidationError::PluginIdInvalid:  return L"Invalid plugin identifier";
        case ValidationError::EmptyInput:       return L"Empty input";
        default:                               return L"Unknown error";
        }
    }
};

class InputValidator {
public:
    // Validate a filesystem path supplied by Explorer shell or plugin
    ValidationResult ValidatePath(const std::wstring& path) const {
        ValidationResult res;

        if (path.empty()) { res.error = ValidationError::EmptyInput; return res; }

        // Embedded null bytes
        if (path.find(L'\0') != std::wstring::npos) {
            res.error = ValidationError::NullByte; return res;
        }

        // Length check
        if (path.size() > 32767) {
            res.error = ValidationError::ExcessiveLength; return res;
        }

        // UNC path: \\server\share
        if (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\') {
            // Block \\.\  device paths
            if (path.size() >= 4 && path[2] == L'.' && path[3] == L'\\') {
                res.error = ValidationError::DevicePath; return res;
            }
            // Block \\?\ absolute paths — allow them but mark controlled
            // UNC shares: allow (common in enterprise)
        }

        // Canonicalize with GetFullPathNameW to resolve . and ..
        wchar_t canon[32768] = {};
        DWORD len = GetFullPathNameW(path.c_str(), 32768, canon, nullptr);
        if (len == 0 || len >= 32768) {
            res.error = ValidationError::ExcessiveLength; return res;
        }
        std::wstring full(canon, len);

        // Check for path traversal: if canonicalized differs by going outside
        // a sensible root (heuristic: original path contained "..")
        if (path.find(L"..") != std::wstring::npos) {
            // Verify the canonical form doesn't escape a given root
            // For shell extensions, any .. in the input is suspicious
            res.error = ValidationError::PathTraversal; return res;
        }

        // Forbidden Windows device names (CON, AUX, COM1-9, LPT1-9, NUL, PRN)
        static const wchar_t* const FORBIDDEN[] = {
            L"CON",L"AUX",L"NUL",L"PRN",
            L"COM0",L"COM1",L"COM2",L"COM3",L"COM4",L"COM5",L"COM6",L"COM7",L"COM8",L"COM9",
            L"LPT0",L"LPT1",L"LPT2",L"LPT3",L"LPT4",L"LPT5",L"LPT6",L"LPT7",L"LPT8",L"LPT9",
            nullptr
        };
        // Check just the filename component
        const wchar_t* fname = wcsrchr(full.c_str(), L'\\');
        fname = fname ? fname + 1 : full.c_str();
        // Strip extension for comparison
        std::wstring fnameNoExt(fname);
        auto dot = fnameNoExt.rfind(L'.');
        if (dot != std::wstring::npos) fnameNoExt = fnameNoExt.substr(0, dot);
        for (const wchar_t* const* d = FORBIDDEN; *d; ++d) {
            if (_wcsicmp(fnameNoExt.c_str(), *d) == 0) {
                res.error = ValidationError::InvalidChars; return res;
            }
        }

        // Check for control characters (0x00–0x1F)
        for (wchar_t c : full) {
            if (c > 0 && c < 0x20) {
                res.error = ValidationError::InvalidChars; return res;
            }
        }

        res.canonical = full;
        return res;
    }

    // Validate thumbnail size request (16 – 2048 pixels)
    ValidationResult ValidateThumbnailSize(uint32_t width, uint32_t height) const {
        ValidationResult res;
        if (width  < 16 || width  > 2048 ||
            height < 16 || height > 2048) {
            res.error = ValidationError::ThumbnailSizeRange;
        }
        return res;
    }

    // Validate a registry key name segment (no null bytes, no path separators)
    ValidationResult ValidateRegistryKey(const std::wstring& key) const {
        ValidationResult res;
        if (key.empty()) { res.error = ValidationError::EmptyInput; return res; }
        if (key.size() > 255) { res.error = ValidationError::ExcessiveLength; return res; }
        for (wchar_t c : key) {
            if (c == L'\0' || c == L'\\') {
                res.error = ValidationError::RegistryKeyInvalid; return res;
            }
        }
        res.canonical = key;
        return res;
    }

    // Validate a plugin identifier (alphanumeric + hyphen/underscore, 1-64 chars)
    ValidationResult ValidatePluginId(const std::wstring& id) const {
        ValidationResult res;
        if (id.empty() || id.size() > 64) {
            res.error = id.empty() ? ValidationError::EmptyInput
                                   : ValidationError::ExcessiveLength;
            return res;
        }
        for (wchar_t c : id) {
            bool ok = (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z') ||
                      (c >= L'0' && c <= L'9') || c == L'-' || c == L'_';
            if (!ok) { res.error = ValidationError::PluginIdInvalid; return res; }
        }
        res.canonical = id;
        return res;
    }
};

}} // namespace ExplorerLens::Engine
