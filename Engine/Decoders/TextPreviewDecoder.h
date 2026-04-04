//==============================================================================
// ExplorerLens Engine — Markdown/Code Preview Decoder
// Custom text→bitmap renderer with syntax highlighting for .md, .txt, .c,
// .cpp, .py, .js, .ts, .json, .xml, .yaml source file thumbnails.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Supported text preview language categories
enum class TextLanguage : uint8_t {
    PlainText,
    Markdown,
    C,
    CPP,
    Python,
    JavaScript,
    TypeScript,
    JSON,
    XML,
    YAML,
    HTML,
    CSS,
    PowerShell,
    Batch,
    INI,
    SQL,
    Rust,
    Go,
    Java,
    CSharp,
    Unknown
};

/// Color scheme for syntax highlighting
struct SyntaxColor
{
    uint32_t keyword = 0xFF569CD6u;     // Blue
    uint32_t string = 0xFFCE9178u;      // Orange
    uint32_t comment = 0xFF6A9955u;     // Green
    uint32_t number = 0xFFB5CEA8u;      // Light green
    uint32_t type = 0xFF4EC9B0u;        // Teal
    uint32_t function = 0xFFDCDCAAu;    // Yellow
    uint32_t operator_ = 0xFFD4D4D4u;   // Light gray
    uint32_t background = 0xFF1E1E1Eu;  // Dark background
    uint32_t foreground = 0xFFD4D4D4u;  // Default text
    uint32_t lineNumber = 0xFF858585u;  // Gray line numbers
};

/// Text preview rendering settings
struct TextPreviewConfig
{
    uint32_t maxLines = 40;        // Max lines to render
    uint32_t maxLineLength = 120;  // Max chars per line
    uint32_t fontSize = 10;        // Font size in points
    uint32_t lineSpacing = 14;     // Line height pixels
    uint32_t marginLeft = 45;      // Left margin for line numbers
    uint32_t marginTop = 8;        // Top margin
    bool showLineNumbers = true;
    bool syntaxHighlight = true;
    bool wordWrap = false;
    SyntaxColor colors;
};

/// Text preview decoder
class TextPreviewDecoder
{
  public:
    /// Detect language from file extension
    static TextLanguage DetectLanguage(const std::wstring& ext)
    {
        static const std::unordered_map<std::wstring, TextLanguage> extMap = {
            {L".txt", TextLanguage::PlainText}, {L".md", TextLanguage::Markdown},
            {L".c", TextLanguage::C},           {L".h", TextLanguage::C},
            {L".cpp", TextLanguage::CPP},       {L".hpp", TextLanguage::CPP},
            {L".cxx", TextLanguage::CPP},       {L".cc", TextLanguage::CPP},
            {L".py", TextLanguage::Python},     {L".pyw", TextLanguage::Python},
            {L".js", TextLanguage::JavaScript}, {L".mjs", TextLanguage::JavaScript},
            {L".ts", TextLanguage::TypeScript}, {L".tsx", TextLanguage::TypeScript},
            {L".json", TextLanguage::JSON},     {L".xml", TextLanguage::XML},
            {L".yaml", TextLanguage::YAML},     {L".yml", TextLanguage::YAML},
            {L".html", TextLanguage::HTML},     {L".htm", TextLanguage::HTML},
            {L".css", TextLanguage::CSS},       {L".ps1", TextLanguage::PowerShell},
            {L".bat", TextLanguage::Batch},     {L".cmd", TextLanguage::Batch},
            {L".ini", TextLanguage::INI},       {L".cfg", TextLanguage::INI},
            {L".sql", TextLanguage::SQL},       {L".rs", TextLanguage::Rust},
            {L".go", TextLanguage::Go},         {L".java", TextLanguage::Java},
            {L".cs", TextLanguage::CSharp}};
        auto it = extMap.find(ext);
        return (it != extMap.end()) ? it->second : TextLanguage::Unknown;
    }

    /// Language display name
    static const wchar_t* LanguageName(TextLanguage lang)
    {
        switch (lang) {
            case TextLanguage::PlainText:
                return L"Plain Text";
            case TextLanguage::Markdown:
                return L"Markdown";
            case TextLanguage::C:
                return L"C";
            case TextLanguage::CPP:
                return L"C++";
            case TextLanguage::Python:
                return L"Python";
            case TextLanguage::JavaScript:
                return L"JavaScript";
            case TextLanguage::TypeScript:
                return L"TypeScript";
            case TextLanguage::JSON:
                return L"JSON";
            case TextLanguage::XML:
                return L"XML";
            case TextLanguage::YAML:
                return L"YAML";
            case TextLanguage::HTML:
                return L"HTML";
            case TextLanguage::CSS:
                return L"CSS";
            case TextLanguage::PowerShell:
                return L"PowerShell";
            case TextLanguage::Batch:
                return L"Batch";
            case TextLanguage::INI:
                return L"INI";
            case TextLanguage::SQL:
                return L"SQL";
            case TextLanguage::Rust:
                return L"Rust";
            case TextLanguage::Go:
                return L"Go";
            case TextLanguage::Java:
                return L"Java";
            case TextLanguage::CSharp:
                return L"C#";
            default:
                return L"Unknown";
        }
    }

    /// Count of supported languages
    static constexpr size_t LanguageCount()
    {
        return 20;
    }

    /// Count of supported extensions
    static size_t ExtensionCount()
    {
        return 31;
    }

    /// Check if extension is a text preview candidate
    static bool IsTextFile(const std::wstring& ext)
    {
        return DetectLanguage(ext) != TextLanguage::Unknown;
    }

    /// Validate config ranges
    static bool ValidateConfig(const TextPreviewConfig& cfg)
    {
        if (cfg.maxLines == 0 || cfg.maxLines > 200)
            return false;
        if (cfg.maxLineLength == 0 || cfg.maxLineLength > 500)
            return false;
        if (cfg.fontSize < 6 || cfg.fontSize > 32)
            return false;
        if (cfg.lineSpacing < cfg.fontSize)
            return false;
        return true;
    }

    /// Calculate thumbnail height for given line count
    static uint32_t CalculateHeight(const TextPreviewConfig& cfg, uint32_t lineCount)
    {
        uint32_t lines = (lineCount < cfg.maxLines) ? lineCount : cfg.maxLines;
        return cfg.marginTop + (lines * cfg.lineSpacing) + cfg.marginTop;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
