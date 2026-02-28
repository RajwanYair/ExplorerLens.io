//==============================================================================
// ExplorerLens Engine — Shell Registration Expansion V2
// Validates shell registration completeness and manages extension lists.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

/// Represents a single shell registration entry
struct ShellRegEntry {
 std::wstring extension; // e.g. L".webp"
 std::wstring description; // e.g. L"WebP Image File"
 std::wstring contentType; // e.g. L"image/webp"
 bool registered = false; // currently in .rgs
};

/// Shell registration manager — tracks and validates extensions
class ShellRegistrationManager {
public:
 /// Add a new extension as registered
 void AddRegistered(const std::wstring& ext, const std::wstring& desc = L"") {
 ShellRegEntry entry;
 entry.extension = ext;
 entry.description = desc;
 entry.registered = true;
 m_entries.push_back(entry);
 m_registeredSet.insert(ext);
 }

 /// Add an extension that code supports but isn't registered
 void AddSupported(const std::wstring& ext, const std::wstring& desc = L"") {
 ShellRegEntry entry;
 entry.extension = ext;
 entry.description = desc;
 entry.registered = false;
 m_entries.push_back(entry);
 }

 /// Check if extension is registered
 bool IsRegistered(const std::wstring& ext) const {
 return m_registeredSet.count(ext) > 0;
 }

 /// Get all extensions that should be added
 std::vector<std::wstring> GetMissingRegistrations() const {
 std::vector<std::wstring> missing;
 for (auto& e : m_entries) {
 if (!e.registered) missing.push_back(e.extension);
 }
 return missing;
 }

 /// Extensions newly added in v10.6
 static std::vector<std::wstring> GetV106NewExtensions() {
 return {
 L".lz4", L".tbz", L".txz", L".tzst",
 L".xar", L".ar", L".chm",
 L".raw", L".ptx", L".r3d",
 L".rgb", L".rgba",
 L".dcm", L".dicom",
 L".dpx", L".cin"
 };
 }

 /// Total registered count
 size_t RegisteredCount() const { return m_registeredSet.size(); }

 /// Total supported count
 size_t TotalCount() const { return m_entries.size(); }

 /// Category name for audit reports
 static const wchar_t* CategoryName(int idx) {
 static const wchar_t* names[] = {
 L"Archives", L"Images", L"ModernImages", L"RAWPhotos",
 L"Documents", L"Fonts", L"3DModels", L"eBooks",
 L"Scientific", L"Film", L"Data", L"Vector"
 };
 return (idx >= 0 && idx < 12) ? names[idx] : L"Unknown";
 }

 /// Audit result
 struct AuditResult {
 size_t registered = 0;
 size_t supported = 0;
 size_t missing = 0;
 bool synced = false;
 };

 AuditResult RunAudit() const {
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

}} // namespace ExplorerLens::Engine

