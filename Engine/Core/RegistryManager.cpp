#include "RegistryManager.h"
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

RegistryManager::RegistryManager() = default;

const wchar_t* RegistryManager::GetHiveName(RegHive hive)
{
    switch (hive) {
        case RegHive::HKCU:
            return L"HKCU";
        case RegHive::HKLM:
            return L"HKLM";
        case RegHive::HKCR:
            return L"HKCR";
        default:
            return L"Unknown";
    }
}

const wchar_t* RegistryManager::GetValueTypeName(RegValueType type)
{
    switch (type) {
        case RegValueType::String:
            return L"REG_SZ";
        case RegValueType::DWord:
            return L"REG_DWORD";
        case RegValueType::QWord:
            return L"REG_QWORD";
        case RegValueType::Binary:
            return L"REG_BINARY";
        case RegValueType::MultiSz:
            return L"REG_MULTI_SZ";
        default:
            return L"Unknown";
    }
}

std::wstring RegistryManager::MakeKey(RegHive hive, const std::wstring& key, const std::wstring& name) const
{
    return std::to_wstring(static_cast<uint32_t>(hive)) + L"\\" + key + L"\\" + name;
}

bool RegistryManager::WriteString(RegHive hive, const std::wstring& key, const std::wstring& name,
                                  const std::wstring& value)
{
    // Update existing or add new
    for (auto& e : m_entries) {
        if (e.hive == hive && e.key == key && e.name == name) {
            e.value = value;
            return true;
        }
    }
    RegEntry entry;
    entry.hive = hive;
    entry.key = key;
    entry.name = name;
    entry.value = value;
    entry.type = RegValueType::String;
    m_entries.push_back(std::move(entry));
    return true;
}

std::wstring RegistryManager::ReadString(RegHive hive, const std::wstring& key, const std::wstring& name,
                                         const std::wstring& defaultVal) const
{
    for (const auto& e : m_entries) {
        if (e.hive == hive && e.key == key && e.name == name) {
            return e.value;
        }
    }
    return defaultVal;
}

bool RegistryManager::DeleteValue(RegHive hive, const std::wstring& key, const std::wstring& name)
{
    auto it = std::remove_if(m_entries.begin(), m_entries.end(),
                             [&](const RegEntry& e) { return e.hive == hive && e.key == key && e.name == name; });
    if (it == m_entries.end())
        return false;
    m_entries.erase(it, m_entries.end());
    return true;
}

}  // namespace Engine
}  // namespace ExplorerLens
