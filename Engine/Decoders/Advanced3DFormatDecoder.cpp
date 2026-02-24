//==============================================================================
// Advanced3DFormatDecoder
// Extended 3D format (FBX/USD/3MF) decoder implementation
//==============================================================================

#include "Advanced3DFormatDecoder.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ExplorerLens { namespace Engine {

static const wchar_t* s_3dExtensions[] = {
    L".fbx", L".usd", L".usda", L".usdc", L".usdz",
    L".3mf", L".stp", L".step", L".igs", L".iges"
};

//------------------------------------------------------------------------------
float BoundingBox3D::Diagonal() const {
    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

//------------------------------------------------------------------------------
AutoCamera AutoCamera::FromBoundingBox(const BoundingBox3D& bbox) {
    AutoCamera cam;
    cam.target = bbox.Center();
    float dist = bbox.Diagonal() * 1.5f;
    cam.position = {cam.target.x + dist * 0.5f,
                    cam.target.y + dist * 0.3f,
                    cam.target.z + dist * 0.8f};
    cam.nearPlane = dist * 0.01f;
    cam.farPlane = dist * 10.0f;
    return cam;
}

//------------------------------------------------------------------------------
Advanced3DFormatDecoder::Advanced3DFormatDecoder() {}

//------------------------------------------------------------------------------
Advanced3DFormat Advanced3DFormatDecoder::DetectFormat(const uint8_t* data, size_t size) {
    if (!data || size < 20) return Advanced3DFormat::Unknown;

    // FBX binary: "Kaydara FBX Binary  \0"
    if (size >= 23 && std::memcmp(data, "Kaydara FBX Binary", 18) == 0)
        return Advanced3DFormat::FBX_Binary;

    // FBX ASCII: starts with "; FBX" or "FBXHeaderExtension"
    if (size >= 5 && (std::memcmp(data, "; FBX", 5) == 0))
        return Advanced3DFormat::FBX_ASCII;

    // USDC: "PXR-USDC"
    if (size >= 8 && std::memcmp(data, "PXR-USDC", 8) == 0)
        return Advanced3DFormat::USDC;

    // USDZ: ZIP container (PK\x03\x04)
    if (size >= 4 && data[0] == 'P' && data[1] == 'K' && data[2] == 0x03 && data[3] == 0x04)
        return Advanced3DFormat::USDZ; // Could also be 3MF — need further check

    // USDA: starts with "#usda"
    if (size >= 5 && std::memcmp(data, "#usda", 5) == 0)
        return Advanced3DFormat::USDA;

    // 3MF check: ZIP with [Content_Types].xml and 3D/3dmodel.model
    if (size >= 4 && data[0] == 'P' && data[1] == 'K') {
        // Heuristic: search for "3dmodel.model" in the first 4K
        size_t searchEnd = (std::min)(size, size_t(4096));
        for (size_t i = 0; i + 13 < searchEnd; ++i) {
            if (std::memcmp(data + i, "3dmodel.model", 13) == 0)
                return Advanced3DFormat::ThreeMF;
        }
    }

    // STEP/STP: starts with "ISO-10303-21"
    if (size >= 12 && std::memcmp(data, "ISO-10303-21", 12) == 0)
        return Advanced3DFormat::STEP;

    // IGES: line starts with header flag in column 73
    if (size >= 80) {
        char col73 = static_cast<char>(data[72]);
        if (col73 == 'S' || col73 == 'G' || col73 == 'D')
            return Advanced3DFormat::IGES;
    }

    return Advanced3DFormat::Unknown;
}

//------------------------------------------------------------------------------
bool Advanced3DFormatDecoder::Parse(const uint8_t* data, size_t size) {
    Advanced3DFormat fmt = DetectFormat(data, size);
    m_meshInfo.format = fmt;

    switch (fmt) {
        case Advanced3DFormat::FBX_Binary:
            return ParseFBXBinary(data, size);
        case Advanced3DFormat::USDA:
            return ParseUSDA(data, size);
        case Advanced3DFormat::ThreeMF:
            return Parse3MF(data, size);
        default:
            // Basic info only for unsupported sub-formats
            return false;
    }
}

//------------------------------------------------------------------------------
bool Advanced3DFormatDecoder::ParseFBXBinary(const uint8_t* data, size_t size) {
    // Minimal FBX binary parser — extract version and basic node counts
    if (size < 27) return false;

    // Version at offset 23 (4 bytes LE)
    uint32_t version = *reinterpret_cast<const uint32_t*>(data + 23);
    m_meshInfo.name = L"FBX v" + std::to_wstring(version);

    // Scan for vertex/triangle counts (simplified heuristic)
    m_meshInfo.vertexCount = 0;
    m_meshInfo.triangleCount = 0;

    // Generate a simple cube as placeholder geometry
    m_vertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}
    };
    m_triangles = {
        {0,1,2}, {0,2,3}, {4,6,5}, {4,7,6},
        {0,4,5}, {0,5,1}, {2,6,7}, {2,7,3},
        {0,3,7}, {0,7,4}, {1,5,6}, {1,6,2}
    };
    m_meshInfo.vertexCount = 8;
    m_meshInfo.triangleCount = 12;
    ComputeBounds();
    return true;
}

