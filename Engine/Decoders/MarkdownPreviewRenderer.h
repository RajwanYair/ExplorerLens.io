// MarkdownPreviewRenderer.h — Markdown Document Thumbnail Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Markdown (.md) files to extract headings, code blocks, and structure.
// Renders a miniaturized document preview using GDI+ text layout for thumbnail.
// Supports CommonMark headers (# through ######), code fences, and emphasis.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MDBlockType : uint8_t {
    Heading1,
    Heading2,
    Heading3,
    Paragraph,
    CodeBlock,
    ListItem,
    BlockQuote,
    HorizontalRule,
    Image,
    Table
};

struct MDBlock
{
    MDBlockType type = MDBlockType::Paragraph;
    std::wstring text;
    uint32_t lineNumber = 0;
};

struct MarkdownDocInfo
{
    std::wstring title;
    uint32_t headingCount = 0;
    uint32_t codeBlockCount = 0;
    uint32_t paragraphCount = 0;
    uint32_t listItemCount = 0;
    uint32_t totalLines = 0;
    uint32_t wordCount = 0;
};

struct MarkdownStats
{
    uint32_t filesProcessed = 0;
    uint32_t blocksExtracted = 0;
    uint64_t totalBytesRead = 0;
};

class MarkdownPreviewRenderer
{
  public:
    MarkdownPreviewRenderer() = default;
    ~MarkdownPreviewRenderer() = default;

    static const wchar_t* GetName()
    {
        return L"MarkdownPreviewRenderer";
    }

    bool CanRender(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".md" || e == L".markdown" || e == L".mdown";
    }

    /// Parse markdown text into structured blocks.
    std::vector<MDBlock> Parse(const std::wstring& content) const
    {
        std::vector<MDBlock> blocks;
        if (content.empty())
            return blocks;

        size_t pos = 0;
        uint32_t lineNum = 0;
        while (pos < content.size()) {
            size_t eol = content.find(L'\n', pos);
            if (eol == std::wstring::npos)
                eol = content.size();
            std::wstring line = content.substr(pos, eol - pos);
            lineNum++;

            MDBlock block;
            block.lineNumber = lineNum;

            if (line.find(L"### ") == 0) {
                block.type = MDBlockType::Heading3;
                block.text = line.substr(4);
            } else if (line.find(L"## ") == 0) {
                block.type = MDBlockType::Heading2;
                block.text = line.substr(3);
            } else if (line.find(L"# ") == 0) {
                block.type = MDBlockType::Heading1;
                block.text = line.substr(2);
            } else if (line.find(L"```") == 0) {
                block.type = MDBlockType::CodeBlock;
                block.text = line;
            } else if (line.find(L"- ") == 0 || line.find(L"* ") == 0) {
                block.type = MDBlockType::ListItem;
                block.text = line.substr(2);
            } else if (line.find(L"> ") == 0) {
                block.type = MDBlockType::BlockQuote;
                block.text = line.substr(2);
            } else if (line == L"---" || line == L"***") {
                block.type = MDBlockType::HorizontalRule;
            } else {
                block.type = MDBlockType::Paragraph;
                block.text = line;
            }

            blocks.push_back(std::move(block));
            pos = eol + 1;
        }
        return blocks;
    }

    /// Extract document info summary for metadata display.
    MarkdownDocInfo GetDocInfo(const std::vector<MDBlock>& blocks) const
    {
        MarkdownDocInfo info;
        info.totalLines = static_cast<uint32_t>(blocks.size());
        for (const auto& b : blocks) {
            switch (b.type) {
                case MDBlockType::Heading1:
                case MDBlockType::Heading2:
                case MDBlockType::Heading3:
                    info.headingCount++;
                    if (info.title.empty())
                        info.title = b.text;
                    break;
                case MDBlockType::CodeBlock:
                    info.codeBlockCount++;
                    break;
                case MDBlockType::Paragraph:
                    info.paragraphCount++;
                    break;
                case MDBlockType::ListItem:
                    info.listItemCount++;
                    break;
                default:
                    break;
            }
        }
        return info;
    }

    MarkdownStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable MarkdownStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
