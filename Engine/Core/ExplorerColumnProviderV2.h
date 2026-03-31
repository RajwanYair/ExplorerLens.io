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

// Column identifier for Explorer Details view metadata columns
enum class ColumnID : uint32_t {
    FileName    = 0,
    Dimensions  = 1,
    ColorSpace  = 2,
    CameraModel = 3,
    FileFormat  = 4,
    FileSize    = 5,
    DateTaken   = 6,
    COUNT       = 7
};

// A typed metadata column value with optional numeric representation
struct ColumnValue {
    ColumnID     id           = ColumnID::FileName;
    std::wstring stringValue;
    uint64_t     numericValue = 0;
    bool         isAvailable  = false;

    static ColumnValue Make(ColumnID cid, const wchar_t* strVal, uint64_t numVal) {
        ColumnValue v;
        v.id           = cid;
        v.stringValue  = strVal ? strVal : L"";
        v.numericValue = numVal;
        v.isAvailable  = true;
        return v;
    }
};

// A column definition entry for the Explorer Details column set
struct ColumnDefinition {
    const wchar_t* displayName   = nullptr;
    ColumnID       id            = ColumnID::FileName;
    uint32_t       defaultWidth  = 100;
    bool           sortable      = true;
};

// Returns the built-in column definition table (null-terminated sentinel not needed; use ColumnID::COUNT)
inline const ColumnDefinition* GetColumnDefinitions() noexcept {
    static const ColumnDefinition defs[] = {
        { L"Dimensions",  ColumnID::Dimensions,  100, true },
        { L"Color Space", ColumnID::ColorSpace,   90, true },
        { L"Camera",      ColumnID::CameraModel, 120, true },
        { L"Format",      ColumnID::FileFormat,   80, true },
        { L"File Size",   ColumnID::FileSize,      80, true },
        { L"Date Taken",  ColumnID::DateTaken,   100, true },
    };
    return defs;
}

} // namespace Engine
} // namespace ExplorerLens
