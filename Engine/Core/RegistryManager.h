#pragma once
// Registry Manager — Windows registry read/write for settings persistence
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Registry hive
enum class RegHive : uint32_t {
    HKCU = 0,  ///< HKEY_CURRENT_USER
    HKLM = 1,  ///< HKEY_LOCAL_MACHINE
    HKCR = 2,  ///< HKEY_CLASSES_ROOT
    COUNT = 3
};

/// Registry value type
enum class RegValueType : uint32_t {
    String = 0,
    DWord = 1,
    QWord = 2,
    Binary = 3,
    MultiSz = 4,
    COUNT = 5
};

/// A registry value entry
struct RegEntry
{
    std::wstring key;
    std::wstring name;
    std::wstring value;
    RegValueType type = RegValueType::String;
    RegHive hive = RegHive::HKCU;
};

/// Manages Windows registry operations for application settings
class RegistryManager
{
  public:
    RegistryManager();

    static const wchar_t* GetHiveName(RegHive hive);
    static const wchar_t* GetValueTypeName(RegValueType type);
    static uint32_t GetHiveCount()
    {
        return static_cast<uint32_t>(RegHive::COUNT);
    }

    /// Write a string value (in-memory simulation)
    bool WriteString(RegHive hive, const std::wstring& key, const std::wstring& name, const std::wstring& value);
    /// Read a string value
    std::wstring ReadString(RegHive hive, const std::wstring& key, const std::wstring& name,
                            const std::wstring& defaultVal = L"") const;
    /// Delete a value
    bool DeleteValue(RegHive hive, const std::wstring& key, const std::wstring& name);
    /// Get all entries
    const std::vector<RegEntry>& GetEntries() const
    {
        return m_entries;
    }

    /// Get ExplorerLens registry base path
    static std::wstring GetBasePath()
    {
        return L"SOFTWARE\\ExplorerLens";
    }

  private:
    std::vector<RegEntry> m_entries;
    std::wstring MakeKey(RegHive hive, const std::wstring& key, const std::wstring& name) const;
};

}  // namespace Engine
}  // namespace ExplorerLens
