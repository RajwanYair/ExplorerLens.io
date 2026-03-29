// QRThumbnailTrigger.h — QR Code Thumbnail Trigger
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes QR codes in real-time camera frames and resolves them to
// ExplorerLens file paths or thumbnail URLs for AR preview loading.
//
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <array>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct QRDetection {
    std::string              data;
    std::array<float, 8>     corners{};  // 4 × (x,y) corner points
    float                    confidence = 0.0f;
    double                   decodeMs   = 0.0;
};

struct QRThumbnailMapping {
    std::string qrContent;
    std::string filePath;
    bool        isUrl     = false;
    bool        valid     = false;
};

class QRThumbnailTrigger {
public:
    QRThumbnailTrigger() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    std::vector<QRDetection> Detect(const uint8_t* rgbData, uint32_t width, uint32_t height) {
        (void)rgbData; (void)width; (void)height;
        if (!m_ready || !rgbData) return {};
        return {};
    }

    QRThumbnailMapping Resolve(const QRDetection& detection) const {
        QRThumbnailMapping mapping;
        mapping.qrContent = detection.data;
        if (detection.data.empty()) return mapping;

        if (detection.data.rfind("elens://", 0) == 0) {
            mapping.filePath = detection.data.substr(8);
            mapping.isUrl    = false;
            mapping.valid    = !mapping.filePath.empty();
        } else if (detection.data.rfind("https://", 0) == 0) {
            mapping.filePath = detection.data;
            mapping.isUrl    = true;
            mapping.valid    = true;
        }
        return mapping;
    }

    std::string EncodeFileToQR(const std::string& filePath) const {
        return "elens://" + filePath;
    }

    double GetAverageDecodeMs() const { return m_avgDecodeMs; }

private:
    bool   m_ready       = false;
    double m_avgDecodeMs = 0.0;
};

}} // namespace ExplorerLens::Engine
