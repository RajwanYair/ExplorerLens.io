//==============================================================================
// JPEG2000Decoder.cpp — JPEG 2000 Thumbnail Decoder Implementation
// ExplorerLens Engine v9.0.0-dev
//
// Implements the JPEG2000Decoder with OpenJPEG integration.
// Uses reduction-level decoding for efficient thumbnail generation from
// wavelet-compressed JP2/J2K/JPX/JPH images.
//==============================================================================

#include "JPEG2000Decoder.h"
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <fstream>

#ifdef HAS_OPENJPEG
    #include <openjpeg.h>
#endif

namespace ExplorerLens::Decoders {

//==============================================================================
// HBITMAP Creator Utility (shared with IThumbnailDecoder pipeline)
//==============================================================================

/// Create HBITMAP from raw BGRA pixel buffer
static HBITMAP CreateHBITMAPFromBGRA(const uint8_t* pixels, uint32_t width, uint32_t height)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(width);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height);  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hBitmap || !bits)
        return nullptr;
    memcpy(bits, pixels, static_cast<size_t>(width) * height * 4);
    return hBitmap;
}

//==============================================================================
// JPEG 2000 File Detection
//==============================================================================

/// Detect JP2 format from file magic bytes
static JP2Format DetectFormatFromMagic(const uint8_t* data, size_t size)
{
    if (size < 12)
        return JP2Format::Unknown;

    // JP2 box-based format: starts with JP2 signature box
    // 00 00 00 0C 6A 50 20 20 0D 0A 87 0A
    static constexpr uint8_t JP2_SIG[] = {0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A};
    if (memcmp(data, JP2_SIG, 12) == 0)
        return JP2Format::JP2;

    // Raw J2K codestream: starts with SOC marker FF 4F
    if (size >= 2 && data[0] == 0xFF && data[1] == 0x4F)
        return JP2Format::J2K;

    return JP2Format::Unknown;
}

//==============================================================================
// JPEG 2000 Decode — OpenJPEG Backend
//==============================================================================

#ifdef HAS_OPENJPEG

/**
 * Memory buffer context for OpenJPEG stream callbacks.
 * Wraps a raw pointer+size into a seekable read stream.
 */
struct OPJMemoryStream
{
    const uint8_t* data;
    OPJ_SIZE_T size;
    OPJ_SIZE_T offset;
};

static OPJ_SIZE_T opjMemRead(void* buffer, OPJ_SIZE_T nbBytes, void* userData)
{
    auto* ms = static_cast<OPJMemoryStream*>(userData);
    if (ms->offset >= ms->size)
        return static_cast<OPJ_SIZE_T>(-1);
    OPJ_SIZE_T avail = ms->size - ms->offset;
    OPJ_SIZE_T toRead = (nbBytes < avail) ? nbBytes : avail;
    memcpy(buffer, ms->data + ms->offset, toRead);
    ms->offset += toRead;
    return toRead;
}

static OPJ_OFF_T opjMemSkip(OPJ_OFF_T nbBytes, void* userData)
{
    auto* ms = static_cast<OPJMemoryStream*>(userData);
    if (nbBytes < 0) {
        if (static_cast<OPJ_SIZE_T>(-nbBytes) > ms->offset) {
            ms->offset = 0;
        } else {
            ms->offset -= static_cast<OPJ_SIZE_T>(-nbBytes);
        }
    } else {
        ms->offset += static_cast<OPJ_SIZE_T>(nbBytes);
        if (ms->offset > ms->size)
            ms->offset = ms->size;
    }
    return static_cast<OPJ_OFF_T>(ms->offset);
}

static OPJ_BOOL opjMemSeek(OPJ_OFF_T nbBytes, void* userData)
{
    auto* ms = static_cast<OPJMemoryStream*>(userData);
    if (nbBytes < 0 || static_cast<OPJ_SIZE_T>(nbBytes) > ms->size)
        return OPJ_FALSE;
    ms->offset = static_cast<OPJ_SIZE_T>(nbBytes);
    return OPJ_TRUE;
}

