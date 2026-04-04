// DocumentDigestOverlay.h — Text Digest Overlay for Document Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Generates text digest overlays for document thumbnails by extracting
// summary information from full text content. Pure string processing with
// no external dependencies.
//
#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cwctype>
#include <string>

namespace ExplorerLens {
namespace Engine {

// Generates text digest overlays for document thumbnails.
// Provides word counting, read-time estimation, language hinting,
// and smart text truncation at word boundaries.
class DocumentDigestOverlay
{
  public:
    DocumentDigestOverlay() : m_wordsPerMinute(200) {}

    // Set the full document text for analysis
    void SetText(const std::wstring& fullText)
    {
        m_fullText = fullText;
    }

    // Generate a digest (preview snippet) of at most maxChars characters.
    // Truncates at a word boundary to avoid cutting words.
    std::wstring GenerateDigest(size_t maxChars) const
    {
        if (m_fullText.empty())
            return std::wstring();
        if (maxChars == 0)
            return std::wstring();

        // Strip leading whitespace for the digest
        size_t start = 0;
        while (start < m_fullText.size() && std::iswspace(m_fullText[start])) {
            ++start;
        }

        std::wstring trimmed = m_fullText.substr(start);
        if (trimmed.empty())
            return std::wstring();

        return TruncateAtWord(trimmed, maxChars);
    }

    // Count the number of whitespace-delimited words in the stored text
    size_t GetWordCount() const
    {
        return CountWords(m_fullText);
    }

    // Estimate reading time in seconds based on word count and words-per-minute
    double GetEstimatedReadTime() const
    {
        size_t words = GetWordCount();
        if (words == 0 || m_wordsPerMinute == 0)
            return 0.0;
        return (static_cast<double>(words) / static_cast<double>(m_wordsPerMinute)) * 60.0;
    }

    // Provide a simple language hint based on character analysis.
    // Returns "CJK" for ideographic-heavy text, "Cyrillic" for Cyrillic,
    // "Arabic" for Arabic/Hebrew scripts, or "Latin" as default.
    static std::string GetLanguageHint(const std::wstring& text)
    {
        if (text.empty())
            return "Unknown";

        size_t cjkCount = 0;
        size_t cyrillicCount = 0;
        size_t arabicCount = 0;
        size_t latinCount = 0;
        size_t total = 0;

        const size_t sampleSize = (std::min)(text.size(), static_cast<size_t>(500));

        for (size_t i = 0; i < sampleSize; ++i) {
            wchar_t ch = text[i];
            if (std::iswspace(ch) || std::iswpunct(ch))
                continue;
            ++total;

            if (ch >= 0x4E00 && ch <= 0x9FFF)
                ++cjkCount;  // CJK Unified
            else if (ch >= 0x3040 && ch <= 0x30FF)
                ++cjkCount;  // Hiragana/Katakana
            else if (ch >= 0x0400 && ch <= 0x04FF)
                ++cyrillicCount;  // Cyrillic
            else if (ch >= 0x0600 && ch <= 0x06FF)
                ++arabicCount;  // Arabic
            else if (ch >= 0x0590 && ch <= 0x05FF)
                ++arabicCount;  // Hebrew
            else if ((ch >= 0x0041 && ch <= 0x007A))
                ++latinCount;  // Basic Latin
        }

        if (total == 0)
            return "Unknown";

        double threshold = 0.3;
        if (static_cast<double>(cjkCount) / total > threshold)
            return "CJK";
        if (static_cast<double>(cyrillicCount) / total > threshold)
            return "Cyrillic";
        if (static_cast<double>(arabicCount) / total > threshold)
            return "Arabic";
        return "Latin";
    }

    // Truncate text at a word boundary, appending ellipsis if truncated.
    static std::wstring TruncateAtWord(const std::wstring& text, size_t maxLen)
    {
        if (maxLen < 4)
            maxLen = 4;
        if (text.size() <= maxLen)
            return text;

        // Find last space before maxLen - 3 (room for "...")
        size_t cutoff = maxLen - 3;
        size_t lastSpace = cutoff;

        // Search backward for a word boundary
        while (lastSpace > 0 && !std::iswspace(text[lastSpace])) {
            --lastSpace;
        }

        // If no space found in reasonable range, hard-cut
        if (lastSpace == 0 || lastSpace < cutoff / 2) {
            lastSpace = cutoff;
        }

        return text.substr(0, lastSpace) + L"...";
    }

    // Configuration
    void SetWordsPerMinute(uint32_t wpm)
    {
        m_wordsPerMinute = (std::max)(wpm, 1u);
    }
    uint32_t GetWordsPerMinute() const
    {
        return m_wordsPerMinute;
    }

    // Access stored text
    const std::wstring& GetText() const
    {
        return m_fullText;
    }

  private:
    // Count whitespace-delimited words
    static size_t CountWords(const std::wstring& text)
    {
        if (text.empty())
            return 0;

        size_t count = 0;
        bool inWord = false;

        for (size_t i = 0; i < text.size(); ++i) {
            if (std::iswspace(text[i])) {
                inWord = false;
            } else {
                if (!inWord)
                    ++count;
                inWord = true;
            }
        }
        return count;
    }

    std::wstring m_fullText;
    uint32_t m_wordsPerMinute;
};

}  // namespace Engine
}  // namespace ExplorerLens
