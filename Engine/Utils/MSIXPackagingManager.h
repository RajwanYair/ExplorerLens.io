//==============================================================================
// ExplorerLens Engine — MSIX Packaging
// Modern MSIX package with capabilities declarations and auto-update foundation.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// MSIX packaging target
enum class MSIXTarget : uint8_t {
    Desktop,            // Win32 desktop bridge
    Store,              // Microsoft Store submission
    Sideload,           // Enterprise sideloading
    Development         // Dev/test unsigned
};

/// MSIX capability declarations
enum class MSIXCapability : uint8_t {
    ShellExtension,         // Shell thumbnail handler
    FileTypeAssociation,    // File type registrations
    COMServer,              // COM DLL registration
    RunFullTrust,           // Full trust (required for shell ext)
    RestrictedFiles,        // Access to all file types
    Removable,              // Removable storage access
    COUNT
};

/// MSIX package info
struct MSIXPackageInfo {
    std::wstring identity;          // Package identity name
    std::wstring publisher;         // Publisher identity
    std::wstring version;           // Package version (A.B.C.D)
    std::wstring displayName;       // User-facing name
    std::wstring description;
    MSIXTarget   target             = MSIXTarget::Desktop;
    uint64_t     estimatedSizeKB    = 3200;
    bool         isSigned           = false;
    bool         hasAutoUpdate      = false;
};

/// MSIX packaging manager
class MSIXPackagingManager {
public:
    /// Target name
    static const wchar_t* TargetName(MSIXTarget t) {
        switch (t) {
            case MSIXTarget::Desktop:     return L"Desktop Bridge";
            case MSIXTarget::Store:       return L"Microsoft Store";
            case MSIXTarget::Sideload:    return L"Enterprise Sideload";
            case MSIXTarget::Development: return L"Development";
            default: return L"Unknown";
        }
    }

    /// Capability name
    static const wchar_t* CapabilityName(MSIXCapability c) {
        switch (c) {
            case MSIXCapability::ShellExtension:      return L"Shell Extension";
            case MSIXCapability::FileTypeAssociation: return L"File Type Association";
            case MSIXCapability::COMServer:           return L"COM Server";
            case MSIXCapability::RunFullTrust:        return L"Run Full Trust";
            case MSIXCapability::RestrictedFiles:     return L"Restricted Files";
            case MSIXCapability::Removable:           return L"Removable Storage";
            default: return L"Unknown";
        }
    }

    /// Target count
    static constexpr size_t TargetCount() { return 4; }

    /// Capability count
    static constexpr size_t CapabilityCount() { return static_cast<size_t>(MSIXCapability::COUNT); }

    /// Generate AppxManifest identity
    static std::wstring GenerateIdentity(const std::wstring& name, const std::wstring& version) {
        return name + L"_" + version;
    }

    /// Validate version format (A.B.C.D)
    static bool ValidateVersion(const std::wstring& version) {
        int dots = 0;
        for (auto c : version) {
            if (c == '.') dots++;
            else if (c < '0' || c > '9') return false;
        }
        return dots == 3;
    }
};

}} // namespace ExplorerLens::Engine