static void opjMemFree(void* userData)
{
    delete static_cast<OPJMemoryStream*>(userData);
}

/// Decode JPEG 2000 using OpenJPEG library
static J2KDecodeResult DecodeWithOpenJPEG(const uint8_t* data, size_t dataSize, JP2Format format,
                                          const JP2DecodeOptions& opts)
{
    J2KDecodeResult result;

    OPJ_CODEC_FORMAT codecFormat = OPJ_CODEC_JP2;
    if (format == JP2Format::J2K)
        codecFormat = OPJ_CODEC_J2K;
    else if (format == JP2Format::JPX)
        codecFormat = OPJ_CODEC_JPX;

    opj_codec_t* codec = opj_create_decompress(codecFormat);
    if (!codec) {
        result.status = JP2DecodeStatus::InternalError;
        return result;
    }

    opj_dparameters_t params;
    opj_set_default_decoder_parameters(&params);
    params.cp_reduce = opts.reductionLevel;

    if (!opj_setup_decoder(codec, &params)) {
        opj_destroy_codec(codec);
        result.status = JP2DecodeStatus::InternalError;
        return result;
    }

    // Create memory stream with OpenJPEG callbacks
    auto* memStream = new OPJMemoryStream{data, static_cast<OPJ_SIZE_T>(dataSize), 0};
    opj_stream_t* stream = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_TRUE);
    if (!stream) {
        delete memStream;
        opj_destroy_codec(codec);
        result.status = JP2DecodeStatus::InternalError;
        return result;
    }
    opj_stream_set_user_data(stream, memStream, opjMemFree);
    opj_stream_set_user_data_length(stream, dataSize);
    opj_stream_set_read_function(stream, opjMemRead);
    opj_stream_set_skip_function(stream, opjMemSkip);
    opj_stream_set_seek_function(stream, opjMemSeek);

    // Read header
    opj_image_t* image = nullptr;
    if (!opj_read_header(stream, codec, &image)) {
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        result.status = JP2DecodeStatus::InvalidFormat;
        return result;
    }

    // Decode
    if (!opj_decode(codec, stream, image)) {
        opj_image_destroy(image);
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        result.status = JP2DecodeStatus::CorruptData;
        return result;
    }

    // Extract pixel data
    uint32_t w = image->comps[0].w;
    uint32_t h = image->comps[0].h;
    uint32_t nc = image->numcomps;

    result.decodedWidth = w;
    result.decodedHeight = h;
    result.info.width = w;
    result.info.height = h;
    result.info.numComponents = static_cast<uint8_t>(nc);

    // Convert to BGRA
    result.pixelData.resize(static_cast<size_t>(w) * h * 4);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = static_cast<size_t>(y) * w + x;
            size_t out = idx * 4;
            if (nc >= 3) {
                result.pixelData[out + 2] = static_cast<uint8_t>(image->comps[0].data[idx]);                    // R
                result.pixelData[out + 1] = static_cast<uint8_t>(image->comps[1].data[idx]);                    // G
                result.pixelData[out + 0] = static_cast<uint8_t>(image->comps[2].data[idx]);                    // B
                result.pixelData[out + 3] = (nc >= 4) ? static_cast<uint8_t>(image->comps[3].data[idx]) : 255;  // A
            } else {
                // Grayscale
                uint8_t v = static_cast<uint8_t>(image->comps[0].data[idx]);
                result.pixelData[out + 0] = v;
                result.pixelData[out + 1] = v;
                result.pixelData[out + 2] = v;
                result.pixelData[out + 3] = 255;
            }
        }
    }

    opj_image_destroy(image);
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);

    result.status = JP2DecodeStatus::Success;
    return result;
}

#endif  // HAS_OPENJPEG

//==============================================================================
// JPEG 2000 Decode — Fallback (header parsing only, no pixel data)
//==============================================================================

