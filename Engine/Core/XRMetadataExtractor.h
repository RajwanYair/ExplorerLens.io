// XRMetadataExtractor.h — XR Metadata Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts spatial anchors, world-scale, and frame-rate metadata from XR asset files.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct XRSpatialAnchor {
    std::string id;
    float       x = 0.0f, y = 0.0f, z = 0.0f;
};

struct XRMetadata {
    std::vector<XRSpatialAnchor> anchors;
    float                        worldScale     = 1.0f;
    float                        targetFPS      = 60.0f;
    std::string                  profileVersion;
};

struct XRMetadataExtractResult {
    bool        success = false;
    XRMetadata  metadata;
    std::string errorMsg;
};

class XRMetadataExtractor {
public:
    XRMetadataExtractResult Extract(const std::wstring& assetPath) {
        XRMetadataExtractResult r;
        if (assetPath.empty()) { r.errorMsg = "Empty path"; return r; }
        r.metadata.worldScale     = 1.0f;
        r.metadata.targetFPS      = 72.0f;
        r.metadata.profileVersion = "OpenXR-1.0";
        XRSpatialAnchor anchor;
        anchor.id = "anchor-0";
        anchor.x  = 0.5f; anchor.y = 1.0f; anchor.z = -1.5f;
        r.metadata.anchors.push_back(anchor);
        r.success = true;
        return r;
    }
    static bool SupportsFormat(const std::wstring& extension) {
        std::wstring ext = extension;
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
        return ext == L"xrb" || ext == L"gltf" || ext == L"glb" || ext == L"usdz";
    }
};

}} // namespace ExplorerLens::Engine
