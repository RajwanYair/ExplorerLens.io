// NotationDecoder.h — Music Notation File Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Music notation file decoder for .musicxml and .ly formats. Parses notation
// header, generates staff-line preview thumbnail.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NotationFormat : uint8_t {
    MusicXML,
    LilyPond,
    MusicJSON,
    MIDI,
    ABC,
    Unknown
};

struct NoteInfo
{
    uint8_t pitch = 60;
    uint8_t octave = 4;
    float duration = 1.0f;
    bool isRest = false;
    char noteName = 'C';
};

struct NotationHeader
{
    NotationFormat format = NotationFormat::Unknown;
    std::string title;
    std::string composer;
    std::string timeSignature;
    std::string keySignature;
    uint32_t measureCount = 0;
    uint32_t noteCount = 0;
    uint16_t tempo = 120;
    bool isValid = false;
};

class NotationDecoder
{
  public:
    static NotationDecoder& Instance()
    {
        static NotationDecoder instance;
        return instance;
    }

    inline NotationFormat DetectFormat(const uint8_t* data, size_t size) const
    {
        if (!data || size < 4)
            return NotationFormat::Unknown;

        if (data[0] == 0x4D && data[1] == 0x54 && data[2] == 0x68 && data[3] == 0x64)
            return NotationFormat::MIDI;

        std::string content(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(512)));
        if (content.find("<?xml") != std::string::npos && content.find("score-partwise") != std::string::npos)
            return NotationFormat::MusicXML;
        if (content.find("\\version") != std::string::npos || content.find("\\relative") != std::string::npos)
            return NotationFormat::LilyPond;
        if (content.find("X:") != std::string::npos && content.find("K:") != std::string::npos)
            return NotationFormat::ABC;
        return NotationFormat::Unknown;
    }

    inline NotationHeader ParseHeader(const uint8_t* data, size_t size) const
    {
        NotationHeader header;
        header.format = DetectFormat(data, size);
        header.isValid = header.format != NotationFormat::Unknown;
        if (!header.isValid)
            return header;

        std::string content(reinterpret_cast<const char*>(data), (std::min)(size, static_cast<size_t>(4096)));

        if (header.format == NotationFormat::MusicXML) {
            header.title = ExtractXMLTag(content, "work-title");
            if (header.title.empty())
                header.title = ExtractXMLTag(content, "movement-title");
            header.composer = ExtractXMLTag(content, "creator");
            header.timeSignature = ExtractXMLTag(content, "beats") + "/" + ExtractXMLTag(content, "beat-type");
        } else if (header.format == NotationFormat::LilyPond) {
            header.title = ExtractLilyPondField(content, "title");
            header.composer = ExtractLilyPondField(content, "composer");
        }

        return header;
    }

    inline std::vector<uint8_t> GenerateStaffPreview(const std::vector<NoteInfo>& notes, uint32_t width, uint32_t height,
                                                     uint8_t bgColor = 245, uint8_t lineColor = 40) const
    {
        std::vector<uint8_t> thumbnail(static_cast<size_t>(width) * height * 3, bgColor);
        if (width == 0 || height == 0)
            return thumbnail;

        float staffTop = height * 0.25f;
        float staffHeight = height * 0.5f;
        float lineSpacing = staffHeight / 4.0f;

        for (int i = 0; i < 5; ++i) {
            int y = static_cast<int>(staffTop + i * lineSpacing);
            DrawHLine(thumbnail.data(), width, height, 10, width - 10, y, lineColor);
        }

        int clefX = 20;
        int clefY = static_cast<int>(staffTop + 2 * lineSpacing);
        DrawDot(thumbnail.data(), width, height, clefX, clefY, 5, 30, 30, 30);
        DrawDot(thumbnail.data(), width, height, clefX, clefY - static_cast<int>(lineSpacing), 3, 30, 30, 30);

        if (!notes.empty()) {
            float noteSpacing = static_cast<float>(width - 80) / (std::max)(static_cast<size_t>(1), notes.size());
            for (size_t i = 0; i < notes.size() && i < 32; ++i) {
                int nx = static_cast<int>(60 + i * noteSpacing);
                int pitchOffset = (notes[i].pitch % 12) - 6;
                int ny = static_cast<int>(staffTop + 2 * lineSpacing - pitchOffset * (lineSpacing / 2.0f));

                if (notes[i].isRest) {
                    DrawRect(thumbnail.data(), width, height, nx - 3, ny - 4, 6, 8, 40, 40, 40);
                } else {
                    bool filled = notes[i].duration <= 0.5f;
                    DrawNoteHead(thumbnail.data(), width, height, nx, ny, static_cast<int>(lineSpacing * 0.35f), filled,
                                 10, 10, 10);

                    if (notes[i].duration <= 2.0f) {
                        int stemDir = ny < static_cast<int>(staffTop + 2 * lineSpacing) ? 1 : -1;
                        int stemLen = static_cast<int>(lineSpacing * 3.0f);
                        DrawVLine(thumbnail.data(), width, height, nx + 3, ny, ny + stemDir * stemLen, 10);
                    }
                }
            }
        }

        return thumbnail;
    }

    inline std::string FormatToString(NotationFormat fmt) const
    {
        switch (fmt) {
            case NotationFormat::MusicXML:
                return "MusicXML";
            case NotationFormat::LilyPond:
                return "LilyPond";
            case NotationFormat::MusicJSON:
                return "MusicJSON";
            case NotationFormat::MIDI:
                return "MIDI";
            case NotationFormat::ABC:
                return "ABC Notation";
            default:
                return "Unknown";
        }
    }

  private:
    NotationDecoder() = default;

    inline std::string ExtractXMLTag(const std::string& xml, const std::string& tag) const
    {
        std::string openTag = "<" + tag;
        size_t start = xml.find(openTag);
        if (start == std::string::npos)
            return "";
        start = xml.find('>', start);
        if (start == std::string::npos)
            return "";
        ++start;
        size_t end = xml.find("</", start);
        if (end == std::string::npos)
            return "";
        return xml.substr(start, end - start);
    }

    inline std::string ExtractLilyPondField(const std::string& content, const std::string& field) const
    {
        size_t pos = content.find(field);
        if (pos == std::string::npos)
            return "";
        pos = content.find('"', pos);
        if (pos == std::string::npos)
            return "";
        size_t end = content.find('"', pos + 1);
        if (end == std::string::npos)
            return "";
        return content.substr(pos + 1, end - pos - 1);
    }

    inline void DrawHLine(uint8_t* p, uint32_t w, uint32_t h, int x0, int x1, int y, uint8_t c) const
    {
        if (y < 0 || y >= static_cast<int>(h))
            return;
        for (int x = (std::max)(0, x0); x <= (std::min)(static_cast<int>(w) - 1, x1); ++x) {
            size_t idx = (static_cast<size_t>(y) * w + x) * 3;
            p[idx] = c;
            p[idx + 1] = c;
            p[idx + 2] = c;
        }
    }

    inline void DrawVLine(uint8_t* p, uint32_t w, uint32_t h, int x, int y0, int y1, uint8_t c) const
    {
        if (x < 0 || x >= static_cast<int>(w))
            return;
        int yStart = (std::min)(y0, y1), yEnd = (std::max)(y0, y1);
        for (int y = (std::max)(0, yStart); y <= (std::min)(static_cast<int>(h) - 1, yEnd); ++y) {
            size_t idx = (static_cast<size_t>(y) * w + x) * 3;
            p[idx] = c;
            p[idx + 1] = c;
            p[idx + 2] = c;
        }
    }

    inline void DrawDot(uint8_t* p, uint32_t w, uint32_t h, int cx, int cy, int r, uint8_t cr, uint8_t cg,
                        uint8_t cb) const
    {
        for (int dy = -r; dy <= r; ++dy)
            for (int dx = -r; dx <= r; ++dx)
                if (dx * dx + dy * dy <= r * r) {
                    int px = cx + dx, py = cy + dy;
                    if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                        size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                        p[idx] = cr;
                        p[idx + 1] = cg;
                        p[idx + 2] = cb;
                    }
                }
    }

    inline void DrawRect(uint8_t* p, uint32_t w, uint32_t h, int x, int y, int rw, int rh, uint8_t cr, uint8_t cg,
                         uint8_t cb) const
    {
        for (int dy = 0; dy < rh; ++dy)
            for (int dx = 0; dx < rw; ++dx) {
                int px = x + dx, py = y + dy;
                if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                    size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                    p[idx] = cr;
                    p[idx + 1] = cg;
                    p[idx + 2] = cb;
                }
            }
    }

    inline void DrawNoteHead(uint8_t* p, uint32_t w, uint32_t h, int cx, int cy, int r, bool filled, uint8_t cr,
                             uint8_t cg, uint8_t cb) const
    {
        for (int dy = -r; dy <= r; ++dy)
            for (int dx = -r - 1; dx <= r + 1; ++dx) {
                float ex = static_cast<float>(dx) / (r + 1);
                float ey = static_cast<float>(dy) / r;
                if (ex * ex + ey * ey <= 1.0f) {
                    bool draw = filled || (ex * ex + ey * ey > 0.6f);
                    if (draw) {
                        int px = cx + dx, py = cy + dy;
                        if (px >= 0 && px < static_cast<int>(w) && py >= 0 && py < static_cast<int>(h)) {
                            size_t idx = (static_cast<size_t>(py) * w + px) * 3;
                            p[idx] = cr;
                            p[idx + 1] = cg;
                            p[idx + 2] = cb;
                        }
                    }
                }
            }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
