// FormatFamilyResolver.h — Hierarchical Format Family Resolver
// Copyright (c) 2026 ExplorerLens Project
//
// Classifies file formats into hierarchical families with MIME type and magic byte
// lookup for fast format routing in the decode pipeline.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class FormatFamily : uint8_t {
    Image = 0, Video, Audio, Document, Archive,
    Scientific, CAD, Font, Shader, Spreadsheet, Unknown = 0xFF
};

struct FormatNode {
    std::string name;
    FormatFamily family = FormatFamily::Unknown;
    std::string parent;
    std::string mimeType;
    std::vector<std::string> extensions;
    std::array<uint8_t, 8> magicBytes = {};
    uint8_t magicLength = 0;
    bool isContainer = false;

    bool MatchesMagic(const uint8_t* data, size_t len) const {
        if (magicLength == 0 || len < magicLength) return false;
        for (uint8_t i = 0; i < magicLength; ++i)
            if (data[i] != magicBytes[i]) return false;
        return true;
    }
};

class FormatFamilyResolver {
public:
    FormatFamilyResolver() { BuildTree(); }
    ~FormatFamilyResolver() = default;

    const FormatNode* Resolve(const uint8_t* headerBytes, size_t len) const {
        for (const auto& node : m_nodes)
            if (node.MatchesMagic(headerBytes, len)) return &node;
        return nullptr;
    }

    FormatFamily GetFamily(const std::string& formatName) const {
        for (const auto& node : m_nodes)
            if (node.name == formatName) return node.family;
        return FormatFamily::Unknown;
    }

    std::vector<FormatNode> GetChildren(const std::string& parentName) const {
        std::vector<FormatNode> children;
        for (const auto& node : m_nodes)
            if (node.parent == parentName) children.push_back(node);
        return children;
    }

    const FormatNode* FindByExtension(const std::string& ext) const {
        for (const auto& node : m_nodes)
            for (const auto& e : node.extensions)
                if (e == ext) return &node;
        return nullptr;
    }

    const FormatNode* FindByMimeType(const std::string& mime) const {
        for (const auto& node : m_nodes)
            if (node.mimeType == mime) return &node;
        return nullptr;
    }

    std::vector<const FormatNode*> GetByFamily(FormatFamily family) const {
        std::vector<const FormatNode*> result;
        for (const auto& node : m_nodes)
            if (node.family == family) result.push_back(&node);
        return result;
    }

    size_t GetNodeCount() const { return m_nodes.size(); }

    void BuildTree() {
        m_nodes.clear();
        auto add = [this](const std::string& name, FormatFamily fam, const std::string& mime,
                          std::vector<std::string> exts, std::array<uint8_t, 8> magic, uint8_t mlen) {
            FormatNode n;
            n.name = name; n.family = fam; n.mimeType = mime;
            n.extensions = std::move(exts); n.magicBytes = magic; n.magicLength = mlen;
            m_nodes.push_back(std::move(n));
        };
        add("JPEG", FormatFamily::Image, "image/jpeg",   {".jpg",".jpeg"}, {0xFF,0xD8,0xFF,0,0,0,0,0}, 3);
        add("PNG",  FormatFamily::Image, "image/png",    {".png"},         {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A}, 8);
        add("WebP", FormatFamily::Image, "image/webp",   {".webp"},        {0x52,0x49,0x46,0x46,0,0,0,0}, 4);
        add("GIF",  FormatFamily::Image, "image/gif",    {".gif"},         {0x47,0x49,0x46,0x38,0,0,0,0}, 4);
        add("BMP",  FormatFamily::Image, "image/bmp",    {".bmp"},         {0x42,0x4D,0,0,0,0,0,0}, 2);
        add("TIFF", FormatFamily::Image, "image/tiff",   {".tif",".tiff"}, {0x49,0x49,0x2A,0x00,0,0,0,0}, 4);
        add("PSD",  FormatFamily::Image, "image/vnd.adobe.photoshop", {".psd"}, {0x38,0x42,0x50,0x53,0,0,0,0}, 4);
        add("PDF",  FormatFamily::Document,"application/pdf", {".pdf"},    {0x25,0x50,0x44,0x46,0x2D,0,0,0}, 5);
        add("FITS", FormatFamily::Scientific,"application/fits",{".fits",".fit"},{0x53,0x49,0x4D,0x50,0x4C,0x45,0,0}, 6);
        add("ZIP",  FormatFamily::Archive,"application/zip",{".zip"},      {0x50,0x4B,0x03,0x04,0,0,0,0}, 4);
    }

private:
    std::vector<FormatNode> m_nodes;
};

}} // namespace ExplorerLens::Engine
