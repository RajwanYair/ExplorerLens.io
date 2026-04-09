// STEPBoundingBoxExtractor.cpp — ISO 10303 STEP/IGES Bounding-Box Extractor
// Copyright (c) 2026 ExplorerLens Project
//
#include "STEPBoundingBoxExtractor.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <climits>

namespace ExplorerLens {
namespace Engine {

CADFileFormat STEPBoundingBoxExtractor::DetectFormat(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 16) return CADFileFormat::Unknown;
    const char* p = reinterpret_cast<const char*>(data);
    if (memcmp(p, "ISO-10303-21", 12) == 0) return CADFileFormat::STEP;
    // IGES: lines are exactly 80 chars wide; check for 'S' in column 73 of first line
    if (size > 80 && p[72] == 'S') return CADFileFormat::IGES;
    return CADFileFormat::Unknown;
}

static void UpdateBBox(BoundingBox3D& bb, double x, double y, double z) noexcept
{
    if (!bb.valid)
    {
        bb.minX = bb.maxX = x;
        bb.minY = bb.maxY = y;
        bb.minZ = bb.maxZ = z;
        bb.valid = true;
        return;
    }
    if (x < bb.minX) bb.minX = x; if (x > bb.maxX) bb.maxX = x;
    if (y < bb.minY) bb.minY = y; if (y > bb.maxY) bb.maxY = y;
    if (z < bb.minZ) bb.minZ = z; if (z > bb.maxZ) bb.maxZ = z;
}

BoundingBox3D STEPBoundingBoxExtractor::ExtractSTEP(const uint8_t* data, size_t size) noexcept
{
    BoundingBox3D bb{};
    bb.valid = false;
    if (!data || size < 16) return bb;

    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    // Sub-sample: skip every N-th line for huge files
    const size_t stride = (size / 65536) + 1;
    size_t lineIdx = 0;

    const char* q = p;
    while (q < end)
    {
        // Find line end
        const char* nl = q;
        while (nl < end && *nl != '\n') ++nl;

        ++lineIdx;
        if (lineIdx % stride == 0)
        {
            // Look for CARTESIAN_POINT or VERTEX_POINT coordinate triples
            const char* cp = q;
            while (cp + 20 < nl)
            {
                if ((memcmp(cp, "CARTESIAN_POINT", 15) == 0 ||
                     memcmp(cp, "VERTEX_POINT",   12) == 0))
                {
                    // Find '(' then parse three doubles
                    const char* par = cp;
                    while (par < nl && *par != '(') ++par;
                    ++par; // skip '('
                    // skip optional label string in '' quotes
                    if (par < nl && *par == '\'')
                    {
                        ++par;
                        while (par < nl && *par != '\'') ++par;
                        if (par < nl) ++par; // skip closing quote
                        while (par < nl && (*par == ',' || *par == ' ')) ++par;
                        if (par < nl && *par == '(') ++par; // inner '('
                    }
                    char* ep1 = nullptr; char* ep2 = nullptr; char* ep3 = nullptr;
                    double x = strtod(par, &ep1);
                    if (ep1 != par && ep1 < nl)
                    {
                        const char* np = ep1;
                        while (np < nl && (*np == ',' || *np == ' ')) ++np;
                        double y = strtod(np, &ep2);
                        if (ep2 != np && ep2 < nl)
                        {
                            const char* np2 = ep2;
                            while (np2 < nl && (*np2 == ',' || *np2 == ' ')) ++np2;
                            double z = strtod(np2, &ep3);
                            if (ep3 != np2) UpdateBBox(bb, x, y, z);
                        }
                    }
                    break;
                }
                ++cp;
            }
        }
        q = nl + 1;
    }
    return bb;
}

BoundingBox3D STEPBoundingBoxExtractor::ExtractIGES(const uint8_t* data, size_t size) noexcept
{
    BoundingBox3D bb{};
    bb.valid = false;
    if (!data || size < 80) return bb;

    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    // IGES parameter section: lines with 'P' in column 73
    for (const char* q = p; q + 80 <= end; q += 81)
    {
        if (q[72] != 'P') continue;
        // Entity type is in the entity directory, but we can look for type 116 (POINT)
        // by checking the parameter data. Simple heuristic: parse 3 comma-delimited doubles.
        char* ep1 = nullptr; char* ep2 = nullptr; char* ep3 = nullptr;
        double x = strtod(q, &ep1);
        if (ep1 != q)
        {
            const char* n2 = ep1;
            while (n2 < q + 72 && (*n2 == ',' || *n2 == ' ')) ++n2;
            double y = strtod(n2, &ep2);
            if (ep2 != n2)
            {
                const char* n3 = ep2;
                while (n3 < q + 72 && (*n3 == ',' || *n3 == ' ')) ++n3;
                double z = strtod(n3, &ep3);
                if (ep3 != n3 &&
                    x > -1e9 && x < 1e9 &&
                    y > -1e9 && y < 1e9 &&
                    z > -1e9 && z < 1e9)
                {
                    UpdateBBox(bb, x, y, z);
                }
            }
        }
    }
    return bb;
}

BoundingBox3D STEPBoundingBoxExtractor::Extract(const uint8_t* data, size_t size) noexcept
{
    switch (DetectFormat(data, size))
    {
        case CADFileFormat::STEP: return ExtractSTEP(data, size);
        case CADFileFormat::IGES: return ExtractIGES(data, size);
        default:                  return {};
    }
}

std::vector<uint8_t> STEPBoundingBoxExtractor::RenderBBoxPreview(
    const BoundingBox3D& bbox, uint32_t width, uint32_t height)
{
    const uint32_t pixCount = width * height;
    std::vector<uint8_t> bgra(static_cast<size_t>(pixCount) * 4, 0);

    // Dark grey background
    for (uint32_t i = 0; i < pixCount; ++i)
    {
        bgra[i * 4 + 0] = 0x28;
        bgra[i * 4 + 1] = 0x28;
        bgra[i * 4 + 2] = 0x28;
        bgra[i * 4 + 3] = 0xFF;
    }

    if (!bbox.valid) return bgra;

    // Simple isometric projection: draw a wireframe box outline
    // Map 3D corners onto 2D screen using isometric angles (30 deg)
    // Scale to fill 80% of the image
    const double cosA = 0.866; // cos(30 deg)
    const double sinA = 0.500; // sin(30 deg)
    const double sx = bbox.SizeX(), sy = bbox.SizeY(), sz = bbox.SizeZ();
    const double maxDim = std::max({sx, sy, sz, 1.0});
    const double scale  = (width * 0.4) / maxDim;

    // Project a 3D point to 2D isometric coordinates
    auto project = [&](double x, double y, double z) -> std::pair<int, int>
    {
        const double px = (x - y) * cosA * scale;
        const double py = (x + y) * sinA * scale - z * scale;
        return { static_cast<int>(width  / 2 + px),
                 static_cast<int>(height / 2 + py) };
    };

    // Draw a line in BGRA
    auto drawLine = [&](std::pair<int,int> a, std::pair<int,int> b,
                        uint8_t B, uint8_t G, uint8_t R) noexcept
    {
        int dx = b.first - a.first, dy = b.second - a.second;
        int steps = std::max(std::abs(dx), std::abs(dy));
        if (steps == 0) return;
        for (int i = 0; i <= steps; ++i)
        {
            int px = a.first  + dx * i / steps;
            int py = a.second + dy * i / steps;
            if (px >= 0 && px < (int)width && py >= 0 && py < (int)height)
            {
                uint32_t idx = (static_cast<uint32_t>(py) * width + static_cast<uint32_t>(px)) * 4;
                bgra[idx+0]=B; bgra[idx+1]=G; bgra[idx+2]=R; bgra[idx+3]=0xFF;
            }
        }
    };

    // 8 corners of the bounding box
    const double x0 = 0, x1 = sx, y0 = 0, y1 = sy, z0 = 0, z1 = sz;
    auto p000 = project(x0,y0,z0), p100 = project(x1,y0,z0);
    auto p010 = project(x0,y1,z0), p110 = project(x1,y1,z0);
    auto p001 = project(x0,y0,z1), p101 = project(x1,y0,z1);
    auto p011 = project(x0,y1,z1), p111 = project(x1,y1,z1);

    // Bottom face edges (white)
    drawLine(p000, p100, 0xCC,0xCC,0xCC);
    drawLine(p100, p110, 0xCC,0xCC,0xCC);
    drawLine(p110, p010, 0xCC,0xCC,0xCC);
    drawLine(p010, p000, 0xCC,0xCC,0xCC);
    // Top face edges (bright white)
    drawLine(p001, p101, 0xFF,0xFF,0xFF);
    drawLine(p101, p111, 0xFF,0xFF,0xFF);
    drawLine(p111, p011, 0xFF,0xFF,0xFF);
    drawLine(p011, p001, 0xFF,0xFF,0xFF);
    // Vertical edges (cyan)
    drawLine(p000, p001, 0xFF,0xB0,0x30);
    drawLine(p100, p101, 0xFF,0xB0,0x30);
    drawLine(p110, p111, 0xFF,0xB0,0x30);
    drawLine(p010, p011, 0xFF,0xB0,0x30);

    return bgra;
}

} // namespace Engine
} // namespace ExplorerLens
