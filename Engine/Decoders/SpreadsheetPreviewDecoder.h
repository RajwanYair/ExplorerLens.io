//==============================================================================
// DarkThumbs Engine — Sprint 277: Spreadsheet/Data Preview
// CSV, Excel, ODS thumbnail generation with cell grid rendering.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Spreadsheet format type
enum class SpreadsheetFormat : uint8_t {
    CSV,            // Comma-separated values
    TSV,            // Tab-separated values
    XLSX,           // Excel Open XML
    XLS,            // Legacy Excel binary
    ODS,            // OpenDocument Spreadsheet
    Numbers,        // Apple Numbers
    COUNT
};

/// Cell data type
enum class CellDataType : uint8_t {
    Text,
    Number,
    Currency,
    Date,
    Boolean,
    Formula,
    Empty,
    COUNT
};

/// Spreadsheet preview config
struct SpreadsheetPreviewConfig {
    uint32_t maxRows        = 20;       // Max visible rows
    uint32_t maxColumns     = 10;       // Max visible columns
    uint32_t cellPaddingPx  = 4;
    uint32_t headerHeight   = 24;
    uint32_t rowHeight      = 20;
    bool     showGridLines  = true;
    bool     showHeaders    = true;
    bool     alternateRows  = true;     // Zebra striping
    uint32_t fontSize       = 11;
};

/// Spreadsheet metadata
struct SpreadsheetMetadata {
    SpreadsheetFormat format    = SpreadsheetFormat::CSV;
    uint32_t    totalRows       = 0;
    uint32_t    totalColumns    = 0;
    uint32_t    sheetCount      = 1;
    std::wstring firstSheetName;
    bool        hasFormulas     = false;
    bool        hasCharts       = false;
};

/// Spreadsheet preview decoder
class SpreadsheetPreviewDecoder {
public:
    static const wchar_t* FormatName(SpreadsheetFormat f) {
        switch (f) {
            case SpreadsheetFormat::CSV:     return L"CSV";
            case SpreadsheetFormat::TSV:     return L"TSV";
            case SpreadsheetFormat::XLSX:    return L"Excel XLSX";
            case SpreadsheetFormat::XLS:     return L"Excel XLS";
            case SpreadsheetFormat::ODS:     return L"OpenDocument";
            case SpreadsheetFormat::Numbers: return L"Numbers";
            default: return L"Unknown";
        }
    }

    static const wchar_t* CellTypeName(CellDataType t) {
        switch (t) {
            case CellDataType::Text:     return L"Text";
            case CellDataType::Number:   return L"Number";
            case CellDataType::Currency: return L"Currency";
            case CellDataType::Date:     return L"Date";
            case CellDataType::Boolean:  return L"Boolean";
            case CellDataType::Formula:  return L"Formula";
            case CellDataType::Empty:    return L"Empty";
            default: return L"Unknown";
        }
    }

    /// File extension to format mapping
    static SpreadsheetFormat DetectFormat(const std::wstring& ext) {
        if (ext == L".csv") return SpreadsheetFormat::CSV;
        if (ext == L".tsv" || ext == L".tab") return SpreadsheetFormat::TSV;
        if (ext == L".xlsx") return SpreadsheetFormat::XLSX;
        if (ext == L".xls") return SpreadsheetFormat::XLS;
        if (ext == L".ods") return SpreadsheetFormat::ODS;
        if (ext == L".numbers") return SpreadsheetFormat::Numbers;
        return SpreadsheetFormat::CSV;
    }

    static constexpr size_t FormatCount() { return static_cast<size_t>(SpreadsheetFormat::COUNT); }
    static constexpr size_t CellTypeCount() { return static_cast<size_t>(CellDataType::COUNT); }
};

}} // namespace DarkThumbs::Engine
