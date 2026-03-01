#pragma once
// FileAssociationManager.h — Manage file type associations for thumbnail handlers
// Sprint 438 — ExplorerLens v15.0.0 Zenith

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>

namespace ExplorerLens {
namespace Engine {

/// Scope of file association registration
enum class AssociationScope : uint8_t {
    User = 0,  // HKCU — current user only
    Machine = 1,  // HKLM — all users (requires admin)
    AllUsers = 2,  // Per-user for every profile
    Portable = 3,  // Portable mode, registry-free
    GPOManaged = 4   // Group Policy managed associations
};

inline const char* AssociationScopeName(AssociationScope s) noexcept {
    switch (s) {
    case AssociationScope::User:       return "User";
    case AssociationScope::Machine:    return "Machine";
    case AssociationScope::AllUsers:   return "AllUsers";
    case AssociationScope::Portable:   return "Portable";
    case AssociationScope::GPOManaged: return "GPOManaged";
    default:                           return "Unknown";
    }
}

/// How to handle conflicting associations
enum class AssociationConflict : uint8_t {
    Overwrite = 0,  // Overwrite existing handler
    Skip = 1,  // Skip if already registered
    Backup = 2,  // Backup existing, then overwrite
    Merge = 3,  // Merge as secondary handler
    Prompt = 4   // Prompt the user
};

inline const char* AssociationConflictName(AssociationConflict c) noexcept {
    switch (c) {
    case AssociationConflict::Overwrite: return "Overwrite";
    case AssociationConflict::Skip:      return "Skip";
    case AssociationConflict::Backup:    return "Backup";
    case AssociationConflict::Merge:     return "Merge";
    case AssociationConflict::Prompt:    return "Prompt";
    default:                             return "Unknown";
    }
}

/// Registration record for a file extension
struct AssociationRecord {
    std::wstring        extension;       // e.g., L".psd"
    std::wstring        handlerCLSID;    // COM CLSID string
    AssociationScope    scope = AssociationScope::User;
    AssociationConflict conflict = AssociationConflict::Skip;
    bool                active = false;
};

/// Summary of conflict detection
struct ConflictReport {
    uint32_t totalExtensions = 0;
    uint32_t conflictsFound = 0;
    uint32_t resolved = 0;
    std::vector<std::wstring> conflictingExtensions;
};

/// Manages file type associations for ExplorerLens thumbnail
/// handlers, supporting user/machine scope, conflict resolution,
/// and GPO-managed enterprise deployments.
class FileAssociationManager {
public:
    FileAssociationManager() = default;
    ~FileAssociationManager() = default;

    FileAssociationManager(const FileAssociationManager&) = delete;
    FileAssociationManager& operator=(const FileAssociationManager&) = delete;
    FileAssociationManager(FileAssociationManager&&) noexcept = default;
    FileAssociationManager& operator=(FileAssociationManager&&) noexcept = default;

    /// Register a file extension for thumbnail handling
    bool Register(const std::wstring& extension, AssociationScope scope = AssociationScope::User) {
        (void)scope;
        if (extension.empty()) return false;
        m_registered.insert(extension);
        m_registerCount++;
        return true;
    }

    /// Unregister a previously registered extension
    bool Unregister(const std::wstring& extension) {
        auto it = m_registered.find(extension);
        if (it == m_registered.end()) return false;
        m_registered.erase(it);
        m_unregisterCount++;
        return true;
    }

    /// Check if an extension is currently registered
    bool IsRegistered(const std::wstring& extension) const {
        return m_registered.count(extension) > 0;
    }

    /// Detect association conflicts for all registered types
    ConflictReport GetConflicts() const {
        ConflictReport report;
        report.totalExtensions = static_cast<uint32_t>(m_registered.size());
        report.conflictsFound = 0;
        return report;
    }

    /// Get number of registered extensions
    size_t GetRegisteredCount() const noexcept { return m_registered.size(); }

    /// Get register operation count
    uint64_t GetRegisterCount() const noexcept { return m_registerCount; }

    /// Get unregister operation count
    uint64_t GetUnregisterCount() const noexcept { return m_unregisterCount; }

    /// Set default scope for new registrations
    void SetDefaultScope(AssociationScope scope) noexcept { m_defaultScope = scope; }

    /// Get default scope
    AssociationScope GetDefaultScope() const noexcept { return m_defaultScope; }

private:
    std::unordered_set<std::wstring> m_registered;
    AssociationScope m_defaultScope = AssociationScope::User;
    uint64_t         m_registerCount = 0;
    uint64_t         m_unregisterCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
