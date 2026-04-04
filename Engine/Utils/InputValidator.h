// FileSafetyValidator.h — Centralized Input Validation Utility
// Copyright (c) 2026 ExplorerLens Project
//
// Security-critical input validation for a Windows Shell Extension (COM DLL)
// running in explorer.exe. Provides validation for file paths, file sizes,
// image dimensions, and thumbnail sizes. All methods are static and
// thread-safe with no global state.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// Result of an input validation check
struct InputValidationResult
{
    bool valid = false;
    const wchar_t* reason = L"";
};

/// Centralized input validation — all static, no state, thread-safe.
class FileSafetyValidator
{
  public:
    // ====================================================================
    // Path Validation
    // ====================================================================

    /// Maximum path length (Windows extended-length path limit)
    static constexpr size_t MAX_PATH_LENGTH = 32767;

    /// Validate a file path for security issues:
    /// - Rejects null bytes (embedded \0)
    /// - Rejects directory traversal (../ or ..\)
    /// - Rejects excessively long paths (> 32767)
    /// - Rejects empty paths
    static InputValidationResult ValidateFilePath(const std::wstring& path)
    {
        if (path.empty()) {
            return {false, L"Path is empty"};
        }

        if (path.size() > MAX_PATH_LENGTH) {
            return {false, L"Path exceeds maximum length (32767)"};
        }

        // Check for embedded null bytes
        for (size_t i = 0; i < path.size(); ++i) {
            if (path[i] == L'\0') {
                return {false, L"Path contains null byte"};
            }
        }

        // Check for directory traversal patterns
        if (ContainsTraversal(path)) {
            return {false, L"Path contains directory traversal"};
        }

        return {true, L""};
    }

    // ====================================================================
    // File Size Validation
    // ====================================================================

    /// Maximum file size for thumbnail generation: 4 GB
    static constexpr uint64_t MAX_FILE_SIZE = 4ULL * 1024ULL * 1024ULL * 1024ULL;

    /// Validate file size for thumbnail operations.
    /// Rejects files larger than 4 GB.
    static InputValidationResult ValidateFileSize(uint64_t sizeBytes)
    {
        if (sizeBytes == 0) {
            return {false, L"File size is zero"};
        }
        if (sizeBytes > MAX_FILE_SIZE) {
            return {false, L"File exceeds 4 GB limit for thumbnails"};
        }
        return {true, L""};
    }

    // ====================================================================
    // Image Dimension Validation
    // ====================================================================

    /// Maximum image dimension (per axis)
    static constexpr uint32_t MAX_IMAGE_DIMENSION = 65536;

    /// Validate image dimensions.
    /// - Rejects dimensions > 65536 per axis
    /// - Rejects zero dimensions
    /// - Detects multiplication overflow (w * h)
    static InputValidationResult ValidateImageDimensions(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) {
            return {false, L"Image dimension is zero"};
        }
        if (width > MAX_IMAGE_DIMENSION) {
            return {false, L"Image width exceeds 65536"};
        }
        if (height > MAX_IMAGE_DIMENSION) {
            return {false, L"Image height exceeds 65536"};
        }

        // Check for multiplication overflow: w * h * 4 (RGBA)
        uint64_t totalPixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
        uint64_t totalBytes = totalPixels * 4ULL;  // RGBA
        // Cap at 16 GB (prevents absurd allocations)
        if (totalBytes > 16ULL * 1024ULL * 1024ULL * 1024ULL) {
            return {false, L"Image total pixel count causes overflow risk"};
        }

        return {true, L""};
    }

    // ====================================================================
    // Thumbnail Size Validation
    // ====================================================================

    /// Maximum thumbnail dimension
    static constexpr uint32_t MAX_THUMBNAIL_SIZE = 4096;

    /// Validate a requested thumbnail size.
    /// Rejects sizes > 4096 px or zero.
    static InputValidationResult ValidateThumbnailSize(uint32_t size)
    {
        if (size == 0) {
            return {false, L"Thumbnail size is zero"};
        }
        if (size > MAX_THUMBNAIL_SIZE) {
            return {false, L"Thumbnail size exceeds 4096 px limit"};
        }
        return {true, L""};
    }

    // ====================================================================
    // String Sanitization
    // ====================================================================

    /// Strip control characters (U+0000–U+001F except tab/CR/LF, and U+007F)
    /// from a string. Returns the sanitized copy.
    static std::wstring SanitizeString(const std::wstring& str)
    {
        std::wstring result;
        result.reserve(str.size());
        for (wchar_t ch : str) {
            // Allow tab (0x09), LF (0x0A), CR (0x0D)
            if (ch == L'\t' || ch == L'\n' || ch == L'\r') {
                result.push_back(ch);
                continue;
            }
            // Strip C0 control characters and DEL
            if (ch < 0x20 || ch == 0x7F) {
                continue;
            }
            result.push_back(ch);
        }
        return result;
    }

  private:
    /// Check if a path contains directory traversal sequences
    static bool ContainsTraversal(const std::wstring& path)
    {
        // Check for ../ and ..\ patterns
        for (size_t i = 0; i + 2 < path.size(); ++i) {
            if (path[i] == L'.' && path[i + 1] == L'.') {
                // Check if followed by separator or end
                if (i + 2 == path.size()) {
                    return true;  // Trailing ".."
                }
                wchar_t next = path[i + 2];
                if (next == L'/' || next == L'\\') {
                    return true;
                }
            }
        }
        // Also check for leading ..
        if (path.size() >= 2 && path[0] == L'.' && path[1] == L'.') {
            return true;
        }
        return false;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
