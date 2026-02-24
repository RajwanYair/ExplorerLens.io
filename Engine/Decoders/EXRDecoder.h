// EXRDecoder.h - OpenEXR Image Decoder
// ExplorerLens Engine v6.1.0+
// Uses WIC when Microsoft OpenEXR codec is available, basic header extraction otherwise

#pragma once
#include "../Core/IThumbnailDecoder.h"
#include <memory>
#include <wincodec.h>
#include <wrl/client.h>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

class EXRDecoder : public IThumbnailDecoder {
public:
    EXRDecoder();
    ~EXRDecoder();

    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"EXRDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;

    static Microsoft::WRL::ComPtr<IWICImagingFactory> s_wicFactory;
    static std::mutex s_factoryMutex;

    HRESULT DecodeFromFile(const wchar_t* path, uint32_t maxSize, HBITMAP* phBitmap);
    HRESULT DecodeWithWIC(const wchar_t* path, uint32_t maxSize, HBITMAP* phBitmap);
    HRESULT EnsureWICFactory();

    std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* path, size_t& fileSize);
};

} // namespace Engine
} // namespace ExplorerLens