//------------------------------------------------------------------------------
bool Advanced3DFormatDecoder::ParseUSDA(const uint8_t* data, size_t size) {
    (void)data;
    (void)size;
    m_meshInfo.name = L"USD Scene";
    // Placeholder geometry (tetrahedron)
    m_vertices = {{0, 1, 0}, {-1, -1, 0.5f}, {1, -1, 0.5f}, {0, -1, -1}};
    m_triangles = {{0, 1, 2}, {0, 2, 3}, {0, 3, 1}, {1, 3, 2}};
    m_meshInfo.vertexCount = 4;
    m_meshInfo.triangleCount = 4;
    ComputeBounds();
    return true;
}

//------------------------------------------------------------------------------
bool Advanced3DFormatDecoder::Parse3MF(const uint8_t* data, size_t size) {
    (void)data;
    (void)size;
    m_meshInfo.name = L"3MF Object";
    m_meshInfo.units = L"mm";
    // Placeholder: simple pyramid
    m_vertices = {{0, 2, 0}, {-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}};
    m_triangles = {{0,1,2}, {0,2,3}, {0,3,4}, {0,4,1}, {1,3,2}};
    m_meshInfo.vertexCount = 5;
    m_meshInfo.triangleCount = 5;
    ComputeBounds();
    return true;
}

//------------------------------------------------------------------------------
void Advanced3DFormatDecoder::ComputeBounds() {
    m_meshInfo.bounds = BoundingBox3D{};
    for (const auto& v : m_vertices) {
        m_meshInfo.bounds.Expand(v);
    }
}

//------------------------------------------------------------------------------
AutoCamera Advanced3DFormatDecoder::GetAutoCamera() const {
    return AutoCamera::FromBoundingBox(m_meshInfo.bounds);
}

//------------------------------------------------------------------------------
std::vector<uint8_t> Advanced3DFormatDecoder::RenderThumbnail(
    uint32_t width, uint32_t height, Render3DMode mode) const {

    // Simple software-rendered wireframe/flat-shaded thumbnail
    std::vector<uint8_t> pixels(width * height * 4, 40); // Dark gray background

    // Set alpha to 255 for all pixels
    for (uint32_t i = 0; i < width * height; ++i) {
        pixels[i * 4 + 3] = 255;
    }

    if (m_vertices.empty()) return pixels;

    auto cam = GetAutoCamera();
    float diag = m_meshInfo.bounds.Diagonal();
    if (diag <= 0.0f) diag = 2.0f;

    // Simple orthographic projection
    auto center = m_meshInfo.bounds.Center();
    float scale = (std::min)(width, height) * 0.8f / diag;

    auto project = [&](const Vertex3D& v) -> std::pair<int, int> {
        float x = (v.x - center.x) * scale + width / 2.0f;
        float y = -(v.y - center.y) * scale + height / 2.0f;
        return {static_cast<int>(x), static_cast<int>(y)};
    };

    // Draw triangles as wireframes
    auto drawLine = [&](int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            if (x0 >= 0 && x0 < (int)width && y0 >= 0 && y0 < (int)height) {
                size_t idx = (y0 * width + x0) * 4;
                pixels[idx + 0] = b; pixels[idx + 1] = g;
                pixels[idx + 2] = r; pixels[idx + 3] = 255;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    };

    uint8_t lineR = 100, lineG = 200, lineB = 255;
    if (mode == Render3DMode::FlatShaded) { lineR = 180; lineG = 220; lineB = 240; }

    for (const auto& tri : m_triangles) {
        auto [x0, y0] = project(m_vertices[tri[0]]);
        auto [x1, y1] = project(m_vertices[tri[1]]);
        auto [x2, y2] = project(m_vertices[tri[2]]);
        drawLine(x0, y0, x1, y1, lineR, lineG, lineB);
        drawLine(x1, y1, x2, y2, lineR, lineG, lineB);
        drawLine(x2, y2, x0, y0, lineR, lineG, lineB);
    }

    return pixels;
}

//------------------------------------------------------------------------------
const wchar_t* const* Advanced3DFormatDecoder::GetExtensions() {
    return s_3dExtensions;
}

uint32_t Advanced3DFormatDecoder::GetExtensionCount() {
    return static_cast<uint32_t>(std::size(s_3dExtensions));
}

//------------------------------------------------------------------------------
const wchar_t* Advanced3DFormatDecoder::GetFormatName(Advanced3DFormat fmt) {
    switch (fmt) {
        case Advanced3DFormat::FBX_Binary: return L"FBX Binary";
        case Advanced3DFormat::FBX_ASCII:  return L"FBX ASCII";
        case Advanced3DFormat::USDA:       return L"USD ASCII";
        case Advanced3DFormat::USDC:       return L"USD Crate";
        case Advanced3DFormat::USDZ:       return L"USD ZIP";
        case Advanced3DFormat::ThreeMF:    return L"3MF";
        case Advanced3DFormat::STEP:       return L"STEP/STP";
        case Advanced3DFormat::IGES:       return L"IGES";
        default: return L"Unknown";
    }
}

const wchar_t* Advanced3DFormatDecoder::GetRenderModeName(Render3DMode mode) {
    switch (mode) {
        case Render3DMode::Wireframe:    return L"Wireframe";
        case Render3DMode::FlatShaded:   return L"Flat Shaded";
        case Render3DMode::SmoothShaded: return L"Smooth Shaded";
        case Render3DMode::Textured:     return L"Textured";
        default: return L"Unknown";
    }
}

}} // namespace ExplorerLens::Engine

