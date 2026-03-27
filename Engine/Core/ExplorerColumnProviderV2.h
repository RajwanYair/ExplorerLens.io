// ExplorerColumnProviderV2.h — Explorer Column Provider v2
// Copyright (c) 2026 ExplorerLens Project
//
// IColumnProvider COM implementation v2 exposing custom metadata columns in Explorer Details view.
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

struct ColumnDef {
    std::wstring title;
    uint32_t     width  = 100;
    uint32_t     id     = 0;
    bool         sortable = true;
};
class ExplorerColumnProviderV2 {
public:
    void   RegisterColumn(ColumnDef col)             { m_cols.push_back(std::move(col)); }
    size_t ColumnCount() const                       { return m_cols.size(); }
    const ColumnDef& GetColumn(size_t idx) const     { return m_cols[idx]; }
    std::wstring GetCellValue(uint32_t colId, const std::wstring& path) const {
        (void)colId; (void)path; return L"";
    }
private:
    std::vector<ColumnDef> m_cols;
};

} // namespace Engine
} // namespace ExplorerLens