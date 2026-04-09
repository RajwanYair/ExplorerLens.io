// IFCEntityCounter.cpp — IFC4/IFC2x3 Entity-Type Frequency Summariser
// Copyright (c) 2026 ExplorerLens Project
//
#include "IFCEntityCounter.h"
#include <cstring>
#include <cctype>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

bool IFCEntityCounter::IsIFC(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 20) return false;
    const char* p = reinterpret_cast<const char*>(data);
    if (memcmp(p, "ISO-10303-21", 12) != 0) return false;
    // Also require FILE_SCHEMA with IFC marker somewhere in first 4 KB
    const size_t scanLen = std::min(size, size_t(4096));
    for (size_t i = 0; i + 10 < scanLen; ++i)
    {
        if (memcmp(p + i, "FILE_SCHEMA", 11) == 0)
        {
            // Look for ('IFC within the next 64 chars
            for (size_t j = i; j < i + 64 && j < scanLen; ++j)
            {
                if (p[j] == '\'' && j + 3 < scanLen &&
                    p[j+1] == 'I' && p[j+2] == 'F' && p[j+3] == 'C')
                    return true;
            }
        }
    }
    return false;
}

std::string IFCEntityCounter::DetectVersion(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 20) return "UNKNOWN";
    const char* p = reinterpret_cast<const char*>(data);
    const size_t scan = std::min(size, size_t(8192));
    for (size_t i = 0; i + 12 < scan; ++i)
    {
        if (p[i] == '\'' && p[i+1] == 'I' && p[i+2] == 'F' && p[i+3] == 'C')
        {
            // Extract up to the closing quote
            size_t j = i + 1;
            char buf[32]{};
            size_t n = 0;
            while (j < scan && p[j] != '\'' && n < 31)
                buf[n++] = p[j++];
            if (n > 2) return std::string(buf);
        }
    }
    return "IFC2X3";
}

IFCFileSummary IFCEntityCounter::Count(const uint8_t* data, size_t size, uint32_t topN) noexcept
{
    IFCFileSummary s{};
    s.valid = false;
    if (!IsIFC(data, size)) return s;

    s.ifcVersion = DetectVersion(data, size);

    std::unordered_map<std::string, uint32_t> counts;
    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    // Every entity line starts with "#NNN=IFC" in the DATA section
    const char* data_sec = nullptr;
    for (const char* q = p; q + 5 < end; ++q)
    {
        if (memcmp(q, "DATA;", 5) == 0) { data_sec = q; break; }
    }
    if (!data_sec) return s;

    for (const char* q = data_sec; q < end; ++q)
    {
        if (*q != '#') continue;
        // Skip '#' and numeric ID
        const char* id = q + 1;
        while (id < end && (uint8_t)(*id - '0') < 10u) ++id;
        if (id >= end || *id != '=') { q = id; continue; }
        ++id; // skip '='
        if (id + 3 >= end || id[0] != 'I' || id[1] != 'F' || id[2] != 'C')
        { q = id; continue; }

        // Read entity name
        const char* name_start = id;
        const char* name_end   = id;
        while (name_end < end && *name_end != '(' && *name_end != ';' && *name_end != '\n')
            ++name_end;
        if (name_end - name_start > 0 && name_end - name_start < 64)
        {
            std::string name(name_start, name_end);
            // Uppercase
            for (auto& c : name) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
            ++counts[name];
        }
        q = name_end;
    }

    s.totalEntities = 0;
    for (const auto& kv : counts) s.totalEntities += kv.second;

    // Sort by count descending
    std::vector<IFCEntityCount> sorted;
    sorted.reserve(counts.size());
    for (const auto& kv : counts)
        sorted.push_back({ kv.first, kv.second });
    std::sort(sorted.begin(), sorted.end(),
              [](const IFCEntityCount& a, const IFCEntityCount& b){ return a.count > b.count; });

    const uint32_t keep = std::min(static_cast<uint32_t>(sorted.size()), topN);
    s.topEntities.assign(sorted.begin(), sorted.begin() + keep);
    s.uniqueValues = static_cast<uint32_t>(counts.size());
    s.valid = true;
    return s;
}

std::vector<uint8_t> IFCEntityCounter::RenderBarChart(
    const IFCFileSummary& summary, uint32_t width, uint32_t height)
{
    const uint32_t pixCount = width * height;
    std::vector<uint8_t> bgra(static_cast<size_t>(pixCount) * 4, 0);

    // Dark background
    for (uint32_t i = 0; i < pixCount; ++i)
    {
        bgra[i*4+0] = 0x1E; bgra[i*4+1] = 0x1E;
        bgra[i*4+2] = 0x1E; bgra[i*4+3] = 0xFF;
    }

    if (!summary.valid || summary.topEntities.empty()) return bgra;

    const uint32_t maxCount = summary.topEntities.front().count;
    if (maxCount == 0) return bgra;

    const size_t n = std::min(summary.topEntities.size(), size_t(8));
    const uint32_t barH   = (height - 20) / static_cast<uint32_t>(n);
    const uint32_t maxBarW = width - 60;

    // Colour palette for bars
    static const uint8_t colours[8][3] = {
        {0x42,0x9E,0xF4}, {0x5C,0xC4,0x5E}, {0xF5,0xA6,0x23},
        {0xE7,0x5A,0x5A}, {0xA3,0x6A,0xC8}, {0x4B,0xC8,0xC8},
        {0xF5,0xD4,0x23}, {0xB0,0xB0,0xB0}
    };

    for (size_t i = 0; i < n; ++i)
    {
        const uint32_t barLen = static_cast<uint32_t>(
            (uint64_t)maxBarW * summary.topEntities[i].count / maxCount);
        const uint32_t yStart = 10 + static_cast<uint32_t>(i) * barH;
        const uint32_t yEnd   = yStart + barH - 2;
        const size_t   ci     = i % 8;

        for (uint32_t y = yStart; y < yEnd && y < height; ++y)
            for (uint32_t x = 5; x < 5 + barLen && x < width; ++x)
            {
                uint32_t idx = (y * width + x) * 4;
                bgra[idx+0] = colours[ci][0]; // B
                bgra[idx+1] = colours[ci][1]; // G
                bgra[idx+2] = colours[ci][2]; // R
                bgra[idx+3] = 0xFF;
            }
    }
    return bgra;
}

} // namespace Engine
} // namespace ExplorerLens
