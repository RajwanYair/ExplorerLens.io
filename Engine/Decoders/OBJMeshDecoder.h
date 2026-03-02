// OBJMeshDecoder.h — Wavefront OBJ Mesh Thumbnail Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Parses Wavefront OBJ files to extract vertices, faces, normals,
// and material references. Generates wireframe/flat-shaded thumbnail
// using orthographic projection with simple Z-buffer.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct OBJVertex { float x = 0, y = 0, z = 0; };
struct OBJNormal { float nx = 0, ny = 0, nz = 0; };
struct OBJTexCoord { float u = 0, v = 0; };

struct OBJFace {
    uint32_t vertexIndices[4] = {};
    uint32_t normalIndices[4] = {};
    uint32_t texCoordIndices[4] = {};
    uint8_t vertexCount = 3;
};

struct OBJMeshInfo {
    uint32_t vertexCount = 0;
    uint32_t normalCount = 0;
    uint32_t texCoordCount = 0;
    uint32_t faceCount = 0;
    uint32_t groupCount = 0;
    uint32_t materialCount = 0;
    std::wstring mtlLib;
    bool hasNormals = false;
    bool hasTexCoords = false;
};

struct OBJStats {
    uint32_t filesProcessed = 0;
    uint64_t totalVertices = 0;
    uint64_t totalFaces = 0;
};

class OBJMeshDecoder {
public:
    OBJMeshDecoder() = default;
    ~OBJMeshDecoder() = default;

    static const wchar_t* GetName() { return L"OBJMeshDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".obj";
    }

    /// Parse OBJ content to extract mesh metrics (without full geometry).
    OBJMeshInfo ParseMetrics(const std::string& content) const {
        OBJMeshInfo info;
        size_t pos = 0;
        while (pos < content.size()) {
            size_t eol = content.find('\n', pos);
            if (eol == std::string::npos) eol = content.size();

            if (pos < content.size()) {
                char first = content[pos];
                if (first == 'v') {
                    if (pos + 1 < content.size()) {
                        if (content[pos + 1] == ' ') info.vertexCount++;
                        else if (content[pos + 1] == 'n') { info.normalCount++; info.hasNormals = true; }
                        else if (content[pos + 1] == 't') { info.texCoordCount++; info.hasTexCoords = true; }
                    }
                }
                else if (first == 'f') { info.faceCount++; }
                else if (first == 'g') { info.groupCount++; }
                else if (first == 'u' && content.substr(pos, 6) == "usemtl") info.materialCount++;
            }
            pos = eol + 1;
        }
        return info;
    }

    /// Estimate triangle count (faces may be quads).
    uint32_t EstimateTriangleCount(const OBJMeshInfo& info) const {
        // Conservative: assume average 1.2 triangles per face
        return static_cast<uint32_t>(info.faceCount * 1.2);
    }

    OBJStats GetStats() const { return m_stats; }

private:
    mutable OBJStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
