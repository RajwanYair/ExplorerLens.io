//==============================================================================
// ExplorerLens Engine — CSV/JSON Preview Decoder
// Structured data thumbnail generator for CSV, JSON, YAML, TOML, XML.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Structured data format
enum class StructuredDataFormat : uint8_t {
    CSV,
    JSON,
    YAML,
    TOML,
    XML,
    INI,
    COUNT
};

/// JSON value type
enum class JSONValueType : uint8_t {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object,
    COUNT
};

/// Data preview style
enum class DataPreviewStyle : uint8_t {
    TreeView,   // Hierarchical tree (JSON/XML)
    TableView,  // Tabular grid (CSV)
    KeyValue,   // Key=value pairs (INI/TOML)
    RawText,    // Formatted raw text
    COUNT
};

/// Structured data preview config
struct StructuredDataConfig
{
    uint32_t maxLines = 30;
    uint32_t maxDepth = 5;
    uint32_t indentSpaces = 2;
    bool syntaxColor = true;
    bool showLineNumbers = false;
    DataPreviewStyle style = DataPreviewStyle::TreeView;
};

/// Structured data preview decoder
class StructuredDataDecoder
{
  public:
    static const wchar_t* FormatName(StructuredDataFormat f)
    {
        switch (f) {
            case StructuredDataFormat::CSV:
                return L"CSV";
            case StructuredDataFormat::JSON:
                return L"JSON";
            case StructuredDataFormat::YAML:
                return L"YAML";
            case StructuredDataFormat::TOML:
                return L"TOML";
            case StructuredDataFormat::XML:
                return L"XML";
            case StructuredDataFormat::INI:
                return L"INI";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* ValueTypeName(JSONValueType t)
    {
        switch (t) {
            case JSONValueType::Null:
                return L"Null";
            case JSONValueType::Boolean:
                return L"Boolean";
            case JSONValueType::Number:
                return L"Number";
            case JSONValueType::String:
                return L"String";
            case JSONValueType::Array:
                return L"Array";
            case JSONValueType::Object:
                return L"Object";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* StyleName(DataPreviewStyle s)
    {
        switch (s) {
            case DataPreviewStyle::TreeView:
                return L"Tree View";
            case DataPreviewStyle::TableView:
                return L"Table View";
            case DataPreviewStyle::KeyValue:
                return L"Key-Value";
            case DataPreviewStyle::RawText:
                return L"Raw Text";
            default:
                return L"Unknown";
        }
    }

    static StructuredDataFormat DetectFormat(const std::wstring& ext)
    {
        if (ext == L".csv")
            return StructuredDataFormat::CSV;
        if (ext == L".json" || ext == L".jsonc")
            return StructuredDataFormat::JSON;
        if (ext == L".yaml" || ext == L".yml")
            return StructuredDataFormat::YAML;
        if (ext == L".toml")
            return StructuredDataFormat::TOML;
        if (ext == L".xml")
            return StructuredDataFormat::XML;
        if (ext == L".ini" || ext == L".cfg")
            return StructuredDataFormat::INI;
        return StructuredDataFormat::JSON;
    }

    static constexpr size_t FormatCount()
    {
        return static_cast<size_t>(StructuredDataFormat::COUNT);
    }
    static constexpr size_t ValueTypeCount()
    {
        return static_cast<size_t>(JSONValueType::COUNT);
    }
    static constexpr size_t StyleCount()
    {
        return static_cast<size_t>(DataPreviewStyle::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
