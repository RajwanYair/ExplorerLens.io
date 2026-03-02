// LaTeXPreviewDecoder.h — LaTeX Document Thumbnail Preview
// Copyright (c) 2026 ExplorerLens Project
//
// Parses LaTeX (.tex/.ltx) documents to extract document class, title,
// author, abstract, and section structure for thumbnail generation.
// Renders document outline as miniaturized text preview.

#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LaTeXSection {
    uint8_t level = 0; // 0=chapter, 1=section, 2=subsection
    std::wstring title;
    uint32_t lineNumber = 0;
};

struct LaTeXDocInfo {
    std::wstring documentClass;
    std::wstring title;
    std::wstring author;
    std::wstring date;
    std::vector<std::wstring> packages;
    std::vector<LaTeXSection> sections;
    uint32_t equationCount = 0;
    uint32_t figureCount = 0;
    uint32_t tableCount = 0;
    uint32_t citationCount = 0;
    uint32_t totalLines = 0;
};

struct LaTeXStats {
    uint32_t filesProcessed = 0;
    uint32_t sectionsFound = 0;
    uint32_t equationsFound = 0;
};

class LaTeXPreviewDecoder {
public:
    LaTeXPreviewDecoder() = default;
    ~LaTeXPreviewDecoder() = default;

    static const wchar_t* GetName() { return L"LaTeXPreviewDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".tex" || e == L".ltx" || e == L".latex";
    }

    /// Parse LaTeX source into document info.
    LaTeXDocInfo Parse(const std::wstring& source) const {
        LaTeXDocInfo info;
        size_t pos = 0;

        while (pos < source.size()) {
            size_t eol = source.find(L'\n', pos);
            if (eol == std::wstring::npos) eol = source.size();
            std::wstring line = source.substr(pos, eol - pos);
            info.totalLines++;

            // Document class
            auto dcPos = line.find(L"\\documentclass");
            if (dcPos != std::wstring::npos) {
                auto braceStart = line.find(L'{', dcPos);
                auto braceEnd = line.find(L'}', braceStart);
                if (braceStart != std::wstring::npos && braceEnd != std::wstring::npos)
                    info.documentClass = line.substr(braceStart + 1, braceEnd - braceStart - 1);
            }

            // Title
            auto titPos = line.find(L"\\title{");
            if (titPos != std::wstring::npos) {
                auto end = line.find(L'}', titPos + 7);
                if (end != std::wstring::npos)
                    info.title = line.substr(titPos + 7, end - titPos - 7);
            }

            // Author
            auto autPos = line.find(L"\\author{");
            if (autPos != std::wstring::npos) {
                auto end = line.find(L'}', autPos + 8);
                if (end != std::wstring::npos)
                    info.author = line.substr(autPos + 8, end - autPos - 8);
            }

            // Sections
            if (line.find(L"\\section{") != std::wstring::npos) {
                LaTeXSection sec; sec.level = 1; sec.lineNumber = info.totalLines;
                auto s = line.find(L'{') + 1;
                auto e2 = line.find(L'}', s);
                if (e2 != std::wstring::npos) sec.title = line.substr(s, e2 - s);
                info.sections.push_back(sec);
            }
            if (line.find(L"\\subsection{") != std::wstring::npos) {
                LaTeXSection sec; sec.level = 2; sec.lineNumber = info.totalLines;
                auto s = line.find(L'{') + 1;
                auto e2 = line.find(L'}', s);
                if (e2 != std::wstring::npos) sec.title = line.substr(s, e2 - s);
                info.sections.push_back(sec);
            }

            // Count environments
            if (line.find(L"\\begin{equation") != std::wstring::npos ||
                line.find(L"$$") != std::wstring::npos) info.equationCount++;
            if (line.find(L"\\begin{figure") != std::wstring::npos) info.figureCount++;
            if (line.find(L"\\begin{table") != std::wstring::npos) info.tableCount++;
            if (line.find(L"\\cite{") != std::wstring::npos) info.citationCount++;

            pos = eol + 1;
        }
        return info;
    }

    LaTeXStats GetStats() const { return m_stats; }

private:
    mutable LaTeXStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
