// CodeSyntaxThumbnail.h — Code File Syntax Token Classification
// Copyright (c) 2026 ExplorerLens Project
//
// Provides lightweight token classification and syntax-colour mapping for
// generating code-file thumbnails.  Handles language detection by file
// extension and keyword lookup.  Does not perform full parsing — works at
// the lexeme/token level only.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Broad token categories used for colouring.
enum class SyntaxTokenType : uint8_t {
    Keyword,
    Type,
    String,
    Comment,
    Number,
    Operator,
    Identifier,
    Preprocessor,
    Punctuation,
    Whitespace,
    Unknown
};

/// Recognised language identifiers.
enum class CodeLanguage : uint8_t {
    Cpp,
    CSharp,
    Java,
    Python,
    JavaScript,
    TypeScript,
    Rust,
    Go,
    Html,
    Css,
    Json,
    Xml,
    Sql,
    Shell,
    Unknown
};

/// ARGB packed colour type for syntax highlighting.
using CodeSyntaxColor = uint32_t;

class CodeSyntaxThumbnail
{
  public:
    CodeSyntaxThumbnail()
    {
        InitLanguageMap();
        InitKeywords();
        InitTokenColors();
    }

    // ── Language detection ────────────────────────────────────────────

    /// Return the language for a given file extension (with or without dot).
    CodeLanguage Classify(const std::wstring& extension) const
    {
        std::wstring ext = NormaliseExtension(extension);
        auto it = m_extensionMap.find(ext);
        return (it != m_extensionMap.end()) ? it->second : CodeLanguage::Unknown;
    }

    /// Check whether the extension maps to a supported language.
    bool IsSupportedLanguage(const std::wstring& extension) const
    {
        return Classify(extension) != CodeLanguage::Unknown;
    }

    /// Human-readable name for a language.
    std::wstring GetLanguageName(CodeLanguage lang) const
    {
        switch (lang) {
            case CodeLanguage::Cpp:
                return L"C/C++";
            case CodeLanguage::CSharp:
                return L"C#";
            case CodeLanguage::Java:
                return L"Java";
            case CodeLanguage::Python:
                return L"Python";
            case CodeLanguage::JavaScript:
                return L"JavaScript";
            case CodeLanguage::TypeScript:
                return L"TypeScript";
            case CodeLanguage::Rust:
                return L"Rust";
            case CodeLanguage::Go:
                return L"Go";
            case CodeLanguage::Html:
                return L"HTML";
            case CodeLanguage::Css:
                return L"CSS";
            case CodeLanguage::Json:
                return L"JSON";
            case CodeLanguage::Xml:
                return L"XML";
            case CodeLanguage::Sql:
                return L"SQL";
            case CodeLanguage::Shell:
                return L"Shell";
            default:
                return L"Unknown";
        }
    }

    // ── Keyword queries ──────────────────────────────────────────────

    /// Number of keywords registered for the given language.
    uint32_t GetKeywordCount(CodeLanguage lang) const
    {
        auto it = m_keywords.find(lang);
        return (it != m_keywords.end()) ? static_cast<uint32_t>(it->second.size()) : 0;
    }

    /// Check whether `word` is a keyword in the given language.
    bool IsKeyword(CodeLanguage lang, const std::wstring& word) const
    {
        auto it = m_keywords.find(lang);
        if (it == m_keywords.end())
            return false;
        return it->second.count(word) > 0;
    }

    // ── Token colour mapping ─────────────────────────────────────────

    /// ARGB colour for a token type (dark-theme defaults).
    CodeSyntaxColor GetColorForToken(SyntaxTokenType type) const
    {
        auto it = m_tokenColors.find(type);
        return (it != m_tokenColors.end()) ? it->second : 0xFFD4D4D4;
    }

    /// Classify a single lexeme into a token type given the language context.
    SyntaxTokenType ClassifyToken(CodeLanguage lang, const std::wstring& token) const
    {
        if (token.empty())
            return SyntaxTokenType::Whitespace;
        if (token[0] == L'#')
            return SyntaxTokenType::Preprocessor;
        if (token[0] == L'"' || token[0] == L'\'')
            return SyntaxTokenType::String;
        if (token.size() >= 2 && token[0] == L'/' && (token[1] == L'/' || token[1] == L'*'))
            return SyntaxTokenType::Comment;
        if (IsNumber(token))
            return SyntaxTokenType::Number;
        if (IsOperator(token))
            return SyntaxTokenType::Operator;
        if (IsPunctuation(token))
            return SyntaxTokenType::Punctuation;
        if (IsKeyword(lang, token))
            return SyntaxTokenType::Keyword;
        if (IsTypeName(token))
            return SyntaxTokenType::Type;
        return SyntaxTokenType::Identifier;
    }

    /// Total number of supported file extensions.
    uint32_t GetSupportedExtensionCount() const
    {
        return static_cast<uint32_t>(m_extensionMap.size());
    }

  private:
    std::unordered_map<std::wstring, CodeLanguage> m_extensionMap;
    std::unordered_map<CodeLanguage, std::unordered_set<std::wstring>> m_keywords;
    std::unordered_map<SyntaxTokenType, CodeSyntaxColor> m_tokenColors;

    // ── Initialisation helpers ───────────────────────────────────────

