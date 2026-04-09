// KiCadNetlistParser.cpp — KiCad 8 S-Expression Netlist & Schematic Parser
// Copyright (c) 2026 ExplorerLens Project
//
#include "KiCadNetlistParser.h"
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

bool KiCadNetlistParser::IsKiCad(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 12) return false;
    const char* p = reinterpret_cast<const char*>(data);
    if (p[0] != '(') return false;
    const char* keywords[] = {
        "kicad_sch", "kicad_pcb", "kicad_mod", "kicad_pro", nullptr
    };
    for (int i = 0; keywords[i]; ++i)
    {
        const size_t klen = strlen(keywords[i]);
        if (size > klen + 1 && memcmp(p + 1, keywords[i], klen) == 0) return true;
    }
    return false;
}

KiCadFileType KiCadNetlistParser::DetectFileType(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 12) return KiCadFileType::Unknown;
    const char* p = reinterpret_cast<const char*>(data);
    if (memcmp(p + 1, "kicad_sch", 9) == 0) return KiCadFileType::Schematic;
    if (memcmp(p + 1, "kicad_pcb", 9) == 0) return KiCadFileType::PCBLayout;
    if (memcmp(p + 1, "kicad_mod", 9) == 0) return KiCadFileType::Footprint;
    if (memcmp(p + 1, "kicad_pro", 9) == 0) return KiCadFileType::Project;
    return KiCadFileType::Unknown;
}

// Helper: extract a quoted string starting at *pos, advance past closing quote
static std::string extractQuoted(const char* p, size_t size, size_t& pos) noexcept
{
    if (pos >= size || p[pos] != '"') return {};
    ++pos; // skip opening quote
    std::string result;
    while (pos < size && p[pos] != '"')
    {
        if (p[pos] == '\\' && pos + 1 < size) { ++pos; }
        result += p[pos++];
    }
    if (pos < size) ++pos; // skip closing quote
    return result;
}

KiCadFileSummary KiCadNetlistParser::Parse(
    const uint8_t* data, size_t size, uint32_t maxComponents) noexcept
{
    KiCadFileSummary s{};
    s.valid = false;
    if (!IsKiCad(data, size)) return s;

    s.fileType = DetectFileType(data, size);
    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    std::unordered_map<std::string, uint32_t> valueCounts;

    // Scan for "(symbol (lib_id ...) (property "Reference" "U1") (property "Value" "STM32"))"
    // and "(footprint ..." patterns in PCB layouts
    uint32_t compCount = 0;
    for (const char* q = p;
         q + 8 < end && compCount < maxComponents; ++q)
    {
        // Match "(symbol " in schematic or "(footprint " in PCB
        bool isSymbol = false, isFootprint = false;
        if (q[0] == '(' && memcmp(q+1, "symbol ", 7) == 0)     isSymbol    = true;
        if (q[0] == '(' && memcmp(q+1, "footprint ", 10) == 0) isFootprint = true;

        if (!isSymbol && !isFootprint) continue;

        // Scan forward within this S-expression for "property" entries
        KiCadComponent comp;
        int depth = 1;
        const char* r = q + 1;
        while (r < end && depth > 0)
        {
            if (*r == '(') ++depth;
            else if (*r == ')') --depth;
            // Look for (property "Reference" "R1") or (property "Value" "10k")
            if (*r == '(' && r + 10 < end && memcmp(r+1, "property ", 9) == 0)
            {
                size_t pos = static_cast<size_t>(r + 10 - p);
                while (pos < size && p[pos] == ' ') ++pos; // skip spaces
                std::string propName = extractQuoted(p, size, pos);
                while (pos < size && p[pos] == ' ') ++pos;
                std::string propVal  = extractQuoted(p, size, pos);

                if (propName == "Reference") comp.reference = propVal;
                else if (propName == "Value") comp.value = propVal;
                else if (propName == "Footprint") comp.footprint = propVal;
            }
            ++r;
        }

        if (!comp.reference.empty() || !comp.value.empty())
        {
            s.components.push_back(comp);
            if (!comp.value.empty()) ++valueCounts[comp.value];
            ++compCount;
        }
        q = r - 1;
    }

    // Board dimensions: look for "(gr_rect (start x y) (end x y))" or "(rect (start"
    for (const char* q = p; q + 14 < end; ++q)
    {
        if (memcmp(q, "(gr_rect", 8) == 0 || memcmp(q, "(rect (s", 8) == 0)
        {
            // Just extract first pair of numbers as approximate size
            double nums[4]{};
            int ni = 0;
            const char* r = q;
            while (r < end && r < q + 200 && ni < 4)
            {
                if (*r == '-' || (*r >= '0' && *r <= '9'))
                {
                    char* ep = nullptr;
                    double v = strtod(r, &ep);
                    if (ep != r) { nums[ni++] = v; r = ep; continue; }
                }
                ++r;
            }
            if (ni >= 4)
            {
                s.boardWidthMM  = std::abs(nums[2] - nums[0]);
                s.boardHeightMM = std::abs(nums[3] - nums[1]);
                break;
            }
        }
    }

    s.uniqueValues = static_cast<uint32_t>(valueCounts.size());
    s.valid = true;
    return s;
}

