// FFmpegCodecRouter.h — Route Video Formats to FFmpeg vs MediaFoundation
// Copyright (c) 2026 ExplorerLens Project
//
// Decides whether a video format should be decoded via FFmpeg or Windows
// Media Foundation based on codec support, performance, and system capabilities.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class VideoCodecPath : uint8_t {
    FFmpeg = 0,
    MediaFoundation = 1,
    GPUDecode = 2,
    Unsupported = 3
};

struct CodecRouteDecision
{
    std::string codecName;
    std::string container;
    VideoCodecPath selectedPath = VideoCodecPath::Unsupported;
    VideoCodecPath fallbackPath = VideoCodecPath::FFmpeg;
    bool hwAccelAvail = false;
    uint32_t priorityScore = 0;
    std::string reason;
};

struct VideoCodecCapability
{
    std::string codecId;
    bool ffmpegSupport = false;
    bool mediaFoundationSupport = false;
    bool gpuDecodeSupport = false;
    uint32_t ffmpegPriority = 50;
    uint32_t mfPriority = 50;
};

class FFmpegCodecRouter
{
  public:
    static FFmpegCodecRouter& Instance()
    {
        static FFmpegCodecRouter s;
        return s;
    }

    void RegisterCodec(const VideoCodecCapability& cap)
    {
        m_codecs[cap.codecId] = cap;
    }

    void InitializeDefaultCodecs()
    {
        RegisterCodec({"h264", true, true, true, 60, 80});
        RegisterCodec({"h265", true, true, true, 60, 70});
        RegisterCodec({"vp8", true, false, false, 80, 0});
        RegisterCodec({"vp9", true, true, true, 70, 60});
        RegisterCodec({"av1", true, true, true, 50, 70});
        RegisterCodec({"mpeg2", true, true, false, 70, 60});
        RegisterCodec({"mpeg4", true, true, false, 70, 50});
        RegisterCodec({"wmv3", false, true, false, 0, 90});
        RegisterCodec({"mjpeg", true, false, false, 90, 0});
        RegisterCodec({"prores", true, false, false, 80, 0});
        RegisterCodec({"dnxhd", true, false, false, 80, 0});
        RegisterCodec({"theora", true, false, false, 80, 0});
        m_initialized = true;
    }

    CodecRouteDecision RouteFormat(const std::string& codecId, const std::string& container = "") const
    {
        CodecRouteDecision decision{};
        decision.codecName = codecId;
        decision.container = container;

        auto it = m_codecs.find(codecId);
        if (it == m_codecs.end()) {
            decision.selectedPath = VideoCodecPath::Unsupported;
            decision.reason = "Unknown codec: " + codecId;
            return decision;
        }

        const auto& cap = it->second;
        decision.hwAccelAvail = cap.gpuDecodeSupport;

        // GPU decode preferred if available
        if (cap.gpuDecodeSupport && m_preferGPU) {
            decision.selectedPath = VideoCodecPath::GPUDecode;
            decision.fallbackPath = cap.ffmpegSupport ? VideoCodecPath::FFmpeg : VideoCodecPath::MediaFoundation;
            decision.priorityScore = 100;
            decision.reason = "GPU decode available";
            return decision;
        }

        // Choose between FFmpeg and MF based on priority scores
        if (cap.ffmpegSupport && cap.mediaFoundationSupport) {
            if (cap.ffmpegPriority >= cap.mfPriority) {
                decision.selectedPath = VideoCodecPath::FFmpeg;
                decision.fallbackPath = VideoCodecPath::MediaFoundation;
                decision.priorityScore = cap.ffmpegPriority;
                decision.reason = "FFmpeg preferred (priority " + std::to_string(cap.ffmpegPriority) + ")";
            } else {
                decision.selectedPath = VideoCodecPath::MediaFoundation;
                decision.fallbackPath = VideoCodecPath::FFmpeg;
                decision.priorityScore = cap.mfPriority;
                decision.reason = "MF preferred (priority " + std::to_string(cap.mfPriority) + ")";
            }
        } else if (cap.ffmpegSupport) {
            decision.selectedPath = VideoCodecPath::FFmpeg;
            decision.priorityScore = cap.ffmpegPriority;
            decision.reason = "FFmpeg only";
        } else if (cap.mediaFoundationSupport) {
            decision.selectedPath = VideoCodecPath::MediaFoundation;
            decision.priorityScore = cap.mfPriority;
            decision.reason = "MediaFoundation only";
        } else {
            decision.selectedPath = VideoCodecPath::Unsupported;
            decision.reason = "No decoder available";
        }

        return decision;
    }

    std::vector<std::string> GetSupportedFormats() const
    {
        std::vector<std::string> formats;
        for (const auto& [id, cap] : m_codecs) {
            if (cap.ffmpegSupport || cap.mediaFoundationSupport || cap.gpuDecodeSupport)
                formats.push_back(id);
        }
        std::sort(formats.begin(), formats.end());
        return formats;
    }

    std::vector<std::string> GetFFmpegOnlyFormats() const
    {
        std::vector<std::string> formats;
        for (const auto& [id, cap] : m_codecs) {
            if (cap.ffmpegSupport && !cap.mediaFoundationSupport)
                formats.push_back(id);
        }
        return formats;
    }

    void SetPreferGPU(bool prefer)
    {
        m_preferGPU = prefer;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    size_t CodecCount() const
    {
        return m_codecs.size();
    }

    bool Validate() const
    {
        if (!m_initialized)
            return true;  // not initialized is valid initial state
        if (m_codecs.empty())
            return false;
        for (const auto& [id, cap] : m_codecs) {
            if (id.empty())
                return false;
            if (!cap.ffmpegSupport && !cap.mediaFoundationSupport && !cap.gpuDecodeSupport)
                return false;  // all-disabled codec shouldn't be registered
        }
        return true;
    }

  private:
    FFmpegCodecRouter() = default;
    ~FFmpegCodecRouter() = default;
    FFmpegCodecRouter(const FFmpegCodecRouter&) = delete;
    FFmpegCodecRouter& operator=(const FFmpegCodecRouter&) = delete;

    std::unordered_map<std::string, VideoCodecCapability> m_codecs;
    bool m_preferGPU = true;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
