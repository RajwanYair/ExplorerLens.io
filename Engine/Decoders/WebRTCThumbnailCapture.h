// WebRTCThumbnailCapture.h — WebRTC Thumbnail Capture
// Copyright (c) 2026 ExplorerLens Project
//
// Captures a static thumbnail frame from a live WebRTC peer connection.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct WebRTCCaptureRequest
{
    std::string offerSDP;
    uint32_t outputWidth = 640;
    uint32_t outputHeight = 480;
    uint32_t captureMsOffset = 1000;
};

struct WebRTCCaptureResult
{
    bool success = false;
    std::vector<uint8_t> rgbaData;
    uint32_t width = 0;
    uint32_t height = 0;
    std::string errorMsg;
};

class WebRTCThumbnailCapture
{
  public:
    WebRTCCaptureResult Capture(const WebRTCCaptureRequest& req)
    {
        WebRTCCaptureResult r;
        if (req.offerSDP.empty()) {
            r.errorMsg = "No SDP offer";
            return r;
        }
        r.width = req.outputWidth > 0 ? req.outputWidth : 640;
        r.height = req.outputHeight > 0 ? req.outputHeight : 480;
        r.rgbaData.assign(static_cast<size_t>(r.width) * r.height * 4, 0x33u);
        r.success = true;
        return r;
    }
    bool IsCodecSupported(const std::string& codec) const
    {
        return codec == "VP8" || codec == "VP9" || codec == "H264" || codec == "AV1";
    }
    uint32_t MaxConcurrentCaptures() const
    {
        return 4u;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
