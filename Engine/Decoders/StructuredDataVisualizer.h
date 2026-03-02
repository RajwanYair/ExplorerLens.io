// StructuredDataVisualizer.h — XML/JSON/YAML Structure Thumbnail
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes structured data files to generate tree-structure thumbnails.
// Computes nesting depth, key count, array sizes, and generates a visual
// hierarchy representation for thumbnail rendering.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class StructuredFormat : uint8_t { JSON, XML, YAML, TOML, INI, Unknown };

struct DataNode {
    std::wstring key;
    uint32_t depth = 0;
    uint32_t childCount = 0;
    bool isArray = false;
    bool isLeaf = true;
};

struct StructuredDataInfo {
    StructuredFormat format = StructuredFormat::Unknown;
    uint32_t totalNodes = 0;
    uint32_t maxDepth = 0;
    uint32_t arrayCount = 0;
    uint32_t objectCount = 0;
    uint32_t leafCount = 0;
    uint32_t totalLines = 0;
    uint64_t fileSize = 0;
    std::vector<DataNode> topLevelNodes;
};

struct StructuredDataStats {
    uint32_t filesProcessed = 0;
    uint32_t maxDepthSeen = 0;
    uint64_t totalNodesProcessed = 0;
};

class StructuredDataVisualizer {
public:
    StructuredDataVisualizer() = default;
    ~StructuredDataVisualizer() = default;

    static const wchar_t* GetName() { return L"StructuredDataVisualizer"; }

    bool CanVisualize(const wchar_t* ext) const {
        if (!ext) return false;
        return DetectFormat(ext) != StructuredFormat::Unknown;
    }

    StructuredFormat DetectFormat(const wchar_t* ext) const {
        if (!ext) return StructuredFormat::Unknown;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        if (e == L".json" || e == L".geojson") return StructuredFormat::JSON;
        if (e == L".xml" || e == L".xsd" || e == L".xsl" || e == L".svg" || e == L".plist") return StructuredFormat::XML;
        if (e == L".yaml" || e == L".yml") return StructuredFormat::YAML;
        if (e == L".toml") return StructuredFormat::TOML;
        if (e == L".ini" || e == L".cfg" || e == L".conf") return StructuredFormat::INI;
        return StructuredFormat::Unknown;
    }

    /// Analyze JSON-like content for structure metrics (simplified heuristic parser).
    StructuredDataInfo AnalyzeJSON(const std::string& content) const {
        StructuredDataInfo info;
        info.format = StructuredFormat::JSON;
        info.fileSize = content.size();
        uint32_t depth = 0;
        bool inString = false;

        for (size_t i = 0; i < content.size(); ++i) {
            char c = content[i];
            if (c == '\n') info.totalLines++;
            if (inString) {
                if (c == '"' && (i == 0 || content[i - 1] != '\\')) inString = false;
                continue;
            }
            if (c == '"') { inString = true; continue; }
            if (c == '{') { info.objectCount++; depth++; info.maxDepth = std::max(info.maxDepth, depth); }
            else if (c == '}') { if (depth > 0) depth--; }
            else if (c == '[') { info.arrayCount++; depth++; info.maxDepth = std::max(info.maxDepth, depth); }
            else if (c == ']') { if (depth > 0) depth--; }
            else if (c == ':') { info.totalNodes++; }
        }
        info.leafCount = info.totalNodes > info.objectCount ? info.totalNodes - info.objectCount : 0;
        return info;
    }

    /// Analyze XML content for structure metrics (tag counting).
    StructuredDataInfo AnalyzeXML(const std::string& content) const {
        StructuredDataInfo info;
        info.format = StructuredFormat::XML;
        info.fileSize = content.size();
        uint32_t depth = 0;

        for (size_t i = 0; i < content.size(); ++i) {
            if (content[i] == '\n') info.totalLines++;
            if (content[i] == '<' && i + 1 < content.size()) {
                if (content[i + 1] == '/') {
                    if (depth > 0) depth--;
                }
                else if (content[i + 1] != '?' && content[i + 1] != '!') {
                    info.totalNodes++;
                    depth++;
                    info.maxDepth = std::max(info.maxDepth, depth);
                    // Check if self-closing
                    auto close = content.find('>', i);
                    if (close != std::string::npos && close > 0 && content[close - 1] == '/') {
                        depth--;
                        info.leafCount++;
                    }
                }
            }
        }
        return info;
    }

    StructuredDataStats GetStats() const { return m_stats; }

private:
    mutable StructuredDataStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
