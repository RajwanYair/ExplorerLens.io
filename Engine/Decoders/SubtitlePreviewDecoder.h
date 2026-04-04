// SubtitlePreviewDecoder.h — Subtitle File Preview (.srt/.ass/.vtt)
// Copyright (c) 2026 ExplorerLens Project
//
// Parses subtitle files to extract cue timing, text content, and formatting.
// Generates a text-based thumbnail showing subtitle samples over a dark
// background resembling a video player overlay.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SubtitleFormat : uint8_t {
    SRT,
    ASS,
    VTT,
    SSA,
    Unknown
};

struct SubtitleCue
{
    double startTime = 0.0;
    double endTime = 0.0;
    std::wstring text;
    uint32_t index = 0;
};

struct SubtitleFileInfo
{
    SubtitleFormat format = SubtitleFormat::Unknown;
    uint32_t cueCount = 0;
    double totalDuration = 0.0;
    std::wstring title;
    std::wstring language;
    std::vector<SubtitleCue> sampleCues;  // First 5 cues for preview
};

struct SubtitleStats
{
    uint32_t filesProcessed = 0;
    uint64_t totalCuesParsed = 0;
};

class SubtitlePreviewDecoder
{
  public:
    SubtitlePreviewDecoder() = default;
    ~SubtitlePreviewDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"SubtitlePreviewDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".srt" || e == L".ass" || e == L".vtt" || e == L".ssa" || e == L".sub";
    }

    SubtitleFormat DetectFormat(const wchar_t* ext) const
    {
        if (!ext)
            return SubtitleFormat::Unknown;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        if (e == L".srt")
            return SubtitleFormat::SRT;
        if (e == L".ass" || e == L".ssa")
            return SubtitleFormat::ASS;
        if (e == L".vtt")
            return SubtitleFormat::VTT;
        return SubtitleFormat::Unknown;
    }

    /// Parse SRT timestamp "HH:MM:SS,mmm" to seconds.
    double ParseSRTTimestamp(const std::string& ts) const
    {
        if (ts.size() < 12)
            return 0.0;
        int h = std::atoi(ts.c_str());
        int m = std::atoi(ts.c_str() + 3);
        int s = std::atoi(ts.c_str() + 6);
        int ms = std::atoi(ts.c_str() + 9);
        return h * 3600.0 + m * 60.0 + s + ms / 1000.0;
    }

    /// Parse SRT content into file info.
    SubtitleFileInfo ParseSRT(const std::string& content) const
    {
        SubtitleFileInfo info;
        info.format = SubtitleFormat::SRT;
        size_t pos = 0;
        uint32_t cueIndex = 0;

        while (pos < content.size()) {
            // Skip blank lines
            while (pos < content.size() && (content[pos] == '\n' || content[pos] == '\r'))
                pos++;
            if (pos >= content.size())
                break;

            // Read cue index
            size_t lineEnd = content.find('\n', pos);
            if (lineEnd == std::string::npos)
                break;
            pos = lineEnd + 1;

            // Read timestamp line
            lineEnd = content.find('\n', pos);
            if (lineEnd == std::string::npos)
                break;
            std::string tsLine = content.substr(pos, lineEnd - pos);
            pos = lineEnd + 1;

            SubtitleCue cue;
            cue.index = ++cueIndex;
            auto arrowPos = tsLine.find("-->");
            if (arrowPos != std::string::npos) {
                cue.startTime = ParseSRTTimestamp(tsLine.substr(0, arrowPos));
                cue.endTime = ParseSRTTimestamp(tsLine.substr(arrowPos + 4));
            }

            // Read text until blank line
            std::wstring text;
            while (pos < content.size() && content[pos] != '\n' && content[pos] != '\r') {
                lineEnd = content.find('\n', pos);
                if (lineEnd == std::string::npos)
                    lineEnd = content.size();
                for (size_t i = pos; i < lineEnd; ++i)
                    text += static_cast<wchar_t>(static_cast<unsigned char>(content[i]));
                text += L' ';
                pos = lineEnd + 1;
            }
            cue.text = text;

            info.cueCount++;
            if (cue.endTime > info.totalDuration)
                info.totalDuration = cue.endTime;
            if (info.sampleCues.size() < 5)
                info.sampleCues.push_back(cue);
        }
        return info;
    }

    SubtitleStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable SubtitleStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