#ifndef HAS_OPENJPEG
static J2KDecodeResult DecodeFallback(const uint8_t* data, size_t dataSize, JP2Format format)
{
    J2KDecodeResult result;

    if (format == JP2Format::J2K && dataSize >= 10) {
        // Parse J2K SIZ marker for dimensions
        // SOC(2) + SIZ marker type(2) + Lsiz(2) + Rsiz(2) + Xsiz(4) + Ysiz(4)
        size_t pos = 2;
        while (pos + 2 < dataSize) {
            uint8_t m1 = data[pos], m2 = data[pos + 1];
            if (m1 == 0xFF && m2 == 0x51) {  // SIZ marker
                if (pos + 10 >= dataSize)
                    break;
                uint16_t lsiz = (uint16_t(data[pos + 2]) << 8) | data[pos + 3];
                if (pos + lsiz + 2 > dataSize)
                    break;
                // Skip Rsiz(2), read Xsiz(4), Ysiz(4)
                uint32_t xsiz = (uint32_t(data[pos + 6]) << 24) | (uint32_t(data[pos + 7]) << 16)
                                | (uint32_t(data[pos + 8]) << 8) | data[pos + 9];
                uint32_t ysiz = (uint32_t(data[pos + 10]) << 24) | (uint32_t(data[pos + 11]) << 16)
                                | (uint32_t(data[pos + 12]) << 8) | data[pos + 13];
                result.decodedWidth = xsiz;
                result.decodedHeight = ysiz;
                result.info.width = xsiz;
                result.info.height = ysiz;
                break;
            }
            if (m1 == 0xFF) {
                uint16_t len = (uint16_t(data[pos + 2]) << 8) | data[pos + 3];
                pos += 2 + len;
            } else {
                pos++;
            }
        }
    }

    // Generate a placeholder thumbnail (blue gradient)
    uint32_t w = 256, h = 256;
    result.pixelData.resize(static_cast<size_t>(w) * h * 4);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = (static_cast<size_t>(y) * w + x) * 4;
            result.pixelData[idx + 0] = static_cast<uint8_t>(200 * y / h);  // B
            result.pixelData[idx + 1] = static_cast<uint8_t>(100 * x / w);  // G
            result.pixelData[idx + 2] = 40;                                 // R
            result.pixelData[idx + 3] = 255;                                // A
        }
    }
    result.decodedWidth = w;
    result.decodedHeight = h;
    result.status = JP2DecodeStatus::Success;
    return result;
}
#endif  // !HAS_OPENJPEG

//==============================================================================
// IThumbnailDecoder-compatible interface
//==============================================================================

/// Standalone function for shell extension integration
HRESULT DecodeJPEG2000Thumbnail(const wchar_t* filePath, uint32_t requestedSize, HBITMAP& hBitmap)
{
    if (!filePath || requestedSize == 0)
        return E_INVALIDARG;

    // Read file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize < 12)
        return E_FAIL;

    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    // Detect format
    JP2Format format = DetectFormatFromMagic(data.data(), data.size());
    if (format == JP2Format::Unknown) {
        // Try extension-based detection
        std::wstring path(filePath);
        size_t dot = path.rfind(L'.');
        if (dot != std::wstring::npos) {
            std::wstring ext = path.substr(dot);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            std::string extA;
            extA.reserve(ext.size());
            for (wchar_t ch : ext) {
                extA.push_back((ch <= 0x7F) ? static_cast<char>(ch) : '?');
            }
            format = JP2Extensions::ClassifyExtension(extA);
        }
    }

    // Decode
    J2KDecodeResult result;

#ifdef HAS_OPENJPEG
    JP2DecodeOptions opts = JP2DecodeOptions::Thumbnail(requestedSize);
    result = DecodeWithOpenJPEG(data.data(), data.size(), format, opts);
#else
    result = DecodeFallback(data.data(), data.size(), format);
#endif

    if (!result.IsSuccess() || !result.HasPixels())
        return E_FAIL;

    // Create HBITMAP
    hBitmap = CreateHBITMAPFromBGRA(result.pixelData.data(), result.decodedWidth, result.decodedHeight);
    return hBitmap ? S_OK : E_OUTOFMEMORY;
}

}  // namespace ExplorerLens::Decoders
