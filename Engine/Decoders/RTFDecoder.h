// RTFDecoder.h — Rich Text Format Document Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Parses RTF {\rtf1...} documents to extract plain text, font table,
// and color table for thumbnail rendering. Strips RTF control words
// and groups to produce clean text for miniature preview.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct RTFDocInfo
{
    uint32_t version = 0;
    std::wstring charset;
    uint32_t fontCount = 0;
    uint32_t colorCount = 0;
    uint32_t paragraphCount = 0;
    uint32_t wordCount = 0;
    uint32_t imageCount = 0;
    uint64_t rawSize = 0;
};

struct RTFStats
{
    uint32_t filesDecoded = 0;
    uint32_t totalWords = 0;
    uint64_t totalBytesProcessed = 0;
};

class RTFDecoder
{
  public:
    RTFDecoder() = default;
    ~RTFDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"RTFDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".rtf";
    }

    /// Detect RTF magic: {\rtf
    bool DetectMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 5)
            return false;
        return data[0] == '{' && data[1] == '\\' && data[2] == 'r' && data[3] == 't' && data[4] == 'f';
    }

    /// Strip RTF control words to extract plain text (first maxChars characters).
    std::wstring ExtractPlainText(const std::string& rtf, size_t maxChars = 2000) const
    {
        std::wstring result;
        result.reserve(maxChars);
        int groupDepth = 0;
        bool skipGroup = false;
        size_t i = 0;

        while (i < rtf.size() && result.size() < maxChars) {
            char c = rtf[i];
            if (c == '{') {
                groupDepth++;
                // Skip \fonttbl, \colortbl, \pict groups
                if (i + 8 < rtf.size()) {
                    std::string peek = rtf.substr(i, 10);
                    if (peek.find("\\fonttbl") != std::string::npos || peek.find("\\colortbl") != std::string::npos
                        || peek.find("\\pict") != std::string::npos)
                        skipGroup = true;
                }
                i++;
            } else if (c == '}') {
                groupDepth--;
                if (groupDepth <= 1)
                    skipGroup = false;
                i++;
            } else if (skipGroup) {
                i++;
            } else if (c == '\\') {
                // Skip control word
                i++;
                if (i < rtf.size() && rtf[i] == '\'') {
                    i += 3;
                    continue;
                }
                std::string ctrl;
                while (i < rtf.size() && ((rtf[i] >= 'a' && rtf[i] <= 'z') || (rtf[i] >= 'A' && rtf[i] <= 'Z')))
                    ctrl += rtf[i++];
                // Skip parameter
                while (i < rtf.size() && ((rtf[i] >= '0' && rtf[i] <= '9') || rtf[i] == '-'))
                    i++;
                if (i < rtf.size() && rtf[i] == ' ')
                    i++;
                if (ctrl == "par" || ctrl == "line")
                    result += L'\n';
                if (ctrl == "tab")
                    result += L'\t';
            } else {
                result += static_cast<wchar_t>(static_cast<unsigned char>(c));
                i++;
            }
        }
        return result;
    }

    /// Parse document info from RTF header.
    RTFDocInfo ParseDocInfo(const std::string& rtf) const
    {
        RTFDocInfo info;
        info.rawSize = rtf.size();
        if (rtf.size() >= 6 && rtf[5] >= '0' && rtf[5] <= '9')
            info.version = rtf[5] - '0';

        // Count font entries
        size_t pos = 0;
        while ((pos = rtf.find("\\f", pos)) != std::string::npos) {
            if (pos + 2 < rtf.size() && rtf[pos + 2] >= '0' && rtf[pos + 2] <= '9')
                info.fontCount++;
            pos += 2;
        }
        info.fontCount = std::min(info.fontCount, 100u);  // Sanity cap

        return info;
    }

    RTFStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable RTFStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
