// SearchIndexBridge.h — Windows Search IFilter Integration Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges ExplorerLens to Windows Search indexer via IFilter — exposes image metadata as indexed properties.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct IndexProperty { std::wstring key; std::wstring value; };
class SearchIndexBridge {
public:
    void   AddProperty(IndexProperty prop)       { m_props.push_back(std::move(prop)); }
    size_t PropertyCount() const                 { return m_props.size(); }
    bool   IsIndexingEnabled() const             { return m_enabled; }
    void   SetEnabled(bool en)                   { m_enabled = en; }
    std::vector<IndexProperty> QueryFile(const std::wstring& path) const {
        (void)path; return m_props;
    }
private:
    bool                       m_enabled = true;
    std::vector<IndexProperty> m_props;
};

} // namespace Engine
} // namespace ExplorerLens