    void InitLanguageMap()
    {
        auto add = [&](const wchar_t* ext, CodeLanguage lang) {
            m_extensionMap[ext] = lang;
        };
        add(L".cpp", CodeLanguage::Cpp);
        add(L".cxx", CodeLanguage::Cpp);
        add(L".cc", CodeLanguage::Cpp);
        add(L".c", CodeLanguage::Cpp);
        add(L".h", CodeLanguage::Cpp);
        add(L".hpp", CodeLanguage::Cpp);
        add(L".cs", CodeLanguage::CSharp);
        add(L".java", CodeLanguage::Java);
        add(L".py", CodeLanguage::Python);
        add(L".js", CodeLanguage::JavaScript);
        add(L".mjs", CodeLanguage::JavaScript);
        add(L".ts", CodeLanguage::TypeScript);
        add(L".tsx", CodeLanguage::TypeScript);
        add(L".rs", CodeLanguage::Rust);
        add(L".go", CodeLanguage::Go);
        add(L".html", CodeLanguage::Html);
        add(L".htm", CodeLanguage::Html);
        add(L".css", CodeLanguage::Css);
        add(L".json", CodeLanguage::Json);
        add(L".xml", CodeLanguage::Xml);
        add(L".sql", CodeLanguage::Sql);
        add(L".sh", CodeLanguage::Shell);
        add(L".bash", CodeLanguage::Shell);
        add(L".ps1", CodeLanguage::Shell);
    }

    void InitKeywords()
    {
        m_keywords[CodeLanguage::Cpp] = {
            L"if",      L"else",     L"for",    L"while",     L"do",        L"switch",   L"case",    L"break",
            L"return",  L"class",    L"struct", L"namespace", L"template",  L"typename", L"const",   L"static",
            L"virtual", L"override", L"public", L"private",   L"protected", L"new",      L"delete",  L"void",
            L"int",     L"float",    L"double", L"bool",      L"auto",      L"using",    L"typedef", L"enum",
            L"sizeof",  L"nullptr",  L"true",   L"false",     L"constexpr", L"noexcept", L"inline",  L"extern"};
        m_keywords[CodeLanguage::Python] = {
            L"def",   L"class", L"if",  L"elif",   L"else",    L"for",   L"while", L"return", L"import", L"from",
            L"as",    L"with",  L"try", L"except", L"finally", L"raise", L"pass",  L"lambda", L"yield",  L"True",
            L"False", L"None",  L"and", L"or",     L"not",     L"in",    L"is",    L"async",  L"await"};
        m_keywords[CodeLanguage::JavaScript] = {L"function", L"var",    L"let",     L"const",     L"if",    L"else",
                                                L"for",      L"while",  L"return",  L"class",     L"new",   L"this",
                                                L"import",   L"export", L"default", L"async",     L"await", L"try",
                                                L"catch",    L"throw",  L"typeof",  L"instanceof"};
        m_keywords[CodeLanguage::Rust] = {L"fn",    L"let",   L"mut",    L"if",     L"else",   L"for",
                                          L"while", L"loop",  L"match",  L"return", L"struct", L"enum",
                                          L"impl",  L"trait", L"pub",    L"use",    L"mod",    L"self",
                                          L"super", L"crate", L"unsafe", L"async",  L"await"};
    }

    void InitTokenColors()
    {
        m_tokenColors[SyntaxTokenType::Keyword] = 0xFF569CD6;       // blue
        m_tokenColors[SyntaxTokenType::Type] = 0xFF4EC9B0;          // teal
        m_tokenColors[SyntaxTokenType::String] = 0xFFCE9178;        // orange
        m_tokenColors[SyntaxTokenType::Comment] = 0xFF6A9955;       // green
        m_tokenColors[SyntaxTokenType::Number] = 0xFFB5CEA8;        // light green
        m_tokenColors[SyntaxTokenType::Operator] = 0xFFD4D4D4;      // grey
        m_tokenColors[SyntaxTokenType::Identifier] = 0xFF9CDCFE;    // light blue
        m_tokenColors[SyntaxTokenType::Preprocessor] = 0xFFC586C0;  // purple
        m_tokenColors[SyntaxTokenType::Punctuation] = 0xFFD4D4D4;   // grey
    }

    static std::wstring NormaliseExtension(const std::wstring& ext)
    {
        std::wstring out = ext;
        if (!out.empty() && out[0] != L'.')
            out.insert(out.begin(), L'.');
        std::transform(out.begin(), out.end(), out.begin(), ::towlower);
        return out;
    }

    static bool IsNumber(const std::wstring& t)
    {
        if (t.empty())
            return false;
        for (wchar_t c : t)
            if (!std::iswdigit(c) && c != L'.' && c != L'x' && c != L'X' && c != L'f' && c != L'F')
                return false;
        return std::iswdigit(t[0]) != 0;
    }

    static bool IsOperator(const std::wstring& t)
    {
        static const std::unordered_set<std::wstring> ops = {
            L"+", L"-", L"*", L"/", L"%", L"=",  L"==", L"!=", L"<",  L">",  L"<=", L">=", L"&&", L"||",
            L"!", L"&", L"|", L"^", L"~", L"<<", L">>", L"->", L"::", L"+=", L"-=", L"*=", L"/="};
        return ops.count(t) > 0;
    }

    static bool IsPunctuation(const std::wstring& t)
    {
        if (t.size() != 1)
            return false;
        return t[0] == L'(' || t[0] == L')' || t[0] == L'{' || t[0] == L'}' || t[0] == L'[' || t[0] == L']'
               || t[0] == L';' || t[0] == L',' || t[0] == L'.';
    }

    static bool IsTypeName(const std::wstring& t)
    {
        // Heuristic: starts with uppercase, has lowercase after
        if (t.size() < 2)
            return false;
        return std::iswupper(t[0]) && std::iswlower(t[1]);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
