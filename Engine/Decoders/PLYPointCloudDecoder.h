// PLYPointCloudDecoder.h — PLY Point Cloud Viewer
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Stanford PLY (Polygon File Format) files in ASCII and binary modes.
// Extracts vertex count, face count, color info, and bounding box for
// generating a point cloud thumbnail via orthographic projection.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PLYFormat : uint8_t { ASCII, BinaryLE, BinaryBE, Unknown };

struct PLYVertex {
    float x = 0, y = 0, z = 0;
    uint8_t r = 200, g = 200, b = 200, a = 255;
};

struct PLYHeader {
    PLYFormat format = PLYFormat::Unknown;
    uint32_t vertexCount = 0;
    uint32_t faceCount = 0;
    bool hasColor = false;
    bool hasNormals = false;
    bool hasAlpha = false;
    uint32_t headerSize = 0; // byte offset to data
};

struct PLYBoundingBox {
    float minX = 0, minY = 0, minZ = 0;
    float maxX = 0, maxY = 0, maxZ = 0;
};

struct PLYStats {
    uint32_t filesProcessed = 0;
    uint64_t totalVertices = 0;
    uint64_t totalFaces = 0;
};

class PLYPointCloudDecoder {
public:
    PLYPointCloudDecoder() = default;
    ~PLYPointCloudDecoder() = default;

    static const wchar_t* GetName() { return L"PLYPointCloudDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".ply";
    }

    /// Detect PLY magic: "ply\n" or "ply\r\n"
    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 4) return false;
        return data[0] == 'p' && data[1] == 'l' && data[2] == 'y' &&
            (data[3] == '\n' || data[3] == '\r');
    }

    /// Parse PLY header (ASCII text lines until "end_header").
    PLYHeader ParseHeader(const uint8_t* data, size_t size) const {
        PLYHeader hdr;
        if (!DetectMagic(data, size)) return hdr;

        std::string text(reinterpret_cast<const char*>(data),
            std::min(size, static_cast<size_t>(8192)));
        auto endHdr = text.find("end_header");
        if (endHdr == std::string::npos) return hdr;
        hdr.headerSize = static_cast<uint32_t>(endHdr) + 11; // +len("end_header\n")

        if (text.find("format ascii") != std::string::npos) hdr.format = PLYFormat::ASCII;
        else if (text.find("binary_little_endian") != std::string::npos) hdr.format = PLYFormat::BinaryLE;
        else if (text.find("binary_big_endian") != std::string::npos) hdr.format = PLYFormat::BinaryBE;

        auto vPos = text.find("element vertex ");
        if (vPos != std::string::npos) hdr.vertexCount = static_cast<uint32_t>(std::atoi(text.c_str() + vPos + 15));

        auto fPos = text.find("element face ");
        if (fPos != std::string::npos) hdr.faceCount = static_cast<uint32_t>(std::atoi(text.c_str() + fPos + 13));

        hdr.hasColor = text.find("property uchar red") != std::string::npos ||
            text.find("property uint8 red") != std::string::npos;
        hdr.hasNormals = text.find("property float nx") != std::string::npos;

        return hdr;
    }

    /// Compute bounding box from vertex array.
    PLYBoundingBox ComputeBounds(const std::vector<PLYVertex>& vertices) const {
        PLYBoundingBox bb;
        if (vertices.empty()) return bb;
        bb.minX = bb.maxX = vertices[0].x;
        bb.minY = bb.maxY = vertices[0].y;
        bb.minZ = bb.maxZ = vertices[0].z;
        for (const auto& v : vertices) {
            bb.minX = std::min(bb.minX, v.x); bb.maxX = std::max(bb.maxX, v.x);
            bb.minY = std::min(bb.minY, v.y); bb.maxY = std::max(bb.maxY, v.y);
            bb.minZ = std::min(bb.minZ, v.z); bb.maxZ = std::max(bb.maxZ, v.z);
        }
        return bb;
    }

    PLYStats GetStats() const { return m_stats; }

private:
    mutable PLYStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
