//==============================================================================
// ExplorerLens Engine — DPX/Cineon Decoder
// Film/TV post-production formats. DPX (SMPTE 268M) and Cineon film scan.
//==============================================================================
#pragma once
#include "../Core/IThumbnailDecoder.h"
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// DPX file header (simplified — SMPTE 268M-2003)
struct DPXHeader {
  uint32_t magic = 0; // 0x53445058 ('SDPX') or 0x58504453 ('XPDS')
  uint32_t imageOffset = 0;
  char version[8] = {};
  uint32_t fileSize = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t bitDepth = 10; // typically 10-bit log
  uint8_t channels = 3;  // RGB
  bool bigEndian = true;
  bool valid = false;
};

/// Cineon file header (Kodak)
struct CineonHeader {
  uint32_t magic = 0; // 0x802A5FD7
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t bitDepth = 10;
  bool valid = false;
};

/// Transfer characteristic for DPX
enum class DPXTransfer : uint8_t {
  Linear,
  LogFilm, ///< Kodak Cineon logarithmic film
  LogarithmicPrintingDensity,
  ITU_R_709,
  SMPTE_ST_2084, // PQ/HDR
  Unknown
};

/// DPX/Cineon decoder
class DPXDecoder : public IThumbnailDecoder {
public:
  DPXDecoder() {
    m_name = L"DPXDecoder";
    m_extensions = {L".dpx", L".cin"};
  }

  const wchar_t *GetName() const override { return m_name.c_str(); }

  bool CanDecode(const wchar_t *filePath) override {
    if (!filePath)
      return false;
    const wchar_t *ext = wcsrchr(filePath, L'.');
    if (!ext)
      return false;
    for (auto &e : m_extensions)
      if (_wcsicmp(e.c_str(), ext) == 0)
        return true;
    return false;
  }

  DecoderInfo GetInfo() const override {
    static const wchar_t *exts[] = {L".dpx", L".cin", nullptr};
    DecoderInfo info;
    info.name = L"DPXDecoder";
    info.version = L"1.0";
    info.supportedExtensions = exts;
    info.extensionCount = 2;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
  }

  HRESULT Decode(const ThumbnailRequest &request,
                 ThumbnailResult &result) override {
    if (!request.filePath) {
      result.status = E_INVALIDARG;
      return E_INVALIDARG;
    }

    // Read file into memory
    std::ifstream file(request.filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      result.status = E_FAIL;
      return E_FAIL;
    }

    auto fileSize = static_cast<size_t>(file.tellg());
    if (fileSize < 2048) {
      result.status = E_FAIL;
      return E_FAIL;
    } // DPX header is 2048 bytes
    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char *>(data.data()),
              static_cast<std::streamsize>(fileSize));
    file.close();

    // Parse DPX header
    DPXHeader hdr = ParseDPXHeader(data.data(), fileSize);
    if (!hdr.valid || hdr.width == 0 || hdr.height == 0) {
      result.status = E_FAIL;
      return E_FAIL;
    }

    // Calculate thumbnail dimensions preserving aspect ratio
    uint32_t thumbW = request.width, thumbH = request.height;
    float scaleX = static_cast<float>(thumbW) / hdr.width;
    float scaleY = static_cast<float>(thumbH) / hdr.height;
    float scale = (std::min)(scaleX, scaleY);
    thumbW = static_cast<uint32_t>(hdr.width * scale);
    thumbH = static_cast<uint32_t>(hdr.height * scale);
    if (thumbW == 0)
      thumbW = 1;
    if (thumbH == 0)
      thumbH = 1;

