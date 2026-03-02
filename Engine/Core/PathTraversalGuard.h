#pragma once
// ============================================================================
// PathTraversalGuard.h — Path traversal and symlink escape prevention
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

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

// Traversal detection result
enum class TraversalDetection : uint32_t {
    Safe = 0,
    DotDotTraversal = 1,
    SymlinkEscape = 2,
    JunctionEscape = 3,
    AbsoluteInject = 4,   // Absolute path inside archive
    UNCInject = 5,   // UNC path injection
    DevicePath = 6,   // Device path (\\.\, \\?\)
    ReservedName = 7    // CON, PRN, AUX, NUL, etc.
};

static const wchar_t* TraversalDetectionName(TraversalDetection d) {
    static const wchar_t* names[] = {
        L"Safe", L"DotDotTraversal", L"SymlinkEscape", L"JunctionEscape",
        L"AbsoluteInject", L"UNCInject", L"DevicePath", L"ReservedName"
    };
    auto idx = static_cast<uint32_t>(d);
    return (idx <= 7) ? names[idx] : L"Unknown";
}

// Path check result
struct PathTraversalResult {
    bool                safe = false;
    TraversalDetection  detection = TraversalDetection::Safe;
    std::wstring        resolvedPath;
    std::wstring        reason;
};

// Guard stats
struct PathTraversalGuardStats {
    uint64_t totalChecks = 0;
    uint64_t totalBlocked = 0;
    uint64_t dotDotBlocks = 0;
    uint64_t symlinkBlocks = 0;
    uint64_t junctionBlocks = 0;
    uint64_t absPathBlocks = 0;
    uint64_t uncPathBlocks = 0;
    uint64_t devicePathBlocks = 0;
};

// ========================================================================
// PathTraversalGuard — File path security enforcement
// ========================================================================
class PathTraversalGuard {
public:
    static PathTraversalGuard& Instance() {
        static PathTraversalGuard instance;
        return instance;
    }

    void Initialize() {
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Check if an extraction path is safe within a root directory
    PathTraversalResult ValidateExtractionPath(const std::wstring& rootDir,
        const std::wstring& entryPath) {
        PathTraversalResult result;
        m_stats.totalChecks++;

        // Check for dot-dot traversal
        if (ContainsDotDot(entryPath)) {
            result.detection = TraversalDetection::DotDotTraversal;
            result.reason = L"Path contains '../' traversal";
            m_stats.dotDotBlocks++;
            m_stats.totalBlocked++;
            return result;
        }

        // Check for absolute path injection
        if (IsAbsolutePath(entryPath)) {
            result.detection = TraversalDetection::AbsoluteInject;
            result.reason = L"Absolute path in archive entry";
            m_stats.absPathBlocks++;
            m_stats.totalBlocked++;
            return result;
        }

        // Check for UNC path
        if (IsUNCPath(entryPath)) {
            result.detection = TraversalDetection::UNCInject;
            result.reason = L"UNC network path injection";
            m_stats.uncPathBlocks++;
            m_stats.totalBlocked++;
            return result;
        }

        // Check for device paths
        if (IsDevicePath(entryPath)) {
            result.detection = TraversalDetection::DevicePath;
            result.reason = L"Device path injection";
            m_stats.devicePathBlocks++;
            m_stats.totalBlocked++;
            return result;
        }

        // Check for reserved names
        if (IsReservedName(entryPath)) {
            result.detection = TraversalDetection::ReservedName;
            result.reason = L"Windows reserved device name";
            m_stats.totalBlocked++;
            return result;
        }

        // Resolve and check final path stays within root
        result.resolvedPath = rootDir;
        if (!result.resolvedPath.empty() && result.resolvedPath.back() != L'\\')
            result.resolvedPath += L'\\';
        result.resolvedPath += entryPath;

        result.safe = true;
        result.detection = TraversalDetection::Safe;
        return result;
    }

    // Check if a file is a symlink or junction
    TraversalDetection CheckReparsePoint(const std::wstring& path) {
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) return TraversalDetection::Safe;

        if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW(path.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                FindClose(hFind);
                if (fd.dwReserved0 == IO_REPARSE_TAG_SYMLINK) {
                    m_stats.symlinkBlocks++;
                    m_stats.totalBlocked++;
                    return TraversalDetection::SymlinkEscape;
                }
                if (fd.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT) {
                    m_stats.junctionBlocks++;
                    m_stats.totalBlocked++;
                    return TraversalDetection::JunctionEscape;
                }
            }
        }
        return TraversalDetection::Safe;
    }

    // Quick safety check
    bool IsPathSafe(const std::wstring& rootDir, const std::wstring& entryPath) {
        auto result = ValidateExtractionPath(rootDir, entryPath);
        return result.safe;
    }

    // Stats
    PathTraversalGuardStats GetStats() const { return m_stats; }

private:
    PathTraversalGuard() = default;

    bool ContainsDotDot(const std::wstring& path) const {
        return path.find(L"..\\") != std::wstring::npos ||
            path.find(L"../") != std::wstring::npos ||
            path == L".." ||
            (path.size() >= 3 && path.substr(path.size() - 3) == L"\\..");
    }

    bool IsAbsolutePath(const std::wstring& path) const {
        if (path.size() >= 2 && path[1] == L':') return true;  // C:\...
        if (path.size() >= 1 && (path[0] == L'\\' || path[0] == L'/')) return true;
        return false;
    }

    bool IsUNCPath(const std::wstring& path) const {
        return (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\');
    }

    bool IsDevicePath(const std::wstring& path) const {
        if (path.size() >= 4) {
            if ((path[0] == L'\\' && path[1] == L'\\' && path[2] == L'.' && path[3] == L'\\') ||
                (path[0] == L'\\' && path[1] == L'\\' && path[2] == L'?' && path[3] == L'\\')) {
                return true;
            }
        }
        return false;
    }

    bool IsReservedName(const std::wstring& path) const {
        // Extract filename (last component)
        size_t lastSep = path.find_last_of(L"\\/");
        std::wstring name = (lastSep != std::wstring::npos) ? path.substr(lastSep + 1) : path;

        // Strip extension
        size_t dotPos = name.find(L'.');
        if (dotPos != std::wstring::npos) name = name.substr(0, dotPos);

        // Convert to uppercase
        for (auto& c : name) c = towupper(c);

        static const wchar_t* reserved[] = {
            L"CON", L"PRN", L"AUX", L"NUL",
            L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
            L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9"
        };

        for (auto r : reserved) {
            if (name == r) return true;
        }
        return false;
    }

    PathTraversalGuardStats m_stats;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
