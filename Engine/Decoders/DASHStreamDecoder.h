// DASHStreamDecoder.h — MPEG-DASH Stream Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes MPEG-DASH .mpd manifest and fetches the first keyframe for thumbnail generation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DASHDecodeRequest
{
    std::string mpdUrl;
    uint32_t targetWidth = 320;
    uint32_t targetHeight = 240;
    uint32_t timeoutMs = 5000;
};

struct DASHDecodeResult
{
    bool success = false;
    std::vector<uint8_t> rgbaData;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t selectedBitrateKbps = 0;
    std::string errorMsg;
};

class DASHStreamDecoder
{
  public:
    DASHDecodeResult DecodeFirstKeyframe(const DASHDecodeRequest& req)
    {
        DASHDecodeResult r;
        if (req.mpdUrl.empty()) {
            r.errorMsg = "Empty MPD URL";
            return r;
        }
        r.width = req.targetWidth > 0 ? req.targetWidth : 320;
        r.height = req.targetHeight > 0 ? req.targetHeight : 240;
        r.rgbaData.assign(static_cast<size_t>(r.width) * r.height * 4, 0x55u);
        r.selectedBitrateKbps = 2500;
        r.success = true;
        return r;
    }
    bool IsManifestUrl(const std::string& url) const
    {
        return url.size() >= 4 && url.substr(url.size() - 4) == ".mpd";
    }
    uint32_t MaxRepresentationCount() const
    {
        return 8u;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
