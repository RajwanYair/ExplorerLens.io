// COLLADADecoder.h — COLLADA (.dae) Scene Preview Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Parses COLLADA XML (.dae) files to extract scene graph metadata,
// geometry counts, material references, and camera/light info for
// generating a 3D scene thumbnail preview.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct COLLADASceneInfo {
    uint32_t geometryCount = 0;
    uint32_t materialCount = 0;
    uint32_t lightCount = 0;
    uint32_t cameraCount = 0;
    uint32_t animationCount = 0;
    uint32_t imageCount = 0;
    uint32_t totalVertices = 0;
    uint32_t totalTriangles = 0;
    std::wstring upAxis; // Y_UP, Z_UP, X_UP
    std::wstring version;
    std::wstring author;
};

struct COLLADAStats {
    uint32_t filesProcessed = 0;
    uint64_t totalGeometries = 0;
    uint64_t totalMaterials = 0;
};

class COLLADADecoder {
public:
    COLLADADecoder() = default;
    ~COLLADADecoder() = default;

    static const wchar_t* GetName() { return L"COLLADADecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".dae" || e == L".collada";
    }

    /// Detect COLLADA XML: look for <COLLADA> root element.
    bool DetectFormat(const uint8_t* data, size_t size) const {
        if (!data || size < 50) return false;
        std::string text(reinterpret_cast<const char*>(data),
            std::min(size, static_cast<size_t>(1024)));
        return text.find("<COLLADA") != std::string::npos;
    }

    /// Parse COLLADA XML to extract scene summary.
    COLLADASceneInfo ParseScene(const std::string& xml) const {
        COLLADASceneInfo info;

        // Count XML elements by tag name
        info.geometryCount = CountTag(xml, "<geometry ");
        if (info.geometryCount == 0) info.geometryCount = CountTag(xml, "<geometry>");
        info.materialCount = CountTag(xml, "<material ");
        info.lightCount = CountTag(xml, "<light ");
        info.cameraCount = CountTag(xml, "<camera ");
        info.animationCount = CountTag(xml, "<animation ");
        info.imageCount = CountTag(xml, "<image ");

        // Extract up_axis
        auto upPos = xml.find("<up_axis>");
        if (upPos != std::string::npos) {
            auto upEnd = xml.find("</up_axis>", upPos);
            if (upEnd != std::string::npos) {
                std::string axis = xml.substr(upPos + 9, upEnd - upPos - 9);
                info.upAxis = std::wstring(axis.begin(), axis.end());
            }
        }

        // Extract version from COLLADA root
        auto verPos = xml.find("version=\"");
        if (verPos != std::string::npos) {
            auto verEnd = xml.find("\"", verPos + 9);
            if (verEnd != std::string::npos) {
                std::string ver = xml.substr(verPos + 9, verEnd - verPos - 9);
                info.version = std::wstring(ver.begin(), ver.end());
            }
        }

        return info;
    }

    COLLADAStats GetStats() const { return m_stats; }

private:
    uint32_t CountTag(const std::string& xml, const std::string& tag) const {
        uint32_t count = 0;
        size_t pos = 0;
        while ((pos = xml.find(tag, pos)) != std::string::npos) {
            count++;
            pos += tag.size();
        }
        return count;
    }

    mutable COLLADAStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
