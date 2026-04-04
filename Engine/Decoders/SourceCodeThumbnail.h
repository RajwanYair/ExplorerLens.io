// SourceCodeThumbnail.h — Source Code Syntax Highlighting Thumbnail
// Copyright (c) 2026 ExplorerLens Project
//
// Generates miniaturized source code previews with language detection and
// basic syntax category colorization. Renders first N lines as thumbnail
// with monospace layout and keyword highlighting.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SourceLanguage : uint8_t {
    Unknown,
    CPP,
    CSharp,
    Java,
    Python,
    JavaScript,
    TypeScript,
    Rust,
    Go,
    Ruby,
    PHP,
    Swift,
    Kotlin,
    Lua,
    Shell,
    SQL,
    HTML,
    CSS,
    YAML,
    JSON
};

struct SourceCodeInfo
{
    SourceLanguage language = SourceLanguage::Unknown;
    uint32_t totalLines = 0;
    uint32_t codeLines = 0;
    uint32_t commentLines = 0;
    uint32_t blankLines = 0;
    uint32_t importCount = 0;
    uint32_t functionCount = 0;
};

struct SourceCodeStats
{
    uint32_t filesProcessed = 0;
    uint32_t languagesDetected = 0;
    uint64_t totalLinesScanned = 0;
};

class SourceCodeThumbnail
{
  public:
    SourceCodeThumbnail()
    {
        InitExtensionMap();
    }
    ~SourceCodeThumbnail() = default;

    static const wchar_t* GetName()
    {
        return L"SourceCodeThumbnail";
    }

    bool CanRender(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return m_extToLang.count(e) > 0;
    }

    SourceLanguage DetectLanguage(const wchar_t* ext) const
    {
        if (!ext)
            return SourceLanguage::Unknown;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        auto it = m_extToLang.find(e);
        return (it != m_extToLang.end()) ? it->second : SourceLanguage::Unknown;
    }

    /// Analyze source code to produce metrics.
    SourceCodeInfo Analyze(const std::wstring& source, SourceLanguage lang) const
    {
        SourceCodeInfo info;
        info.language = lang;
        size_t pos = 0;
        bool inBlockComment = false;

        while (pos < source.size()) {
            size_t eol = source.find(L'\n', pos);
            if (eol == std::wstring::npos)
                eol = source.size();
            std::wstring line = source.substr(pos, eol - pos);
            info.totalLines++;

            // Trim whitespace
            auto trimmed = line;
            while (!trimmed.empty() && (trimmed.front() == L' ' || trimmed.front() == L'\t'))
                trimmed.erase(trimmed.begin());

            if (trimmed.empty()) {
                info.blankLines++;
            } else if (inBlockComment || trimmed.find(L"//") == 0 || trimmed.find(L"#") == 0) {
                info.commentLines++;
                if (trimmed.find(L"*/") != std::wstring::npos)
                    inBlockComment = false;
            } else {
                info.codeLines++;
                if (trimmed.find(L"/*") != std::wstring::npos)
                    inBlockComment = true;
                if (trimmed.find(L"import ") == 0 || trimmed.find(L"#include") == 0 || trimmed.find(L"using ") == 0
                    || trimmed.find(L"require(") != std::wstring::npos)
                    info.importCount++;
                if (trimmed.find(L"function ") != std::wstring::npos || trimmed.find(L"def ") == 0
                    || trimmed.find(L"fn ") == 0 || trimmed.find(L"func ") == 0
                    || trimmed.find(L"void ") != std::wstring::npos)
                    info.functionCount++;
            }

            pos = eol + 1;
        }
        return info;
    }

    uint32_t GetSupportedExtensionCount() const
    {
        return static_cast<uint32_t>(m_extToLang.size());
    }

    SourceCodeStats GetStats() const
    {
        return m_stats;
    }

  private:
    void InitExtensionMap()
    {
        m_extToLang[L".cpp"] = SourceLanguage::CPP;
        m_extToLang[L".c"] = SourceLanguage::CPP;
        m_extToLang[L".h"] = SourceLanguage::CPP;
        m_extToLang[L".hpp"] = SourceLanguage::CPP;
        m_extToLang[L".cs"] = SourceLanguage::CSharp;
        m_extToLang[L".java"] = SourceLanguage::Java;
        m_extToLang[L".py"] = SourceLanguage::Python;
        m_extToLang[L".js"] = SourceLanguage::JavaScript;
        m_extToLang[L".ts"] = SourceLanguage::TypeScript;
        m_extToLang[L".rs"] = SourceLanguage::Rust;
        m_extToLang[L".go"] = SourceLanguage::Go;
        m_extToLang[L".rb"] = SourceLanguage::Ruby;
        m_extToLang[L".php"] = SourceLanguage::PHP;
        m_extToLang[L".swift"] = SourceLanguage::Swift;
        m_extToLang[L".kt"] = SourceLanguage::Kotlin;
        m_extToLang[L".lua"] = SourceLanguage::Lua;
        m_extToLang[L".sh"] = SourceLanguage::Shell;
        m_extToLang[L".sql"] = SourceLanguage::SQL;
        m_extToLang[L".html"] = SourceLanguage::HTML;
        m_extToLang[L".css"] = SourceLanguage::CSS;
        m_extToLang[L".yaml"] = SourceLanguage::YAML;
        m_extToLang[L".yml"] = SourceLanguage::YAML;
        m_extToLang[L".json"] = SourceLanguage::JSON;
    }

    std::unordered_map<std::wstring, SourceLanguage> m_extToLang;
    mutable SourceCodeStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
