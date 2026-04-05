// VideoFrameExtractor.h — DXVA2 / MediaFoundation GPU-assisted video frame extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Seeks to a timestamp within a video file and extracts the nearest keyframe as a
// BGRA pixel buffer. Routes to DXVA2 on systems with a supported GPU; falls back
// to the software MediaFoundation source reader on all other configurations.
//
#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace ExplorerLens { namespace Engine {

enum class VideoDecodeBackend : uint8_t
{
    DXVA2_HARDWARE = 0,
    MF_SOFTWARE    = 1,
    UNAVAILABLE    = 2,
};

struct VideoFrameRequest
{
    std::wstring filePath;
    double       timestampSeconds = 0.0;
    uint32_t     targetWidth      = 256;
    uint32_t     targetHeight     = 256;
};

struct ScrubberFrameResult
{
    bool               success     = false;
    uint32_t           width       = 0;
    uint32_t           height      = 0;
    uint32_t           strideBytes = 0;
    double             actualPts   = 0.0;
    VideoDecodeBackend backend     = VideoDecodeBackend::UNAVAILABLE;
};

class VideoFrameExtractor
{
public:
    static VideoFrameExtractor& Instance() noexcept;

    ScrubberFrameResult   ExtractFrame(const VideoFrameRequest& req) noexcept;
    bool               SeekToTimestamp(double timestampSeconds)   noexcept;
    void               Reset()                                    noexcept;

    static std::string_view BackendName(VideoDecodeBackend b) noexcept;

    VideoDecodeBackend CurrentBackend() const noexcept { return m_backend;       }
    uint32_t           ExtractCount()   const noexcept { return m_extractCount;  }
    double             LastPts()        const noexcept { return m_lastPts;       }

private:
    VideoDecodeBackend m_backend      = VideoDecodeBackend::UNAVAILABLE;
    uint32_t           m_extractCount = 0;
    double             m_lastPts      = 0.0;
};

}} // namespace ExplorerLens::Engine
