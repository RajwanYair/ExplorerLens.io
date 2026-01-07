#pragma once

#include "../contracts/thumbnail_contracts.h"
#include <memory>
#include <vector>
#include <string>

namespace DarkThumbs::Engine::Interfaces {

    using namespace DarkThumbs::Engine::Contracts;

    class IThumbnailDecoder {
    public:
        virtual ~IThumbnailDecoder() = default;
        // Decode logic. pixelData is an output buffer.
        virtual HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result, std::vector<uint8_t>& pixelData) = 0;
        virtual bool SupportsFormat(const std::wstring& extension) = 0;
        virtual const wchar_t* GetName() const = 0;
    };

    class ICacheProvider {
    public:
        virtual ~ICacheProvider() = default;
        virtual HRESULT Get(const CacheKeyV2& key, std::vector<uint8_t>& data) = 0;
        virtual HRESULT Set(const CacheKeyV2& key, const std::vector<uint8_t>& data) = 0;
        virtual HRESULT Clear() = 0;
    };

    class IGPURenderer {
    public:
        virtual ~IGPURenderer() = default;
        virtual HRESULT Render(const std::vector<uint8_t>& inputPixels, uint32_t width, uint32_t height, std::vector<uint8_t>& outputPixels) = 0;
        virtual bool IsAvailable() const = 0;
    };

    class IFormatDetector {
    public:
        virtual ~IFormatDetector() = default;
        virtual HRESULT DetectFormat(const std::wstring& path, std::wstring& detectedFormat) = 0;
    };

    class IArchiveReader {
    public:
        virtual ~IArchiveReader() = default;
        virtual HRESULT Open(const std::wstring& path) = 0;
        virtual HRESULT ExtractFile(const std::wstring& filename, std::vector<uint8_t>& data) = 0;
        virtual HRESULT ListFiles(std::vector<std::wstring>& fileList) = 0;
    };

    class IThumbnailPipeline {
    public:
        virtual ~IThumbnailPipeline() = default;
        // The main entry point for generating a thumbnail
        virtual HRESULT GenerateThumbnail(const ThumbnailRequest& request, ThumbnailResult& result, std::vector<uint8_t>& finalImage) = 0;
    };

}
