// AudioDecoder.cpp - Audio Album Art / Waveform Thumbnail Decoder
// Implementation ExplorerLens Engine v6.2.0+ Copyright (c) 2026 ExplorerLens
// Project

#include "AudioDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <algorithm>
#include <cstring>
#include <cwchar>
#include <gdiplus.h>
#include <objidl.h>
#include <propkey.h>
#include <propsys.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <wincodec.h>
#include <windows.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace ExplorerLens {
namespace Engine {

const wchar_t *AudioDecoder::m_extensions[] = {
    L".mp3",  L".flac", L".wma", L".aac", L".m4a",  L".ogg", L".opus", L".wav",
    L".aiff", L".aif",  L".ape", L".wv",  L".alac", L".mpc", nullptr};
const uint32_t AudioDecoder::m_extensionCount = 14;

AudioDecoder::AudioDecoder() = default;
AudioDecoder::~AudioDecoder() = default;

bool AudioDecoder::CanDecode(const wchar_t *filePath) {
  if (!filePath)
    return false;
  return IsAudioFormat(filePath);
}

HRESULT AudioDecoder::Decode(const ThumbnailRequest &request,
                             ThumbnailResult &result) {
  PROFILE_SCOPE(ProfileComponent::DECODE_AUDIO);

  result.hBitmap = nullptr;
  result.width = 0;
  result.height = 0;
  if (!request.filePath)
    return E_INVALIDARG;

  // Try to extract embedded album art
  HRESULT hr = ExtractAlbumArt(request.filePath, &result.hBitmap);

  // Fallback: generate waveform visualization placeholder
  if (FAILED(hr) || !result.hBitmap) {
    const wchar_t *ext = PathFindExtensionW(request.filePath);
    result.hBitmap =
        GenerateWaveformPlaceholder(request.width, request.height, ext);
    hr = result.hBitmap ? S_OK : E_FAIL;
  }

  if (SUCCEEDED(hr) && result.hBitmap) {
    BITMAP bm;
    if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
      result.width = bm.bmWidth;
      result.height = bm.bmHeight;
    }
  }
  return hr;
}

DecoderInfo AudioDecoder::GetInfo() const {
  DecoderInfo info;
  info.name = L"Audio Decoder";
  info.version = L"1.0.0";
  info.supportedExtensions = const_cast<const wchar_t **>(m_extensions);
  info.extensionCount = m_extensionCount;
  info.supportsGPU = false;
  info.isArchiveDecoder = false;
  return info;
}

const wchar_t **AudioDecoder::GetSupportedExtensions() const {
  return const_cast<const wchar_t **>(m_extensions);
}

// ============================================================================
// Album Art Extraction
// ============================================================================

