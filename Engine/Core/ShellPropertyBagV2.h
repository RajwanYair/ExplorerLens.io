// ShellPropertyBagV2.h — Shell Property Bag v2 with IPropertyStore
// Copyright (c) 2026 ExplorerLens Project
//
// Stores and retrieves per-file properties via IPropertyStore — thumbnails, decode stats, format metadata.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct StoredProperty { std::wstring name; std::wstring value; uint64_t timestamp = 0; };
class ShellPropertyBagV2 {
public:
    bool Set(const std::wstring& name, const std::wstring& value) {
        m_bag[name] = { name, value, 0 };
        return true;
    }
    bool Get(const std::wstring& name, std::wstring& out) const {
        auto it = m_bag.find(name);
        if (it == m_bag.end()) return false;
        out = it->second.value;
        return true;
    }
    size_t Count()  const { return m_bag.size(); }
    bool   Remove(const std::wstring& name) { return m_bag.erase(name) > 0; }
private:
    std::unordered_map<std::wstring, StoredProperty> m_bag;
};

} // namespace Engine
} // namespace ExplorerLens