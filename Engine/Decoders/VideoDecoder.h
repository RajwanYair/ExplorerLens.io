// VideoDecoder.h - Video Thumbnail Decoder (Media Foundation)
// DarkThumbs Engine v6.2.0+
// Copyright (c) 2026 DarkThumbs Project
//
// Supports: MP4, MKV, AVI, WMV, MOV, FLV, WEBM, M4V, MPG, MPEG, TS, MTS
// Features:
// - Extracts key frame at configurable position (default: 10% into video)
// - Uses Windows Media Foundation (Win7+)
// - Falls back to DirectShow if MF unavailable
// - Thread-safe, COM-initialized per call

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>

namespace DarkThumbs {
namespace Engine {

class VideoDecoder : public IThumbnailDecoder {
public:
    VideoDecoder();
    ~VideoDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"VideoDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

    // Configuration
    void SetSeekPosition(float normalizedPos) { m_seekPosition = normalizedPos; }
    float GetSeekPosition() const { return m_seekPosition; }
    void EnableHardwareAcceleration(bool enable) { m_useHardwareAccel = enable; }
    bool IsHardwareAccelerationEnabled() const { return m_useHardwareAccel; }

    // Video metadata extraction
    struct VideoMetadata {
        uint32_t width = 0;
        uint32_t height = 0;
        uint64_t durationMs = 0;
        const wchar_t* codec = nullptr;
        bool hasKeyframes = false;
    };
    bool GetMetadata(const wchar_t* filePath, VideoMetadata& metadata);

private:
    // Frame extraction via Media Foundation Source Reader
    HRESULT ExtractFrameMF(const wchar_t* filePath, uint32_t width,
                           uint32_t height, HBITMAP* phBitmap);
    
    // Intelligent keyframe search  
    HRESULT SeekToKeyframe(IMFSourceReader* pReader, LONGLONG duration);

    // Fallback: Shell IThumbnailProvider for video
    HRESULT ExtractFrameShell(const wchar_t* filePath, uint32_t width,
                              uint32_t height, HBITMAP* phBitmap);

    // Format detection
    bool IsVideoFormat(const wchar_t* path);

    // Seek position (0.0 = start, 1.0 = end), default 10%
    float m_seekPosition = 0.10f;
    
    // Hardware acceleration (DXVA2)
    bool m_useHardwareAccel = true;
    
    // Keyframe search window (5 seconds)
    static constexpr LONGLONG KEYFRAME_SEARCH_WINDOW = 50000000; // 5 seconds in 100-nanosecond units

    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace DarkThumbs
