// AudioDecoder.h - Audio Album Art / Waveform Thumbnail Decoder
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: MP3, FLAC, WMA, AAC, M4A, OGG, OPUS, WAV, AIFF, APE, WV
// Features:
// - Extracts embedded album art (ID3v2 APIC, FLAC PICTURE, WMA)
// - Falls back to Windows Property System for tag reading
// - Generates waveform visualization when no art found
// - Thread-safe, no global state

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <memory>
#include <string>

namespace ExplorerLens {
namespace Engine {

class AudioDecoder : public IThumbnailDecoder {
public:
    AudioDecoder();
    ~AudioDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"AudioDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

    // Audio metadata extraction
    struct AudioMetadata {
        std::wstring artist;
        std::wstring album;
        std::wstring title;
        uint32_t durationSec = 0;
        uint32_t bitrate = 0;
    };
    bool GetAudioMetadata(const wchar_t* filePath, AudioMetadata& metadata);

private:
    // Album art extraction
    HRESULT ExtractAlbumArt(const wchar_t* filePath, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtMP3(const uint8_t* data, size_t size, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtFLAC(const uint8_t* data, size_t size, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtOGG(const uint8_t* data, size_t size, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtM4A(const uint8_t* data, size_t size, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtWMA(const wchar_t* filePath, HBITMAP* phBitmap);
    HRESULT ExtractAlbumArtPropertySystem(const wchar_t* filePath, HBITMAP* phBitmap);

    // Waveform fallback
    HBITMAP GenerateWaveformPlaceholder(uint32_t width, uint32_t height, const wchar_t* ext);

    // Image data to HBITMAP (JPEG/PNG embedded art)
    HBITMAP CreateBitmapFromImageData(const uint8_t* data, size_t size);

    // Utility
    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);
    bool IsAudioFormat(const wchar_t* path);

    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

