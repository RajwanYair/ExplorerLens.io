// STLMeshDecoder.h — STL Binary/ASCII Mesh Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Parses STL (Stereolithography) files in both ASCII and binary formats.
// Binary: 80-byte header + triangle count + triangle data.
// ASCII: "solid" keyword followed by "facet normal" blocks.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct STLTriangle {
    float normal[3] = {};
    float v1[3] = {}, v2[3] = {}, v3[3] = {};
    uint16_t attributeByteCount = 0;
};

struct STLMeshInfo {
    bool isASCII = false;
    bool isBinary = false;
    uint32_t triangleCount = 0;
    std::string solidName;
    float boundMin[3] = {};
    float boundMax[3] = {};
    uint64_t fileSize = 0;
};

struct STLStats {
    uint32_t filesProcessed = 0;
    uint64_t totalTriangles = 0;
};

class STLMeshDecoder {
public:
    STLMeshDecoder() = default;
    ~STLMeshDecoder() = default;

    static const wchar_t* GetName() { return L"STLMeshDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".stl";
    }

    /// Detect whether STL is ASCII or binary.
    bool IsASCII(const uint8_t* data, size_t size) const {
        if (!data || size < 6) return false;
        // ASCII starts with "solid " followed by name
        return memcmp(data, "solid ", 6) == 0;
    }

    /// Parse binary STL header.
    STLMeshInfo ParseBinary(const uint8_t* data, size_t size) const {
        STLMeshInfo info;
        info.fileSize = size;
        if (!data || size < 84) return info;

        info.isBinary = true;
        // 80-byte header (may contain name)
        char header[81] = {};
        memcpy(header, data, 80);
        info.solidName = header;

        // Triangle count at offset 80
        memcpy(&info.triangleCount, data + 80, 4);

        // Validate file size matches triangle count
        uint64_t expectedSize = 84 + static_cast<uint64_t>(info.triangleCount) * 50;
        if (expectedSize > size) {
            info.triangleCount = static_cast<uint32_t>((size - 84) / 50);
        }

        return info;
    }

    /// Parse ASCII STL to count triangles.
    STLMeshInfo ParseASCII(const uint8_t* data, size_t size) const {
        STLMeshInfo info;
        info.fileSize = size;
        if (!data || size < 10) return info;

        info.isASCII = true;
        std::string text(reinterpret_cast<const char*>(data),
            std::min(size, static_cast<size_t>(1024)));

        // Extract solid name
        auto nameEnd = text.find('\n');
        if (nameEnd != std::string::npos && nameEnd > 6)
            info.solidName = text.substr(6, nameEnd - 6);

        // Count "facet" keywords
        std::string fullText(reinterpret_cast<const char*>(data), size);
        size_t pos = 0;
        while ((pos = fullText.find("facet normal", pos)) != std::string::npos) {
            info.triangleCount++;
            pos += 12;
        }
        return info;
    }

    /// Auto-detect format and parse.
    STLMeshInfo Parse(const uint8_t* data, size_t size) const {
        if (IsASCII(data, size)) return ParseASCII(data, size);
        return ParseBinary(data, size);
    }

    STLStats GetStats() const { return m_stats; }

private:
    mutable STLStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