    // Decode pixel data: 10-bit packed RGB to 8-bit BGRA thumbnail
    // Use nearest-neighbor downsampling for speed
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(thumbW);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(thumbH); // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bits = nullptr;
    HBITMAP hbm =
        CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hbm || !bits) {
      result.status = E_OUTOFMEMORY;
      return E_OUTOFMEMORY;
    }

    uint8_t *dst = static_cast<uint8_t *>(bits);
    const uint8_t *pixels = data.data() + hdr.imageOffset;
    size_t pixelDataSize = fileSize - hdr.imageOffset;

    for (uint32_t y = 0; y < thumbH; ++y) {
      uint32_t srcY = static_cast<uint32_t>(y / scale);
      if (srcY >= hdr.height)
        srcY = hdr.height - 1;
      for (uint32_t x = 0; x < thumbW; ++x) {
        uint32_t srcX = static_cast<uint32_t>(x / scale);
        if (srcX >= hdr.width)
          srcX = hdr.width - 1;

        uint8_t r = 0, g = 0, b = 0;
        if (hdr.bitDepth == 10) {
          // 10-bit packed: 3 pixels per 32-bit word (method A)
          size_t pixelIndex = static_cast<size_t>(srcY) * hdr.width + srcX;
          size_t wordOffset =
              (pixelIndex / 1) * 4; // 1 pixel per 32-bit word for DPX
          if (wordOffset + 4 <= pixelDataSize) {
            uint32_t word;
            if (hdr.bigEndian) {
              word = (static_cast<uint32_t>(pixels[wordOffset]) << 24) |
                     (static_cast<uint32_t>(pixels[wordOffset + 1]) << 16) |
                     (static_cast<uint32_t>(pixels[wordOffset + 2]) << 8) |
                     static_cast<uint32_t>(pixels[wordOffset + 3]);
            } else {
              word = (static_cast<uint32_t>(pixels[wordOffset + 3]) << 24) |
                     (static_cast<uint32_t>(pixels[wordOffset + 2]) << 16) |
                     (static_cast<uint32_t>(pixels[wordOffset + 1]) << 8) |
                     static_cast<uint32_t>(pixels[wordOffset]);
            }
            // DPX packing: R[31:22] G[21:12] B[11:2]
            r = LogToLinear(static_cast<uint16_t>((word >> 22) & 0x3FF));
            g = LogToLinear(static_cast<uint16_t>((word >> 12) & 0x3FF));
            b = LogToLinear(static_cast<uint16_t>((word >> 2) & 0x3FF));
          }
        } else {
          // 8-bit or 16-bit: simple RGB
          size_t bytesPerPixel =
              static_cast<size_t>(hdr.channels) * (hdr.bitDepth / 8);
          size_t offset =
              (static_cast<size_t>(srcY) * hdr.width + srcX) * bytesPerPixel;
          if (offset + hdr.channels <= pixelDataSize) {
            r = (hdr.bitDepth == 16) ? pixels[offset + 1] : pixels[offset];
            g = (hdr.bitDepth == 16) ? pixels[offset + 3] : pixels[offset + 1];
            b = (hdr.bitDepth == 16) ? pixels[offset + 5] : pixels[offset + 2];
          }
        }

        size_t dstPixel = (static_cast<size_t>(y) * thumbW + x) * 4;
        dst[dstPixel + 0] = b; // BGRA
        dst[dstPixel + 1] = g;
        dst[dstPixel + 2] = r;
        dst[dstPixel + 3] = 255;
      }
    }

    result.hBitmap = hbm;
    result.width = thumbW;
    result.height = thumbH;
    result.status = S_OK;
    return S_OK;
  }

  const wchar_t **GetSupportedExtensions() const override {
    static const wchar_t *exts[] = {L".dpx", L".cin", nullptr};
    return exts;
  }

  uint32_t GetExtensionCount() const override { return 2; }

  bool SupportsGPU() const override { return false; }

  bool IsArchiveDecoder() const override { return false; }

  /// Check DPX magic bytes
  static bool IsDPXFile(const uint8_t *data, size_t size) {
    if (size < 4)
      return false;
    uint32_t magic =
        (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return (magic == 0x53445058 || magic == 0x58504453);
  }

  /// Check Cineon magic bytes
  static bool IsCineonFile(const uint8_t *data, size_t size) {
    if (size < 4)
      return false;
    uint32_t magic =
        (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return (magic == 0x802A5FD7);
  }

  /// DPX transfer name
  static const wchar_t *TransferName(DPXTransfer t) {
    switch (t) {
    case DPXTransfer::Linear:
      return L"Linear";
    case DPXTransfer::LogFilm:
      return L"Log Film";
    case DPXTransfer::LogarithmicPrintingDensity:
      return L"Log Print Density";
    case DPXTransfer::ITU_R_709:
      return L"ITU-R BT.709";
    case DPXTransfer::SMPTE_ST_2084:
      return L"SMPTE ST2084 (PQ)";
    default:
      return L"Unknown";
    }
  }

  static constexpr uint32_t TransferTypeCount() { return 6; }

  /// Parse DPX file header from raw bytes
  static DPXHeader ParseDPXHeader(const uint8_t *data, size_t size) {
    DPXHeader hdr;
    if (size < 2048)
      return hdr;

    uint32_t magic =
        (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    if (magic == 0x53445058) {
      // 'SDPX' — big-endian
      hdr.bigEndian = true;
      hdr.magic = magic;
    } else if (magic == 0x58504453) {
      // 'XPDS' — little-endian
      hdr.bigEndian = false;
      hdr.magic = magic;
    } else {
      return hdr; // Not a DPX file
    }

    auto readU32 = [&](size_t offset) -> uint32_t {
      if (hdr.bigEndian)
        return (static_cast<uint32_t>(data[offset]) << 24) |
               (static_cast<uint32_t>(data[offset + 1]) << 16) |
               (static_cast<uint32_t>(data[offset + 2]) << 8) |
               static_cast<uint32_t>(data[offset + 3]);
      else
        return static_cast<uint32_t>(data[offset]) |
               (static_cast<uint32_t>(data[offset + 1]) << 8) |
               (static_cast<uint32_t>(data[offset + 2]) << 16) |
               (static_cast<uint32_t>(data[offset + 3]) << 24);
    };

    hdr.imageOffset = readU32(4); // Offset to image data
    for (int i = 0; i < 8; ++i)
      hdr.version[i] = static_cast<char>(data[8 + i]);
    hdr.fileSize = readU32(16); // Total file size

    // Image information header starts at offset 768
    // Pixel per line (width) at offset 772
    // Lines per element (height) at offset 776
    hdr.width = readU32(772);
    hdr.height = readU32(776);

    // Image element descriptor at offset 780
    // Bit depth at offset 803 (bits per component)
    if (size > 803)
      hdr.bitDepth = data[803];
    hdr.channels = 3; // RGB default

    hdr.valid = (hdr.width > 0 && hdr.height > 0 && hdr.width <= 32768 &&
                 hdr.height <= 32768 && hdr.imageOffset < size);
    return hdr;
  }

  /// Convert 10-bit log to 8-bit linear (simplified: 0→0, 1023→255)
  static uint8_t LogToLinear(uint16_t logValue) {
    return static_cast<uint8_t>((static_cast<uint32_t>(logValue) * 255u) /
                                1023u);
  }

private:
  std::wstring m_name;
  std::vector<std::wstring> m_extensions;
};

/// Alias for test compatibility
using DPXCineonDecoder = DPXDecoder;

} // namespace Engine
} // namespace ExplorerLens