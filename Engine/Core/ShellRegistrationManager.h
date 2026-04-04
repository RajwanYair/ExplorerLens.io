//==============================================================================
// ExplorerLens Engine — Shell Registration Expansion V2
// Validates shell registration completeness and manages extension lists.
//==============================================================================
#pragma once
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Represents a single shell registration entry
struct ShellRegEntry
{
    std::wstring extension;    // e.g. L".webp"
    std::wstring description;  // e.g. L"WebP Image File"
    std::wstring contentType;  // e.g. L"image/webp"
    bool registered = false;   // currently in .rgs
};

/// Shell registration manager — tracks and validates extensions
class ShellRegistrationManager
{
  public:
    /// Add a new extension as registered
    void AddRegistered(const std::wstring& ext, const std::wstring& desc = L"")
    {
        ShellRegEntry entry;
        entry.extension = ext;
        entry.description = desc;
        entry.registered = true;
        m_entries.push_back(entry);
        m_registeredSet.insert(ext);
    }

    /// Add an extension that code supports but isn't registered
    void AddSupported(const std::wstring& ext, const std::wstring& desc = L"")
    {
        ShellRegEntry entry;
        entry.extension = ext;
        entry.description = desc;
        entry.registered = false;
        m_entries.push_back(entry);
    }

    /// Check if extension is registered
    bool IsRegistered(const std::wstring& ext) const
    {
        return m_registeredSet.count(ext) > 0;
    }

    /// Get all extensions that should be added
    std::vector<std::wstring> GetMissingRegistrations() const
    {
        std::vector<std::wstring> missing;
        for (auto& e : m_entries) {
            if (!e.registered)
                missing.push_back(e.extension);
        }
        return missing;
    }

    /// Extensions newly added in v10.6
    static std::vector<std::wstring> GetV106NewExtensions()
    {
        return {L".lz4", L".tbz", L".txz", L".tzst", L".xar", L".ar",    L".chm", L".raw",
                L".ptx", L".r3d", L".rgb", L".rgba", L".dcm", L".dicom", L".dpx", L".cin"};
    }

    /// Total registered count
    size_t RegisteredCount() const
    {
        return m_registeredSet.size();
    }

    /// Total supported count
    size_t TotalCount() const
    {
        return m_entries.size();
    }

    /// Category name for audit reports
    static const wchar_t* CategoryName(int idx)
    {
        static const wchar_t* names[] = {L"Archives",   L"Images", L"ModernImages", L"RAWPhotos",
                                         L"Documents",  L"Fonts",  L"3DModels",     L"eBooks",
                                         L"Scientific", L"Film",   L"Data",         L"Vector"};
        return (idx >= 0 && idx < 12) ? names[idx] : L"Unknown";
    }

    /// Audit result
    struct AuditResult
    {
        size_t registered = 0;
        size_t supported = 0;
        size_t missing = 0;
        bool synced = false;
    };

    AuditResult RunAudit() const
    {
        AuditResult r;
        r.registered = m_registeredSet.size();
        r.supported = m_entries.size();
        r.missing = r.supported - r.registered;
        r.synced = (r.missing == 0);
        return r;
    }

  private:
    std::vector<ShellRegEntry> m_entries;
    std::unordered_set<std::wstring> m_registeredSet;
};

// -- Shell notification engine ------------------------------------------------

enum class ShellNotifyType : uint8_t {
    FileChanged,
    FileAdded,
    Delete,
    DirectoryCreated,
    DirectoryRemoved,
    AssocChanged,
    ThumbnailReady,
    UpdateDir
};

inline const char* ShellNotifyTypeName(ShellNotifyType t) noexcept
{
    switch (t) {
        case ShellNotifyType::FileChanged:
            return "FileChanged";
        case ShellNotifyType::FileAdded:
            return "FileAdded";
        case ShellNotifyType::Delete:
            return "Delete";
        case ShellNotifyType::DirectoryCreated:
            return "DirectoryCreated";
        case ShellNotifyType::DirectoryRemoved:
            return "DirectoryRemoved";
        case ShellNotifyType::AssocChanged:
            return "AssocChanged";
        case ShellNotifyType::ThumbnailReady:
            return "ThumbnailReady";
        case ShellNotifyType::UpdateDir:
            return "UpdateDir";
        default:
            return "Unknown";
    }
}

enum class ShellNotifyPriority : uint8_t {
    Immediate,
    Normal,
    Batched,
    Deferred
};

inline const char* ShellNotifyPriorityName(ShellNotifyPriority p) noexcept
{
    switch (p) {
        case ShellNotifyPriority::Immediate:
            return "Immediate";
        case ShellNotifyPriority::Normal:
            return "Normal";
        case ShellNotifyPriority::Batched:
            return "Batched";
        case ShellNotifyPriority::Deferred:
            return "Deferred";
        default:
            return "Unknown";
    }
}

struct ShellNotification
{
    ShellNotifyType type = ShellNotifyType::FileChanged;
    ShellNotifyPriority priority = ShellNotifyPriority::Normal;
    std::string itemPath;
    uint64_t timestamp = 0;
};

class ShellNotificationEngine
{
  public:
    bool SendNotification(const ShellNotification& n)
    {
        if (n.priority == ShellNotifyPriority::Immediate) {
            ++m_totalSent;
            return true;
        }
        ++m_pendingCount;
        return true;
    }
    uint32_t GetPendingCount() const noexcept
    {
        return m_pendingCount;
    }
    uint32_t BatchFlush() noexcept
    {
        uint32_t flushed = m_pendingCount;
        m_totalSent += flushed;
        m_pendingCount = 0;
        return flushed;
    }
    uint32_t GetTotalSent() const noexcept
    {
        return m_totalSent;
    }

  private:
    uint32_t m_pendingCount = 0;
    uint32_t m_totalSent = 0;
};

// -- File preview router -------------------------------------------------------

}  // namespace Engine
}  // namespace ExplorerLens