HRESULT AudioDecoder::ExtractAlbumArt(const wchar_t *filePath,
                                      HBITMAP *phBitmap) {
  if (!phBitmap)
    return E_INVALIDARG;
  *phBitmap = nullptr;

  // Read file for manual tag parsing
  size_t fileSize = 0;
  auto data = ReadFileData(filePath, fileSize);
  if (!data || fileSize == 0)
    return E_FAIL;

  // Try MP3 ID3v2 first
  if (fileSize >= 10 && data[0] == 'I' && data[1] == 'D' && data[2] == '3') {
    HRESULT hr = ExtractAlbumArtMP3(data.get(), fileSize, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;
  }

  // Try FLAC
  if (fileSize >= 4 && data[0] == 'f' && data[1] == 'L' && data[2] == 'a' &&
      data[3] == 'C') {
    HRESULT hr = ExtractAlbumArtFLAC(data.get(), fileSize, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;
  }

  // Try OGG Vorbis/Opus
  if (fileSize >= 4 && data[0] == 'O' && data[1] == 'g' && data[2] == 'g' &&
      data[3] == 'S') {
    HRESULT hr = ExtractAlbumArtOGG(data.get(), fileSize, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;
  }

  // Try M4A/AAC (ftyp signature)
  if (fileSize >= 12 && data[4] == 'f' && data[5] == 't' && data[6] == 'y' &&
      data[7] == 'p') {
    HRESULT hr = ExtractAlbumArtM4A(data.get(), fileSize, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;
  }

  // Try WMA using Property System
  const wchar_t *ext = PathFindExtensionW(filePath);
  if (ext && _wcsicmp(ext, L".wma") == 0) {
    HRESULT hr = ExtractAlbumArtWMA(filePath, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;
  }

  // Fallback to Windows Property System (handles WMA, M4A, etc.)
  return ExtractAlbumArtPropertySystem(filePath, phBitmap);
}

HRESULT AudioDecoder::ExtractAlbumArtMP3(const uint8_t *data, size_t size,
                                         HBITMAP *phBitmap) {
  // Parse ID3v2 header
  if (size < 10)
    return E_FAIL;

  uint8_t majorVersion = data[3];
  // uint8_t revision = data[4];
  // uint8_t flags = data[5];

  // ID3v2 size (syncsafe integer)
  uint32_t tagSize = ((data[6] & 0x7F) << 21) | ((data[7] & 0x7F) << 14) |
                     ((data[8] & 0x7F) << 7) | (data[9] & 0x7F);

  if (tagSize + 10 > size)
    tagSize = static_cast<uint32_t>(size - 10);

  size_t offset = 10;

  // Skip extended header if present (flag bit 6)
  // For simplicity, just scan for APIC frame

  while (offset + 10 < 10 + tagSize) {
    // Frame header: 4-byte ID, 4-byte size, 2-byte flags
    char frameId[5] = {};
    memcpy(frameId, data + offset, 4);

    uint32_t frameSize = 0;
    if (majorVersion >= 4) {
      // v2.4: syncsafe integer
      frameSize = ((data[offset + 4] & 0x7F) << 21) |
                  ((data[offset + 5] & 0x7F) << 14) |
                  ((data[offset + 6] & 0x7F) << 7) | (data[offset + 7] & 0x7F);
    } else {
      // v2.3: regular big-endian
      frameSize = (data[offset + 4] << 24) | (data[offset + 5] << 16) |
                  (data[offset + 6] << 8) | data[offset + 7];
    }

    if (frameSize == 0 || offset + 10 + frameSize > size)
      break;

    if (strcmp(frameId, "APIC") == 0 && frameSize > 20) {
      // APIC frame: encoding(1), mime(null-term), type(1), desc(null-term),
      // data
      size_t apicOffset = offset + 10;
      // uint8_t encoding = data[apicOffset];
      apicOffset++; // skip encoding

      // Skip MIME type
      while (apicOffset < offset + 10 + frameSize && data[apicOffset] != 0)
        apicOffset++;
      apicOffset++; // skip null

      if (apicOffset < offset + 10 + frameSize) {
        // uint8_t pictureType = data[apicOffset];
        apicOffset++; // skip picture type

        // Skip description
        while (apicOffset < offset + 10 + frameSize && data[apicOffset] != 0)
          apicOffset++;
        apicOffset++; // skip null

        size_t imageSize = (offset + 10 + frameSize) - apicOffset;
        if (imageSize > 0 && apicOffset + imageSize <= size) {
          *phBitmap = CreateBitmapFromImageData(data + apicOffset, imageSize);
          if (*phBitmap)
            return S_OK;
        }
      }
    }

    offset += 10 + frameSize;
  }

  return E_FAIL;
}

HRESULT AudioDecoder::ExtractAlbumArtFLAC(const uint8_t *data, size_t size,
                                          HBITMAP *phBitmap) {
  // Parse FLAC metadata blocks
  size_t offset = 4; // Skip "fLaC"

  while (offset + 4 < size) {
    bool isLast = (data[offset] & 0x80) != 0;
    uint8_t blockType = data[offset] & 0x7F;
    uint32_t blockSize =
        (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
    offset += 4;

    if (offset + blockSize > size)
      break;

    // Block type 6 = PICTURE
    if (blockType == 6 && blockSize > 32) {
      size_t picOffset = offset;
      // uint32_t pictureType = (big endian)
      picOffset += 4; // skip picture type

      // MIME type length + string
      uint32_t mimeLen = (data[picOffset] << 24) | (data[picOffset + 1] << 16) |
                         (data[picOffset + 2] << 8) | data[picOffset + 3];
      picOffset += 4 + mimeLen;

      // Description length + string
      if (picOffset + 4 > offset + blockSize) {
        offset += blockSize;
        continue;
      }
      uint32_t descLen = (data[picOffset] << 24) | (data[picOffset + 1] << 16) |
                         (data[picOffset + 2] << 8) | data[picOffset + 3];
      picOffset += 4 + descLen;

      // Width, height, color depth, indexed colors = 4*4 = 16 bytes
      picOffset += 16;

      // Image data length
      if (picOffset + 4 > offset + blockSize) {
        offset += blockSize;
        continue;
      }
      uint32_t imageLen = (data[picOffset] << 24) |
                          (data[picOffset + 1] << 16) |
                          (data[picOffset + 2] << 8) | data[picOffset + 3];
      picOffset += 4;

      if (imageLen > 0 && picOffset + imageLen <= size) {
        *phBitmap = CreateBitmapFromImageData(data + picOffset, imageLen);
        if (*phBitmap)
          return S_OK;
      }
    }

    offset += blockSize;
    if (isLast)
      break;
  }

  return E_FAIL;
}

// ============================================================================
// OGG Album Art Extraction
// ============================================================================

HRESULT AudioDecoder::ExtractAlbumArtOGG(const uint8_t *data, size_t size,
                                         HBITMAP *phBitmap) {
  // OGG Vorbis: Parse Vorbis comment headers for METADATA_BLOCK_PICTURE
  // The picture tag is base64-encoded FLAC picture block (RFC 7845)
  if (size < 58)
    return E_FAIL; // Minimum OGG page header

  // Search for METADATA_BLOCK_PICTURE= in Vorbis comments
  const char *searchKey = "METADATA_BLOCK_PICTURE=";
  const size_t keyLen = 23;
  size_t offset = 0;

  while (offset + keyLen < size) {
    // Case-insensitive search for the key
    bool found = true;
    for (size_t i = 0; i < keyLen && found; ++i) {
      char c = static_cast<char>(data[offset + i]);
      char k = searchKey[i];
      if (c >= 'a' && c <= 'z')
        c -= 32;
      if (k >= 'a' && k <= 'z')
        k -= 32;
      if (c != k)
        found = false;
    }

    if (found) {
      // Decode base64 data following the key
      offset += keyLen;
      size_t b64End = offset;
      while (b64End < size && data[b64End] != '\0' && data[b64End] != '\n' &&
             data[b64End] != '\r') {
        ++b64End;
      }

      size_t b64Len = b64End - offset;
      if (b64Len < 16) {
        ++offset;
        continue;
      }

      // Base64 decode
      static const uint8_t b64Table[256] = {
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 62,  255, 255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,
          60,  61,  255, 255, 255, 0,   255, 255, 255, 0,   1,   2,   3,   4,
          5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,
          19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 255, 255, 26,
          27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
          41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
          255, 255, 255, 255,
      };

      std::vector<uint8_t> decoded;
      decoded.reserve(b64Len * 3 / 4);
      uint32_t accum = 0;
      int bits = 0;
      for (size_t i = offset; i < b64End; ++i) {
        uint8_t v = b64Table[data[i]];
        if (v == 255)
          continue;
        accum = (accum << 6) | v;
        bits += 6;
        if (bits >= 8) {
          bits -= 8;
          decoded.push_back(static_cast<uint8_t>((accum >> bits) & 0xFF));
        }
      }

      if (decoded.size() < 32) {
        ++offset;
        continue;
      }

      // FLAC picture block structure:
      // [4] picture type, [4] MIME length, [MIME] mime type,
      // [4] desc length, [desc] description,
      // [4] width, [4] height, [4] bit depth, [4] colors,
      // [4] data length, [data] image data
      auto readBE32 = [&](size_t off) -> uint32_t {
        return (static_cast<uint32_t>(decoded[off]) << 24) |
               (static_cast<uint32_t>(decoded[off + 1]) << 16) |
               (static_cast<uint32_t>(decoded[off + 2]) << 8) |
               static_cast<uint32_t>(decoded[off + 3]);
      };

      size_t pos = 0;
      /*uint32_t pictureType =*/readBE32(pos);
      pos += 4;
      uint32_t mimeLen = readBE32(pos);
      pos += 4;
      pos += mimeLen; // skip MIME type
      if (pos + 4 > decoded.size()) {
        ++offset;
        continue;
      }
      uint32_t descLen = readBE32(pos);
      pos += 4;
      pos += descLen; // skip description
      pos += 16;      // skip width(4) + height(4) + bitdepth(4) + colors(4)
      if (pos + 4 > decoded.size()) {
        ++offset;
        continue;
      }
      uint32_t imgDataLen = readBE32(pos);
      pos += 4;
      if (pos + imgDataLen > decoded.size()) {
        ++offset;
        continue;
      }

      *phBitmap = CreateBitmapFromImageData(decoded.data() + pos, imgDataLen);
      if (*phBitmap)
        return S_OK;
    }
    ++offset;
  }

  return E_FAIL;
}

// ============================================================================
// M4A/AAC Album Art Extraction
// ============================================================================

HRESULT AudioDecoder::ExtractAlbumArtM4A(const uint8_t *data, size_t size,
                                         HBITMAP *phBitmap) {
  // M4A uses MP4 container with 'covr' atom in 'meta' > 'ilst'
  // Parse atom hierarchy to find cover art
  size_t offset = 0;

  while (offset + 8 < size) {
    uint32_t atomSize = (data[offset] << 24) | (data[offset + 1] << 16) |
                        (data[offset + 2] << 8) | data[offset + 3];

    if (atomSize < 8 || offset + atomSize > size)
      break;

    // Check for 'covr' atom
    if (offset + 8 <= size && data[offset + 4] == 'c' &&
        data[offset + 5] == 'o' && data[offset + 6] == 'v' &&
        data[offset + 7] == 'r') {

      // Skip atom header and data header (typically 16 bytes)
      size_t imgOffset = offset + 16;
      if (imgOffset < offset + atomSize) {
        size_t imgSize = atomSize - 16;
        *phBitmap = CreateBitmapFromImageData(data + imgOffset, imgSize);
        if (*phBitmap)
          return S_OK;
      }
    }

    offset += atomSize;
  }

  return E_FAIL;
}

// ============================================================================
// WMA Album Art Extraction
// ============================================================================

HRESULT AudioDecoder::ExtractAlbumArtWMA(const wchar_t *filePath,
                                         HBITMAP *phBitmap) {
  // WMA uses Property System - delegate to PropertySystem method
  return ExtractAlbumArtPropertySystem(filePath, phBitmap);
}

// ============================================================================
// Property System Album Art Extraction
// ============================================================================

HRESULT AudioDecoder::ExtractAlbumArtPropertySystem(const wchar_t *filePath,
                                                    HBITMAP *phBitmap) {
  // Use Windows Property System to get thumbnail
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
  if (!comInit)
    return hr;

  IPropertyStore *pStore = nullptr;
  hr = SHGetPropertyStoreFromParsingName(filePath, nullptr, GPS_DEFAULT,
                                         IID_PPV_ARGS(&pStore));

  if (SUCCEEDED(hr) && pStore) {
    PROPVARIANT pv;
    PropVariantInit(&pv);
    hr = pStore->GetValue(PKEY_ThumbnailStream, &pv);
    if (SUCCEEDED(hr) && pv.vt == VT_STREAM && pv.pStream) {
      // Read stream into buffer
      STATSTG stat;
      if (SUCCEEDED(pv.pStream->Stat(&stat, STATFLAG_NONAME))) {
        size_t imgSize = static_cast<size_t>(stat.cbSize.QuadPart);
        if (imgSize > 0 && imgSize < 10 * 1024 * 1024) {
          auto imgData = std::make_unique<uint8_t[]>(imgSize);
          ULONG bytesRead = 0;
          if (SUCCEEDED(pv.pStream->Read(
                  imgData.get(), static_cast<ULONG>(imgSize), &bytesRead))) {
            *phBitmap = CreateBitmapFromImageData(imgData.get(), bytesRead);
          }
        }
      }
    }
    PropVariantClear(&pv);
    pStore->Release();
  }

  CoUninitialize();
  return (*phBitmap) ? S_OK : E_FAIL;
}

// ============================================================================
// Waveform Placeholder
// ============================================================================

HBITMAP AudioDecoder::GenerateWaveformPlaceholder(uint32_t width,
                                                  uint32_t height,
                                                  const wchar_t *ext) {
  Gdiplus::GdiplusStartupInput gdipInput;
  ULONG_PTR gdipToken = 0;
  if (Gdiplus::GdiplusStartup(&gdipToken, &gdipInput, nullptr) != Gdiplus::Ok)
    return nullptr;

  auto bmp =
      std::make_unique<Gdiplus::Bitmap>(width, height, PixelFormat32bppARGB);
  Gdiplus::Graphics g(bmp.get());
  g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

  // Dark background
  g.Clear(Gdiplus::Color(255, 30, 30, 35));

  // Draw waveform bars
  float barWidth = width / 48.0f;
  float maxBarH = height * 0.6f;
  Gdiplus::SolidBrush barBrush(Gdiplus::Color(200, 30, 180, 230));

  // Pseudo-random waveform pattern (deterministic for consistency)
  uint32_t seed = 0x12345678;
  for (int i = 0; i < 48; i++) {
    seed = seed * 1103515245 + 12345;
    float h = (seed % 1000) / 1000.0f * maxBarH;
    if (h < maxBarH * 0.1f)
      h = maxBarH * 0.1f;

    float x = i * barWidth + barWidth * 0.1f;
    float bw = barWidth * 0.8f;
    float y = (height - h) / 2.0f;

    g.FillRectangle(&barBrush, x, y, bw, h);
  }

  // Format label
  Gdiplus::FontFamily fontFamily(L"Segoe UI");
  float fontSize = height * 0.1f;
  if (fontSize < 8)
    fontSize = 8;
  Gdiplus::Font font(&fontFamily, fontSize, Gdiplus::FontStyleBold,
                     Gdiplus::UnitPixel);
  Gdiplus::SolidBrush textBrush(Gdiplus::Color(200, 255, 255, 255));

  std::wstring label = L"\xD83C\xDFB5"; // Music note
  if (ext && ext[0] == L'.') {
    label = ext + 1;
    // Uppercase
    for (auto &c : label)
      c = towupper(c);
  }

  Gdiplus::StringFormat fmt;
  fmt.SetAlignment(Gdiplus::StringAlignmentCenter);
  Gdiplus::RectF labelRect(0, height * 0.82f, static_cast<float>(width),
                           fontSize * 1.5f);
  g.DrawString(label.c_str(), -1, &font, labelRect, &fmt, &textBrush);

  HBITMAP hBitmap = nullptr;
  bmp->GetHBITMAP(Gdiplus::Color(255, 30, 30, 35), &hBitmap);
  Gdiplus::GdiplusShutdown(gdipToken);
  return hBitmap;
}

// ============================================================================
// WIC Image Data to HBITMAP
// ============================================================================

HBITMAP AudioDecoder::CreateBitmapFromImageData(const uint8_t *data,
                                                size_t size) {
  if (!data || size == 0)
    return nullptr;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
  if (!comInit)
    return nullptr;

  IWICImagingFactory *pFactory = nullptr;
  hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&pFactory));
  if (FAILED(hr) || !pFactory) {
    CoUninitialize();
    return nullptr;
  }

  IWICStream *pStream = nullptr;
  hr = pFactory->CreateStream(&pStream);
  if (FAILED(hr) || !pStream) {
    pFactory->Release();
    CoUninitialize();
    return nullptr;
  }

  hr = pStream->InitializeFromMemory(const_cast<BYTE *>(data),
                                     static_cast<DWORD>(size));
  if (FAILED(hr)) {
    pStream->Release();
    pFactory->Release();
    CoUninitialize();
    return nullptr;
  }

  IWICBitmapDecoder *pDecoder = nullptr;
  hr = pFactory->CreateDecoderFromStream(
      pStream, nullptr, WICDecodeMetadataCacheOnDemand, &pDecoder);

  HBITMAP hBitmap = nullptr;

  if (SUCCEEDED(hr) && pDecoder) {
    IWICBitmapFrameDecode *pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (SUCCEEDED(hr) && pFrame) {
      IWICFormatConverter *pConverter = nullptr;
      hr = pFactory->CreateFormatConverter(&pConverter);
      if (SUCCEEDED(hr) && pConverter) {
        hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppBGRA,
                                    WICBitmapDitherTypeNone, nullptr, 0.0,
                                    WICBitmapPaletteTypeCustom);
        if (SUCCEEDED(hr)) {
          UINT w = 0, h = 0;
          pConverter->GetSize(&w, &h);
          if (w > 0 && h > 0) {
            BITMAPINFO bmi = {};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = w;
            bmi.bmiHeader.biHeight = -(LONG)h; // top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            BYTE *pBits = nullptr;
            HDC hdc = GetDC(nullptr);
            hBitmap =
                CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS,
                                 reinterpret_cast<void **>(&pBits), nullptr, 0);
            ReleaseDC(nullptr, hdc);

            if (hBitmap && pBits) {
              UINT stride = w * 4;
              hr = pConverter->CopyPixels(nullptr, stride, stride * h, pBits);
              if (FAILED(hr)) {
                DeleteObject(hBitmap);
                hBitmap = nullptr;
              }
            }
          }
        }
        pConverter->Release();
      }
      pFrame->Release();
    }
    pDecoder->Release();
  }

  pStream->Release();
  pFactory->Release();
  CoUninitialize();
  return hBitmap;
}

// ============================================================================
// Utilities
// ============================================================================

std::unique_ptr<uint8_t[]> AudioDecoder::ReadFileData(const wchar_t *path,
                                                      size_t &fileSize) {
  fileSize = 0;
  HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (hFile == INVALID_HANDLE_VALUE)
    return nullptr;

  LARGE_INTEGER size;
  if (!GetFileSizeEx(hFile, &size) ||
      size.QuadPart > 200 * 1024 * 1024) { // 200MB limit for audio
    CloseHandle(hFile);
    return nullptr;
  }

  // Only read first 2MB for tag parsing (album art is usually in header)
  size_t readSize = static_cast<size_t>(
      (std::min)(size.QuadPart, (LONGLONG)(2 * 1024 * 1024)));
  fileSize = readSize;
  auto buffer = std::make_unique<uint8_t[]>(readSize);
  DWORD bytesRead = 0;
  if (!ReadFile(hFile, buffer.get(), static_cast<DWORD>(readSize), &bytesRead,
                nullptr)) {
    CloseHandle(hFile);
    return nullptr;
  }
  fileSize = bytesRead;
  CloseHandle(hFile);
  return buffer;
}

bool AudioDecoder::IsAudioFormat(const wchar_t *path) {
  if (!path)
    return false;
  const wchar_t *ext = PathFindExtensionW(path);
  if (!ext || ext[0] == L'\0')
    return false;
  for (int i = 0; m_extensions[i] != nullptr; i++) {
    if (_wcsicmp(ext, m_extensions[i]) == 0)
      return true;
  }
  return false;
}

// ============================================================================
// Audio Metadata Extraction
// ============================================================================

bool AudioDecoder::GetAudioMetadata(const wchar_t *filePath,
                                    AudioMetadata &metadata) {
  if (!filePath)
    return false;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
  if (!comInit)
    return false;

  IPropertyStore *pStore = nullptr;
  hr = SHGetPropertyStoreFromParsingName(filePath, nullptr, GPS_DEFAULT,
                                         IID_PPV_ARGS(&pStore));

  if (SUCCEEDED(hr) && pStore) {
    PROPVARIANT pv;

    // Artist
    PropVariantInit(&pv);
    if (SUCCEEDED(pStore->GetValue(PKEY_Music_Artist, &pv)) &&
        pv.vt == VT_LPWSTR) {
      metadata.artist = pv.pwszVal;
    }
    PropVariantClear(&pv);

    // Album
    PropVariantInit(&pv);
    if (SUCCEEDED(pStore->GetValue(PKEY_Music_AlbumTitle, &pv)) &&
        pv.vt == VT_LPWSTR) {
      metadata.album = pv.pwszVal;
    }
    PropVariantClear(&pv);

    // Title
    PropVariantInit(&pv);
    if (SUCCEEDED(pStore->GetValue(PKEY_Title, &pv)) && pv.vt == VT_LPWSTR) {
      metadata.title = pv.pwszVal;
    }
    PropVariantClear(&pv);

    // Duration (in 100-nanosecond units)
    PropVariantInit(&pv);
    if (SUCCEEDED(pStore->GetValue(PKEY_Media_Duration, &pv)) &&
        pv.vt == VT_UI8) {
      metadata.durationSec =
          static_cast<uint32_t>(pv.uhVal.QuadPart / 10000000);
    }
    PropVariantClear(&pv);

    // Bitrate (in bits per second)
    PropVariantInit(&pv);
    if (SUCCEEDED(pStore->GetValue(PKEY_Audio_EncodingBitrate, &pv)) &&
        pv.vt == VT_UI4) {
      metadata.bitrate = pv.ulVal;
    }
    PropVariantClear(&pv);

    pStore->Release();
  }

  CoUninitialize();
  return !metadata.artist.empty() || !metadata.album.empty() ||
         !metadata.title.empty();
}

} // namespace Engine
} // namespace ExplorerLens
