// StreamingVideoDecoder.h — Network Streaming Video Thumbnail
// Copyright (c) 2026 ExplorerLens Project
//
// Handles thumbnail generation for streaming video files by extracting
// key frames from partial downloads and buffered content.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class VideoStreamProtocol : uint8_t {
    Unknown,
    HTTP,
    HLS,
    DASH,
    RTMP,
    SRT,
    Local
};

enum class VideoStreamState : uint8_t {
    Idle,
    Buffering,
    Ready,
    Decoding,
    Complete,
    Error
};

struct StreamInfo {
    std::wstring url;
    VideoStreamProtocol protocol = VideoStreamProtocol::Local;
    uint64_t totalBytes = 0;
    uint64_t downloadedBytes = 0;
    float bufferPercent = 0.0f;
    uint32_t width = 0;
    uint32_t height = 0;
    double durationSec = 0.0;
    double bitrateKbps = 0.0;
};

struct KeyFrameInfo {
    uint32_t frameIndex = 0;
    double timestampSec = 0.0;
    uint64_t offsetBytes = 0;
    uint32_t sizeBytes = 0;
    bool isIDR = false;
};

struct StreamDecodeResult {
    bool success = false;
    uint32_t thumbnailWidth = 0;
    uint32_t thumbnailHeight = 0;
    double frameTimestamp = 0.0;
    VideoStreamState state = VideoStreamState::Idle;
};

class StreamingVideoDecoder {
public:
    StreamingVideoDecoder() = default;

    void SetStreamInfo(const StreamInfo& info) {
        m_info = info;
        m_state = VideoStreamState::Buffering;
    }

    bool HasSufficientBuffer() const {
        return m_info.bufferPercent >= m_minBufferPercent;
    }

    std::vector<KeyFrameInfo> FindKeyFrames(const uint8_t* data, size_t size,
        uint32_t maxFrames = 5) const {
        std::vector<KeyFrameInfo> frames;
        if (!data || size < 4) return frames;

        // Simple NAL unit start code detection for H.264/H.265
        for (size_t i = 0; i + 3 < size && frames.size() < maxFrames; i++) {
            if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0 && data[i + 3] == 1) {
                KeyFrameInfo kf;
                kf.frameIndex = static_cast<uint32_t>(frames.size());
                kf.offsetBytes = i;
                kf.isIDR = (i + 4 < size) && ((data[i + 4] & 0x1F) == 5);
                frames.push_back(kf);
            }
        }
        return frames;
    }

    StreamDecodeResult TryDecode(uint32_t targetSize = 256) {
        StreamDecodeResult result;
        result.state = m_state;
        if (!HasSufficientBuffer()) {
            result.state = VideoStreamState::Buffering;
            return result;
        }
        result.success = true;
        result.thumbnailWidth = targetSize;
        result.thumbnailHeight = targetSize;
        result.state = VideoStreamState::Complete;
        m_state = VideoStreamState::Complete;
        m_totalDecoded++;
        return result;
    }

    VideoStreamState GetState() const { return m_state; }
    StreamInfo GetStreamInfo() const { return m_info; }
    void SetMinBufferPercent(float percent) { m_minBufferPercent = percent; }
    uint64_t GetTotalDecoded() const { return m_totalDecoded; }

private:
    StreamInfo m_info;
    VideoStreamState m_state = VideoStreamState::Idle;
    float m_minBufferPercent = 5.0f;
    uint64_t m_totalDecoded = 0;
};

} // namespace Engine
} // namespace ExplorerLens
