//==============================================================================
// DarkThumbs Engine — Sprint 283: Database Preview (SQLite)
// SQLite/database file thumbnail with schema and data grid rendering.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Database engine type
enum class DatabaseEngine : uint8_t {
    SQLite,         // SQLite3 files
    Access,         // MS Access .mdb/.accdb
    DBase,          // dBASE .dbf
    Paradox,        // Borland Paradox
    COUNT
};

/// SQL column type
enum class SQLColumnType : uint8_t {
    Integer,
    Real,
    Text,
    Blob,
    Null,
    DateTime,
    Boolean,
    COUNT
};

/// Database preview style
enum class DatabasePreviewStyle : uint8_t {
    SchemaView,     // Table/column listing
    DataGrid,       // Row/column data grid
    ERDiagram,      // Entity-relationship view
    Statistics,     // Table sizes / row counts
    COUNT
};

/// Database file metadata
struct DatabaseMetadata {
    DatabaseEngine  engine      = DatabaseEngine::SQLite;
    uint32_t        tableCount  = 0;
    uint32_t        viewCount   = 0;
    uint32_t        indexCount  = 0;
    uint64_t        totalRows   = 0;
    uint64_t        fileSizeBytes = 0;
    std::wstring    sqliteVersion;
};

/// Database preview config
struct DatabasePreviewConfig {
    DatabasePreviewStyle style  = DatabasePreviewStyle::SchemaView;
    uint32_t maxTables          = 10;
    uint32_t maxColumnsPerTable = 8;
    uint32_t maxPreviewRows     = 15;
    bool     showTypes          = true;
    bool     showRowCounts      = true;
};

/// Database preview decoder
class DatabasePreviewDecoder {
public:
    static const wchar_t* EngineName(DatabaseEngine e) {
        switch (e) {
            case DatabaseEngine::SQLite:   return L"SQLite";
            case DatabaseEngine::Access:   return L"Access";
            case DatabaseEngine::DBase:    return L"dBASE";
            case DatabaseEngine::Paradox:  return L"Paradox";
            default: return L"Unknown";
        }
    }

    static const wchar_t* ColumnTypeName(SQLColumnType t) {
        switch (t) {
            case SQLColumnType::Integer:  return L"INTEGER";
            case SQLColumnType::Real:     return L"REAL";
            case SQLColumnType::Text:     return L"TEXT";
            case SQLColumnType::Blob:     return L"BLOB";
            case SQLColumnType::Null:     return L"NULL";
            case SQLColumnType::DateTime: return L"DATETIME";
            case SQLColumnType::Boolean:  return L"BOOLEAN";
            default: return L"Unknown";
        }
    }

    static const wchar_t* PreviewStyleName(DatabasePreviewStyle s) {
        switch (s) {
            case DatabasePreviewStyle::SchemaView: return L"Schema View";
            case DatabasePreviewStyle::DataGrid:   return L"Data Grid";
            case DatabasePreviewStyle::ERDiagram:  return L"ER Diagram";
            case DatabasePreviewStyle::Statistics: return L"Statistics";
            default: return L"Unknown";
        }
    }

    /// SQLite magic: "SQLite format 3\000"
    static bool CheckSQLiteMagic(const uint8_t* data, size_t size) {
        if (size < 16) return false;
        const char magic[] = "SQLite format 3";
        for (int i = 0; i < 15; ++i)
            if (data[i] != static_cast<uint8_t>(magic[i])) return false;
        return data[15] == 0;
    }

    static constexpr size_t EngineCount() { return static_cast<size_t>(DatabaseEngine::COUNT); }
    static constexpr size_t ColumnTypeCount() { return static_cast<size_t>(SQLColumnType::COUNT); }
    static constexpr size_t PreviewStyleCount() { return static_cast<size_t>(DatabasePreviewStyle::COUNT); }
};

}} // namespace DarkThumbs::Engine