std::vector<uint8_t> KiCadNetlistParser::RenderPieChart(
    const KiCadFileSummary& summary, uint32_t width, uint32_t height)
{
    const uint32_t pixCount = width * height;
    std::vector<uint8_t> bgra(static_cast<size_t>(pixCount) * 4, 0);

    // Dark background
    for (uint32_t i = 0; i < pixCount; ++i)
    {
        bgra[i*4+0]=0x18; bgra[i*4+1]=0x18;
        bgra[i*4+2]=0x18; bgra[i*4+3]=0xFF;
    }

    if (!summary.valid || summary.components.empty()) return bgra;

    // Count by first letter of reference (R, C, L, U, Q, J → categories)
    std::unordered_map<char, uint32_t> catCounts;
    for (const auto& c : summary.components)
    {
        if (!c.reference.empty())
        {
            char cat = static_cast<char>(toupper(static_cast<unsigned char>(c.reference[0])));
            ++catCounts[cat];
        }
    }

    // Build sorted slices
    std::vector<std::pair<char, uint32_t>> slices(catCounts.begin(), catCounts.end());
    std::sort(slices.begin(), slices.end(),
              [](const std::pair<char,uint32_t>& a, const std::pair<char,uint32_t>& b)
              { return a.second > b.second; });

    uint32_t total = 0;
    for (const auto& s : slices) total += s.second;
    if (total == 0) return bgra;

    static const uint8_t colours[8][3] = {
        {0x40,0xA0,0xF0}, {0x50,0xC8,0x50}, {0xF0,0xA0,0x30},
        {0xE0,0x55,0x55}, {0xA0,0x60,0xC8}, {0x45,0xC8,0xC8},
        {0xF0,0xD0,0x25}, {0xB0,0xB0,0xB0}
    };

    const double cx = width  / 2.0;
    const double cy = height / 2.0;
    const double r  = std::min(cx, cy) * 0.85;
    const double PI = 3.14159265358979323846;

    double angle = -PI / 2.0; // start at top
    for (size_t si = 0; si < slices.size() && si < 8; ++si)
    {
        const double sweep = 2.0 * PI * slices[si].second / total;
        const size_t ci    = si % 8;
        // Rasterise the wedge by scanning pixels
        for (uint32_t y = 0; y < height; ++y)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                const double dx = x - cx, dy = y - cy;
                const double dist = sqrt(dx*dx + dy*dy);
                if (dist > r) continue;
                double a = atan2(dy, dx);
                // Normalise to [0, 2*PI] relative to start angle
                double rel = a - angle;
                while (rel < 0)        rel += 2 * PI;
                while (rel > 2 * PI)   rel -= 2 * PI;
                if (rel < sweep)
                {
                    uint32_t idx = (y * width + x) * 4;
                    bgra[idx+0] = colours[ci][0];
                    bgra[idx+1] = colours[ci][1];
                    bgra[idx+2] = colours[ci][2];
                    bgra[idx+3] = 0xFF;
                }
            }
        }
        angle += sweep;
    }

    // Draw thin black separators between slices
    angle = -PI / 2.0;
    for (size_t si = 0; si < slices.size() && si < 8; ++si)
    {
        const double sweep = 2.0 * PI * slices[si].second / total;
        const double ex = cx + r * cos(angle);
        const double ey = cy + r * sin(angle);
        const int steps = static_cast<int>(r);
        for (int i = 0; i <= steps; ++i)
        {
            int px = static_cast<int>(cx + (ex - cx) * i / steps);
            int py = static_cast<int>(cy + (ey - cy) * i / steps);
            if (px >= 0 && px < (int)width && py >= 0 && py < (int)height)
            {
                uint32_t idx = (static_cast<uint32_t>(py) * width + static_cast<uint32_t>(px)) * 4;
                bgra[idx+0]=0; bgra[idx+1]=0; bgra[idx+2]=0; bgra[idx+3]=0xFF;
            }
        }
        angle += sweep;
    }

    return bgra;
}

} // namespace Engine
} // namespace ExplorerLens
