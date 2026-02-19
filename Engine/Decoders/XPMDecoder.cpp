//==============================================================================
// XPM (X PixMap) Decoder — Implementation
// Sprint 186: SGI/RGB & Legacy Format Support
// Parses XPM3 format (C source code format with color definitions).
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "XPMDecoder.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>

namespace DarkThumbs::Decoders {

    //==========================================================================
    // Extension check
    //==========================================================================
    bool XPMDecoder::IsXPMExtension(const std::string& ext)
    {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == ".xpm";
    }

    //==========================================================================
    // Extract content between quotes from an XPM line
    //==========================================================================
    std::string XPMDecoder::ExtractQuoted(const std::string& line)
    {
        size_t start = line.find('"');
        if (start == std::string::npos) return "";
        start++;
        size_t end = line.find('"', start);
        if (end == std::string::npos) return "";
        return line.substr(start, end - start);
    }

    //==========================================================================
    // Parse hex digit
    //==========================================================================
    static int HexDigit(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    }

    //==========================================================================
    // Parse color string — supports #RRGGBB, #RGB, and common named colors
    //==========================================================================
    uint32_t XPMDecoder::ParseColor(const std::string& colorStr)
    {
        std::string s = colorStr;
        // Trim whitespace
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();

        // Convert to lowercase for named colors
        std::string lower = s;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        // Transparent / None
        if (lower == "none" || lower == "transparent")
            return 0x00000000; // Fully transparent

        // Hex color
        if (!s.empty() && s[0] == '#') {
            if (s.size() == 7) {
                // #RRGGBB
                uint8_t r = static_cast<uint8_t>(HexDigit(s[1]) * 16 + HexDigit(s[2]));
                uint8_t g = static_cast<uint8_t>(HexDigit(s[3]) * 16 + HexDigit(s[4]));
                uint8_t b = static_cast<uint8_t>(HexDigit(s[5]) * 16 + HexDigit(s[6]));
                return (255u << 24) | (r << 16) | (g << 8) | b; // ARGB
            }
            if (s.size() == 4) {
                // #RGB
                uint8_t r = static_cast<uint8_t>(HexDigit(s[1]) * 17);
                uint8_t g = static_cast<uint8_t>(HexDigit(s[2]) * 17);
                uint8_t b = static_cast<uint8_t>(HexDigit(s[3]) * 17);
                return (255u << 24) | (r << 16) | (g << 8) | b;
            }
            if (s.size() == 13) {
                // #RRRRGGGGBBBB (X11 format, use high bytes)
                uint8_t r = static_cast<uint8_t>(HexDigit(s[1]) * 16 + HexDigit(s[2]));
                uint8_t g = static_cast<uint8_t>(HexDigit(s[5]) * 16 + HexDigit(s[6]));
                uint8_t b = static_cast<uint8_t>(HexDigit(s[9]) * 16 + HexDigit(s[10]));
                return (255u << 24) | (r << 16) | (g << 8) | b;
            }
        }

        // Common named colors (subset)
        if (lower == "black")   return 0xFF000000;
        if (lower == "white")   return 0xFFFFFFFF;
        if (lower == "red")     return 0xFFFF0000;
        if (lower == "green")   return 0xFF00FF00;
        if (lower == "blue")    return 0xFF0000FF;
        if (lower == "yellow")  return 0xFFFFFF00;
        if (lower == "cyan")    return 0xFF00FFFF;
        if (lower == "magenta") return 0xFFFF00FF;
        if (lower == "gray" || lower == "grey") return 0xFF808080;

        // Default: opaque middle gray
        return 0xFF808080;
    }

