//==============================================================================
// DarkThumbs Engine - Validation Helpers
// Copyright (c) 2026 - DarkThumbs Project  
// Task B6: Reusable validation functions
//==============================================================================

#pragma once

#include <windows.h>
#include <string>
#include <algorithm>

namespace DarkThumbs {
namespace Engine {
namespace Validation {

    /// <summary>
    /// Validates file path for safety and correctness
    /// </summary>
    inline bool IsValidFilePath(const wchar_t* path) {
        if (!path || path[0] == L'\0') {
            return false;
        }
        
        size_t len = wcslen(path);
        
        // Check maximum path length (32767 for Unicode API, 260 for ANSI)
        if (len >= MAX_PATH && len < 32767) {
            // Long path - check for \\?\ prefix
            if (wcsncmp(path, L"\\\\?\\", 4) != 0) {
                return false;
            }
        } else if (len >= 32767) {
            return false; // Exceeds Windows limit
        }
        
        // Check for invalid characters (basic check)
        const wchar_t* invalidChars = L"<>|\"";
        for (const wchar_t* p = path; *p; ++p) {
            if (wcschr(invalidChars, *p)) {
                return false;
            }
            // Check for control characters
            if (*p < 32 && *p != L'\t') {
                return false;
            }
        }
        
        return true;
    }

    /// <summary>
    /// Validates thumbnail dimensions
    /// </summary>
    inline bool IsValidDimensions(uint32_t width, uint32_t height) {
        // Zero dimensions invalid
        if (width == 0 || height == 0) {
            return false;
        }
        
        // Check reasonable maximums (8K = 7680x4320)
        constexpr uint32_t MAX_DIM = 8192;
        if (width > MAX_DIM || height > MAX_DIM) {
            return false;
        }
        
        // Check for potential overflow in pixel buffer calculation
        constexpr uint64_t MAX_PIXELS = 8192ULL * 8192ULL;  // 64 megapixels
        uint64_t totalPixels = static_cast<uint64_t>(width) * height;
        if (totalPixels > MAX_PIXELS) {
            return false;
        }
        
        return true;
    }

    /// <summary>
    /// Validates pixel buffer size
    /// </summary>
    inline bool IsValidBufferSize(size_t bufferSize) {
        // Check for reasonable max (512MB)
        constexpr size_t MAX_SIZE = 512ULL * 1024 * 1024;
        return bufferSize > 0 && bufferSize <= MAX_SIZE;
    }

    /// <summary>
    /// Validates file extension
    /// </summary>
    inline bool IsValidExtension(const wchar_t* ext) {
        if (!ext || ext[0] != L'.') {
            return false;
        }
        
        size_t len = wcslen(ext);
        if (len < 2 || len > 10) {  // .x to .extension (max 9 chars after dot)
            return false;
        }
        
        // Check for valid extension characters
        for (const wchar_t* p = ext + 1; *p; ++p) {
            if (!iswalnum(*p) && *p != L'_' && *p != L'-') {
                return false;
            }
        }
        
        return true;
    }

    /// <summary>
    /// Validates decoder name
    /// </summary>
    inline bool IsValidDecoderName(const wchar_t* name) {
        if (!name || name[0] == L'\0') {
            return false;
        }
        
        size_t len = wcslen(name);
        if (len > 64) {  // Reasonable max
            return false;
        }
        
        return true;
    }

    /// <summary>
    /// Sanitizes file path for logging (removes sensitive info)
    /// </summary>
    inline std::wstring SanitizePathForLogging(const wchar_t* path) {
        if (!path) return L"<null>";
        
        std::wstring sanitized(path);
        
        // Replace username in path with <user>
        const wchar_t* users = L"\\Users\\";
        size_t pos = sanitized.find(users);
        if (pos != std::wstring::npos) {
            size_t userStart = pos + wcslen(users);
            size_t nextSlash = sanitized.find(L'\\', userStart);
            if (nextSlash != std::wstring::npos) {
                sanitized.replace(userStart, nextSlash - userStart, L"<user>");
            }
        }
        
        // Limit path length in logs
        if (sanitized.length() > 120) {
            sanitized = sanitized.substr(0, 60) + L"..." + 
                        sanitized.substr(sanitized.length() - 57);
        }
        
        return sanitized;
    }

} // namespace Validation
} // namespace Engine
} // namespace DarkThumbs
