// RegistryBatchHandler.h — Batch Win32 Registry Operations
// Copyright (c) 2026 ExplorerLens Project
//
// Provides transactional batch registry operations with rollback support.
// Used by COM registration and shell extension configuration.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class RegistryOp : uint8_t {
    CreateKey = 0,
    DeleteKey = 1,
    SetValue = 2,
    DeleteValue = 3
};

enum class RegistryRootKey : uint8_t {
    HKCR = 0,
    HKCU = 1,
    HKLM = 2
};

struct RegistryChange
{
    RegistryOp operation = RegistryOp::SetValue;
    RegistryRootKey rootKey = RegistryRootKey::HKCU;
    std::wstring subKeyPath;
    std::wstring valueName;
    std::wstring valueData;
    DWORD valueType = REG_SZ;
    bool applied = false;
    bool canRollback = true;
};

struct RegistryBatchResult
{
    uint32_t totalOps = 0;
    uint32_t successCount = 0;
    uint32_t failCount = 0;
    uint32_t rollbackOps = 0;
    bool committed = false;
};

class RegistryBatchHandler
{
  public:
    static RegistryBatchHandler& Instance()
    {
        static RegistryBatchHandler s;
        return s;
    }

    void AddChange(const RegistryChange& change)
    {
        m_changes.push_back(change);
    }

    void AddCreateKey(RegistryRootKey root, const std::wstring& subKey)
    {
        RegistryChange c;
        c.operation = RegistryOp::CreateKey;
        c.rootKey = root;
        c.subKeyPath = subKey;
        m_changes.push_back(c);
    }

    void AddSetValue(RegistryRootKey root, const std::wstring& subKey, const std::wstring& name,
                     const std::wstring& data)
    {
        RegistryChange c;
        c.operation = RegistryOp::SetValue;
        c.rootKey = root;
        c.subKeyPath = subKey;
        c.valueName = name;
        c.valueData = data;
        c.valueType = REG_SZ;
        m_changes.push_back(c);
    }

    RegistryBatchResult Execute()
    {
        RegistryBatchResult result{};
        result.totalOps = static_cast<uint32_t>(m_changes.size());
        for (auto& change : m_changes) {
            HKEY root = MapRoot(change.rootKey);
            bool ok = false;
            switch (change.operation) {
                case RegistryOp::CreateKey:
                    ok = DoCreateKey(root, change);
                    break;
                case RegistryOp::SetValue:
                    ok = DoSetValue(root, change);
                    break;
                case RegistryOp::DeleteKey:
                    ok = DoDeleteKey(root, change);
                    break;
                case RegistryOp::DeleteValue:
                    ok = DoDeleteValue(root, change);
                    break;
            }
            change.applied = ok;
            if (ok)
                ++result.successCount;
            else
                ++result.failCount;
        }
        result.committed = (result.failCount == 0);
        return result;
    }

    uint32_t Rollback()
    {
        uint32_t rolled = 0;
        for (auto it = m_changes.rbegin(); it != m_changes.rend(); ++it) {
            if (it->applied && it->canRollback) {
                HKEY root = MapRoot(it->rootKey);
                if (it->operation == RegistryOp::CreateKey) {
                    ::RegDeleteKeyW(root, it->subKeyPath.c_str());
                    ++rolled;
                } else if (it->operation == RegistryOp::SetValue) {
                    HKEY hk = nullptr;
                    if (::RegOpenKeyExW(root, it->subKeyPath.c_str(), 0, KEY_SET_VALUE, &hk) == ERROR_SUCCESS) {
                        ::RegDeleteValueW(hk, it->valueName.c_str());
                        ::RegCloseKey(hk);
                        ++rolled;
                    }
                }
                it->applied = false;
            }
        }
        return rolled;
    }

    void Clear()
    {
        m_changes.clear();
    }
    size_t PendingCount() const
    {
        return m_changes.size();
    }

    bool Validate() const
    {
        for (const auto& c : m_changes) {
            if (c.subKeyPath.empty())
                return false;
            if (c.operation == RegistryOp::SetValue && c.valueName.empty())
                return false;
        }
        return true;
    }

  private:
    RegistryBatchHandler() = default;
    ~RegistryBatchHandler() = default;
    RegistryBatchHandler(const RegistryBatchHandler&) = delete;
    RegistryBatchHandler& operator=(const RegistryBatchHandler&) = delete;

    static HKEY MapRoot(RegistryRootKey rk)
    {
        switch (rk) {
            case RegistryRootKey::HKCR:
                return HKEY_CLASSES_ROOT;
            case RegistryRootKey::HKLM:
                return HKEY_LOCAL_MACHINE;
            default:
                return HKEY_CURRENT_USER;
        }
    }

    bool DoCreateKey(HKEY root, const RegistryChange& c)
    {
        HKEY hk = nullptr;
        LONG r = ::RegCreateKeyExW(root, c.subKeyPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr,
                                   &hk, nullptr);
        if (r == ERROR_SUCCESS && hk) {
            ::RegCloseKey(hk);
            return true;
        }
        return false;
    }

    bool DoSetValue(HKEY root, const RegistryChange& c)
    {
        HKEY hk = nullptr;
        LONG r = ::RegOpenKeyExW(root, c.subKeyPath.c_str(), 0, KEY_SET_VALUE, &hk);
        if (r != ERROR_SUCCESS)
            return false;
        r = ::RegSetValueExW(hk, c.valueName.c_str(), 0, c.valueType, reinterpret_cast<const BYTE*>(c.valueData.c_str()),
                             static_cast<DWORD>((c.valueData.size() + 1) * sizeof(wchar_t)));
        ::RegCloseKey(hk);
        return r == ERROR_SUCCESS;
    }

    bool DoDeleteKey(HKEY root, const RegistryChange& c)
    {
        return ::RegDeleteKeyW(root, c.subKeyPath.c_str()) == ERROR_SUCCESS;
    }

    bool DoDeleteValue(HKEY root, const RegistryChange& c)
    {
        HKEY hk = nullptr;
        if (::RegOpenKeyExW(root, c.subKeyPath.c_str(), 0, KEY_SET_VALUE, &hk) != ERROR_SUCCESS)
            return false;
        LONG r = ::RegDeleteValueW(hk, c.valueName.c_str());
        ::RegCloseKey(hk);
        return r == ERROR_SUCCESS;
    }

    std::vector<RegistryChange> m_changes;
};

}  // namespace Engine
}  // namespace ExplorerLens