    //==========================================================================
    // Read info — parse the header line (width height ncolors cpp)
    //==========================================================================
    XPMDecoder::ImageInfo XPMDecoder::ReadInfo(const std::string& filePath) const
    {
        ImageInfo info;
        std::ifstream file(filePath);
        if (!file.is_open()) return info;

        std::string line;
        // Find the header line (first quoted string with 4 numbers)
        while (std::getline(file, line)) {
            std::string content = ExtractQuoted(line);
            if (content.empty()) continue;

            std::istringstream iss(content);
            uint32_t w, h, nc, cpp;
            if (iss >> w >> h >> nc >> cpp) {
                info.width = w;
                info.height = h;
                info.numColors = nc;
                info.charsPerPixel = cpp;
                break;
            }
        }

        return info;
    }

    //==========================================================================
    // Decode XPM3 format
    //==========================================================================
    XPMDecoder::DecodeResult XPMDecoder::Decode(const std::string& filePath,
                                                 uint32_t targetWidth) const
    {
        DecodeResult result;
        std::ifstream file(filePath);
        if (!file.is_open()) {
            result.error = "Cannot open file";
            return result;
        }

        // Read all lines
        std::vector<std::string> lines;
        {
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
        }

        // Find header line
        size_t headerIdx = 0;
        uint32_t w = 0, h = 0, numColors = 0, cpp = 0;
        for (size_t i = 0; i < lines.size(); ++i) {
            std::string content = ExtractQuoted(lines[i]);
            if (content.empty()) continue;

            std::istringstream iss(content);
            if (iss >> w >> h >> numColors >> cpp) {
                headerIdx = i;
                break;
            }
        }

        if (w == 0 || h == 0 || numColors == 0 || cpp == 0) {
            result.error = "Invalid XPM header";
            return result;
        }

        // Sanity limits
        if (w > 4096 || h > 4096 || numColors > 65536 || cpp > 4) {
            result.error = "XPM dimensions too large";
            return result;
        }

        // Parse color table
        std::unordered_map<std::string, uint32_t> colorMap;
        size_t colorStart = headerIdx + 1;
        uint32_t colorsRead = 0;

        for (size_t i = colorStart; i < lines.size() && colorsRead < numColors; ++i) {
            std::string content = ExtractQuoted(lines[i]);
            if (content.size() < cpp) continue;

            std::string key = content.substr(0, cpp);
            // Find color value — look for "c " specifier
            size_t cPos = content.find("\tc ");
            if (cPos == std::string::npos) cPos = content.find(" c ");
            if (cPos != std::string::npos) {
                std::string colorVal = content.substr(cPos + 3);
                // Trim trailing whitespace and comments
                size_t tabPos = colorVal.find('\t');
                if (tabPos != std::string::npos) colorVal = colorVal.substr(0, tabPos);
                colorMap[key] = ParseColor(colorVal);
            }
            else {
                colorMap[key] = 0xFF808080; // Default
            }
            colorsRead++;
        }

        // Parse pixel data
        size_t pixelStart = colorStart + numColors;
        result.width = w;
        result.height = h;
        result.pixelData.resize(static_cast<size_t>(w) * h * 4);

        uint32_t row = 0;
        for (size_t i = pixelStart; i < lines.size() && row < h; ++i) {
            std::string content = ExtractQuoted(lines[i]);
            if (content.size() < static_cast<size_t>(w) * cpp) continue;

            for (uint32_t x = 0; x < w; ++x) {
                std::string key = content.substr(static_cast<size_t>(x) * cpp, cpp);
                uint32_t argb = 0xFF808080;
                auto it = colorMap.find(key);
                if (it != colorMap.end()) argb = it->second;

                size_t off = (static_cast<size_t>(row) * w + x) * 4;
                result.pixelData[off + 0] = static_cast<uint8_t>(argb & 0xFF);         // B
                result.pixelData[off + 1] = static_cast<uint8_t>((argb >> 8) & 0xFF);  // G
                result.pixelData[off + 2] = static_cast<uint8_t>((argb >> 16) & 0xFF); // R
                result.pixelData[off + 3] = static_cast<uint8_t>((argb >> 24) & 0xFF); // A
            }
            row++;
        }

        result.success = (row == h);
        if (!result.success) {
            result.error = "Incomplete pixel data";
        }
        return result;
    }

} // namespace DarkThumbs::Decoders
