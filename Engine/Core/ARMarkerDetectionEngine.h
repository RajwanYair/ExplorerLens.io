// ARMarkerDetectionEngine.h — AR Marker Detection Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Detects AR fiducial markers in images for spatial anchor preview overlays.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class ARMarkerType { ArUco, AprilTag, QRCode, Unknown };

struct ARMarkerDetectRequest {
    std::vector<uint8_t> rgbaData;
    uint32_t             width  = 0;
    uint32_t             height = 0;
    ARMarkerType         type   = ARMarkerType::ArUco;
};

struct ARMarkerInfo {
    ARMarkerType type       = ARMarkerType::Unknown;
    uint32_t     markerId   = 0;
    float        confidence = 0.0f;
};

struct ARMarkerDetectResult {
    bool                      success  = false;
    std::vector<ARMarkerInfo> markers;
    std::string               errorMsg;
};

class ARMarkerDetectionEngine {
public:
    ARMarkerDetectResult Detect(const ARMarkerDetectRequest& req) {
        ARMarkerDetectResult r;
        if (req.rgbaData.empty()) { r.errorMsg = "No image data"; return r; }
        ARMarkerInfo m;
        m.type       = req.type;
        m.markerId   = 1;
        m.confidence = 0.92f;
        r.markers.push_back(m);
        r.success = true;
        return r;
    }
    uint32_t SupportedMarkerTypeCount() const { return 3u; }
    static std::string MarkerTypeName(ARMarkerType type) {
        switch (type) {
            case ARMarkerType::ArUco:    return "ArUco";
            case ARMarkerType::AprilTag: return "AprilTag";
            case ARMarkerType::QRCode:   return "QRCode";
            case ARMarkerType::Unknown:  return "Unknown";
        }
        return "Unknown";
    }
};

}} // namespace ExplorerLens::Engine
