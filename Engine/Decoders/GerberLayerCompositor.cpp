// GerberLayerCompositor.cpp — RS-274X Gerber Layer Compositor
// Copyright (c) 2026 ExplorerLens Project
//
#include "GerberLayerCompositor.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

bool GerberLayerCompositor::IsGerber(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 4) return false;
    const char* p = reinterpret_cast<const char*>(data);
    // RS-274X starts with "G04 " (comment) or "%FS" (format spec) or "G36" etc.
    if (p[0] == 'G' && p[1] == '0' && p[2] == '4' && p[3] == ' ') return true;
    if (p[0] == '%' && p[1] == 'F' && p[2] == 'S') return true;
    if (p[0] == 'G' && (p[1] == '3' || p[1] == '7') && (p[2] >= '0' && p[2] <= '9')) return true;
    return false;
}

GerberLayerType GerberLayerCompositor::DetectLayerType(const char* ext) noexcept
{
    if (!ext || *ext == '\0') return GerberLayerType::Unknown;
    // Compare lowercased
    char buf[8]{};
    for (int i = 0; i < 7 && ext[i]; ++i)
        buf[i] = static_cast<char>(tolower(static_cast<unsigned char>(ext[i])));

    if (strcmp(buf, ".gtl")  == 0 || strcmp(buf, ".cmp") == 0) return GerberLayerType::CopperTop;
    if (strcmp(buf, ".gbl")  == 0 || strcmp(buf, ".sol") == 0) return GerberLayerType::CopperBottom;
    if (strcmp(buf, ".gts")  == 0) return GerberLayerType::SoldermaskTop;
    if (strcmp(buf, ".gbs")  == 0) return GerberLayerType::SoldermaskBottom;
    if (strcmp(buf, ".gto")  == 0 || strcmp(buf, ".sst") == 0) return GerberLayerType::SilkscreenTop;
    if (strcmp(buf, ".gbo")  == 0 || strcmp(buf, ".ssb") == 0) return GerberLayerType::SilkscreenBottom;
    if (strcmp(buf, ".gm1")  == 0 || strcmp(buf, ".gko") == 0) return GerberLayerType::BoardOutline;
    if (strcmp(buf, ".xln")  == 0 || strcmp(buf, ".drl") == 0) return GerberLayerType::DrillThrough;
    return GerberLayerType::Unknown;
}

std::vector<GerberAperture> GerberLayerCompositor::ParseApertures(
    const uint8_t* data, size_t size) noexcept
{
    std::vector<GerberAperture> apts;
    if (!data || size < 5) return apts;

    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    for (const char* q = p; q + 5 < end; ++q)
    {
        // Aperture definition: %ADD<code><type>,<params>*%
        if (q[0] != '%' || q[1] != 'A' || q[2] != 'D' || q[3] != 'D') continue;
        const char* r = q + 4;
        char* ep = nullptr;
        long code = strtol(r, &ep, 10);
        if (ep == r || code < 10 || ep >= end) continue;

        GerberAperture apt{};
        apt.code   = static_cast<uint32_t>(code);
        apt.type   = *ep; // 'C', 'R', 'O'

        const char* params = ep + 1;
        if (params < end && *params == ',') ++params;
        apt.param0 = strtod(params, &ep);
        if (ep != params)
        {
            const char* p2 = ep;
            while (p2 < end && *p2 == 'X') ++p2;
            apt.param1 = strtod(p2, &ep);
        }
        apts.push_back(apt);
        q += 4;
    }
    return apts;
}

GerberLayerInfo GerberLayerCompositor::ProbeLayer(
    const uint8_t* data, size_t size, const char* fileExtension) noexcept
{
    GerberLayerInfo info{};
    info.valid = false;
    if (!IsGerber(data, size)) return info;

    info.type  = DetectLayerType(fileExtension);
    if (fileExtension) info.fileExtension = fileExtension;

    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    for (const char* q = p; q + 2 < end; ++q)
    {
        if (*q == 'D' && (q[1] == '0' || q[1] == '1' || q[1] == '3'))
        {
            if (q[1] == '3') ++info.flashCount;
            else              ++info.drawCount;
        }
    }
    info.valid = true;
    return info;
}

std::vector<uint8_t> GerberLayerCompositor::RasteriseLayer(
    const uint8_t* data, size_t size, GerberLayerType layerType,
    uint32_t width, uint32_t height)
{
    const uint32_t pixCount = width * height;
    std::vector<uint8_t> bgra(static_cast<size_t>(pixCount) * 4, 0);

    // Background: dark PCB green
    for (uint32_t i = 0; i < pixCount; ++i)
    {
        bgra[i*4+0] = 0x18; bgra[i*4+1] = 0x38;
        bgra[i*4+2] = 0x18; bgra[i*4+3] = 0xFF;
    }

    if (!data || size < 4) return bgra;

    // Layer colour
    uint8_t B = 0x30, G = 0xA0, R = 0x30; // default copper gold
    switch (layerType)
    {
        case GerberLayerType::CopperTop:
        case GerberLayerType::CopperBottom:
        case GerberLayerType::CopperInner:
            B = 0x20; G = 0xC0; R = 0xC0; break; // gold
        case GerberLayerType::SoldermaskTop:
        case GerberLayerType::SoldermaskBottom:
            B = 0x30; G = 0x90; R = 0x30; break; // green
        case GerberLayerType::SilkscreenTop:
        case GerberLayerType::SilkscreenBottom:
            B = 0xF0; G = 0xF0; R = 0xF0; break; // white
        case GerberLayerType::DrillThrough:
            B = 0xA0; G = 0xA0; R = 0xA0; break; // grey
        case GerberLayerType::BoardOutline:
            B = 0x60; G = 0x60; R = 0xFF; break; // blue
        default: break;
    }

    // Scan flash operations and rasterise as filled circles
    const char* p   = reinterpret_cast<const char*>(data);
    const char* end = p + size;
    double curX = 0, curY = 0;
    double gridMin = -100.0, gridMax = 100.0; // assume ±100mm grid

    auto toPixel = [&](double v, uint32_t dim) -> uint32_t
    {
        double norm = (v - gridMin) / (gridMax - gridMin);
        uint32_t px = static_cast<uint32_t>(norm * (dim - 1));
        return px < dim ? px : dim - 1;
    };

    uint32_t plotCount = 0;
    for (const char* q = p; q < end && plotCount < 4096; ++q)
    {
        if (*q == 'X')
        {
            char* ep = nullptr;
            long xi = strtol(q + 1, &ep, 10);
            if (ep != q + 1) { curX = xi * 0.0001; q = ep - 1; }
        }
        else if (*q == 'Y')
        {
            char* ep = nullptr;
            long yi = strtol(q + 1, &ep, 10);
            if (ep != q + 1) { curY = yi * 0.0001; q = ep - 1; }
        }
        else if (q[0] == 'D' && q[1] == '0' && q[2] == '3')
        {
            // Flash at (curX, curY)
            uint32_t px = toPixel(curX, width);
            uint32_t py = toPixel(curY, height);
            // Draw a 3×3 dot
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                {
                    int nx = (int)px + dx, ny = (int)py + dy;
                    if (nx >= 0 && nx < (int)width && ny >= 0 && ny < (int)height)
                    {
                        uint32_t idx = (static_cast<uint32_t>(ny) * width + static_cast<uint32_t>(nx)) * 4;
                        bgra[idx+0]=B; bgra[idx+1]=G; bgra[idx+2]=R; bgra[idx+3]=0xFF;
                    }
                }
            ++plotCount;
        }
    }
    return bgra;
}

} // namespace Engine
} // namespace ExplorerLens
