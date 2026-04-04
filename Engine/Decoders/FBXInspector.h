// FBXInspector.h — Autodesk FBX Model Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Inspects FBX files (binary format) to extract header version, node count,
// and scene hierarchy for 3D model thumbnail generation. Supports FBX 7100+
// binary format. ASCII FBX detected but redirected to text parser.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FBXNodeInfo
{
    std::string name;
    uint32_t propertyCount = 0;
    uint32_t childCount = 0;
    uint64_t dataOffset = 0;
};

struct FBXFileInfo
{
    bool isValid = false;
    bool isBinary = false;
    bool isASCII = false;
    uint32_t version = 0;  // e.g., 7100, 7400, 7500
    uint32_t nodeCount = 0;
    uint32_t objectCount = 0;
    uint32_t geometryCount = 0;
    uint32_t materialCount = 0;
    uint32_t textureCount = 0;
    uint64_t fileSize = 0;
    std::vector<FBXNodeInfo> topNodes;
};

struct FBXStats
{
    uint32_t filesInspected = 0;
    uint64_t totalNodes = 0;
};

class FBXInspector
{
  public:
    FBXInspector() = default;
    ~FBXInspector() = default;

    static const wchar_t* GetName()
    {
        return L"FBXInspector";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".fbx";
    }

    /// Detect FBX binary magic: "Kaydara FBX Binary  \x00" (23 bytes)
    bool DetectBinaryMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 27)
            return false;
        return memcmp(data, "Kaydara FBX Binary  ", 20) == 0;
    }

    /// Detect FBX ASCII format.
    bool DetectASCIIMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 20)
            return false;
        std::string text(reinterpret_cast<const char*>(data), std::min(size, static_cast<size_t>(256)));
        return text.find("FBXHeaderExtension") != std::string::npos;
    }

    /// Parse FBX binary header to get version and top-level structure.
    FBXFileInfo Inspect(const uint8_t* data, size_t size) const
    {
        FBXFileInfo info;
        info.fileSize = size;

        if (DetectBinaryMagic(data, size)) {
            info.isBinary = true;
            info.isValid = true;
            // Version at offset 23 (little-endian uint32)
            if (size >= 27)
                memcpy(&info.version, data + 23, 4);

            // Scan top-level nodes (simplified)
            size_t offset = 27;
            while (offset + 13 <= size) {
                uint32_t endOffset = 0;
                memcpy(&endOffset, data + offset, 4);
                if (endOffset == 0 || endOffset > size)
                    break;

                uint32_t numProps = 0;
                memcpy(&numProps, data + offset + 4, 4);

                uint8_t nameLen = data[offset + 12];
                if (offset + 13 + nameLen > size)
                    break;

                FBXNodeInfo node;
                node.name = std::string(reinterpret_cast<const char*>(data + offset + 13), nameLen);
                node.propertyCount = numProps;
                node.dataOffset = offset;
                info.topNodes.push_back(node);
                info.nodeCount++;

                if (node.name == "Objects")
                    info.objectCount++;
                if (node.name == "Geometry")
                    info.geometryCount++;
                if (node.name == "Material")
                    info.materialCount++;

                offset = endOffset;
            }
        } else if (DetectASCIIMagic(data, size)) {
            info.isASCII = true;
            info.isValid = true;
        }

        return info;
    }

    FBXStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable FBXStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
