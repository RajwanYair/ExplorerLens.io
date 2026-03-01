// PropertyStoreHandler.h — IPropertyStore COM Handler for File Metadata
// Copyright (c) 2026 ExplorerLens Project
//
// Exposes file metadata (dimensions, codec, color depth, format version)
// in Windows Explorer's Details pane via the IPropertyStore COM interface.
// Registers as a property handler for supported file extensions.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Property identifier for custom metadata
enum class PropertyID : uint32_t {
  ImageWidth = 0x1001,
  ImageHeight = 0x1002,
  ColorDepth = 0x1003,
  CodecName = 0x1004,
  FormatVersion = 0x1005,
  HasAlpha = 0x1006,
  IsAnimated = 0x1007,
  FrameCount = 0x1008,
  DPI = 0x1009,
  ColorSpace = 0x100A,
  CompressionType = 0x100B,
  FileFormatName = 0x100C,
  ThumbnailSize = 0x100D,
  GPUAccelerated = 0x100E,
  DecoderName = 0x100F,
  DecodeTimeMs = 0x1010,
};

/// Property value type
enum class PropertyType : uint8_t {
  UInt32 = 0,
  UInt64 = 1,
  String = 2,
  Bool = 3,
  Float = 4,
  DateTime = 5,
};

/// A single property key-value pair
struct PropertyValue {
  PropertyID id = PropertyID::ImageWidth;
  PropertyType type = PropertyType::UInt32;
  union {
    uint32_t u32;
    uint64_t u64;
    float f32;
    bool boolean;
  } numeric = {};
  std::wstring stringValue;
  bool isReadOnly = true;
};

/// Property store capability flags
enum class PropertyCapability : uint32_t {
  None = 0,
  Read = 1 << 0, ///< Can read properties
  Write = 1 << 1, ///< Can write properties (metadata editing)
  Enumerate = 1 << 2, ///< Can enumerate all properties
  Initialize = 1 << 3, ///< Supports IInitializeWithStream
  All = 0x0F
};

/// Property handler registration info
struct PropertyHandlerRegistration {
  const wchar_t* extension = nullptr;
  const wchar_t* progId = nullptr;
  const wchar_t* description = nullptr;
  PropertyCapability caps = PropertyCapability::Read;
};

/// Explorer property store handler engine
class PropertyStoreHandler {
public:
  static PropertyStoreHandler& Instance() {
    static PropertyStoreHandler instance;
    return instance;
  }

  /// Extract properties from a file
  std::vector<PropertyValue> GetProperties(const wchar_t* filePath) const {
    std::vector<PropertyValue> props;
    if (!filePath)
      return props;

    // Detect format and extract relevant properties
    auto ext = GetExtension(filePath);
    auto it = m_formatHandlers.find(ext);
    if (it != m_formatHandlers.end()) {
      props = it->second.extractFn(filePath);
    }
    return props;
  }

  /// Get supported extensions
  std::vector<std::wstring> GetSupportedExtensions() const {
    std::vector<std::wstring> exts;
    exts.reserve(m_formatHandlers.size());
    for (const auto& [ext, handler] : m_formatHandlers)
      exts.push_back(ext);
    return exts;
  }

  /// Check if an extension is supported
  bool IsSupported(const wchar_t* extension) const {
    if (!extension)
      return false;
    std::wstring ext(extension);
    for (auto& c : ext)
      c = static_cast<wchar_t>(towlower(c));
    return m_formatHandlers.count(ext) > 0;
  }

  /// Get property display name
  static const wchar_t* GetPropertyDisplayName(PropertyID id) {
    switch (id) {
    case PropertyID::ImageWidth:
      return L"Width";
    case PropertyID::ImageHeight:
      return L"Height";
    case PropertyID::ColorDepth:
      return L"Color Depth";
    case PropertyID::CodecName:
      return L"Codec";
    case PropertyID::FormatVersion:
      return L"Format Version";
    case PropertyID::HasAlpha:
      return L"Has Alpha";
    case PropertyID::IsAnimated:
      return L"Animated";
    case PropertyID::FrameCount:
      return L"Frame Count";
    case PropertyID::DPI:
      return L"DPI";
    case PropertyID::ColorSpace:
      return L"Color Space";
    case PropertyID::CompressionType:
      return L"Compression";
    case PropertyID::FileFormatName:
      return L"Format";
    case PropertyID::ThumbnailSize:
      return L"Thumbnail Size";
    case PropertyID::GPUAccelerated:
      return L"GPU Accelerated";
    case PropertyID::DecoderName:
      return L"Decoder";
    case PropertyID::DecodeTimeMs:
      return L"Decode Time (ms)";
    default:
      return L"Unknown";
    }
  }

  /// Get the COM CLSID for property handler registration
  /// Uses a separate CLSID from the thumbnail provider
  static const GUID& GetPropertyHandlerCLSID() {
    // {B2E7F4A1-3C8D-4E9F-A5B6-1D2E3F4A5B6C}
    static const GUID clsid = {
    0xB2E7F4A1,
    0x3C8D,
    0x4E9F,
    {0xA5, 0xB6, 0x1D, 0x2E, 0x3F, 0x4A, 0x5B, 0x6C} };
    return clsid;
  }

  /// Get registration entries for COM registration
  std::vector<PropertyHandlerRegistration> GetRegistrations() const {
    std::vector<PropertyHandlerRegistration> regs;
    for (const auto& [ext, handler] : m_formatHandlers) {
      PropertyHandlerRegistration reg;
      reg.extension = ext.c_str();
      reg.description = handler.description;
      reg.caps = handler.capabilities;
      regs.push_back(reg);
    }
    return regs;
  }

  /// Property ID name lookup
  static const char* PropertyIDName(PropertyID id) {
    switch (id) {
    case PropertyID::ImageWidth:
      return "ImageWidth";
    case PropertyID::ImageHeight:
      return "ImageHeight";
    case PropertyID::ColorDepth:
      return "ColorDepth";
    case PropertyID::CodecName:
      return "CodecName";
    case PropertyID::FormatVersion:
      return "FormatVersion";
    case PropertyID::HasAlpha:
      return "HasAlpha";
    case PropertyID::IsAnimated:
      return "IsAnimated";
    case PropertyID::FrameCount:
      return "FrameCount";
    case PropertyID::DPI:
      return "DPI";
    case PropertyID::ColorSpace:
      return "ColorSpace";
    case PropertyID::CompressionType:
      return "CompressionType";
    case PropertyID::FileFormatName:
      return "FileFormatName";
    case PropertyID::ThumbnailSize:
      return "ThumbnailSize";
    case PropertyID::GPUAccelerated:
      return "GPUAccelerated";
    case PropertyID::DecoderName:
      return "DecoderName";
    case PropertyID::DecodeTimeMs:
      return "DecodeTimeMs";
    default:
      return "Unknown";
    }
  }

  /// Get total number of property IDs defined
  static constexpr uint32_t GetPropertyIDCount() { return 16; }

private:
  using ExtractFn = std::vector<PropertyValue>(*)(const wchar_t*);

  struct FormatHandler {
    ExtractFn extractFn = nullptr;
    const wchar_t* description = nullptr;
    PropertyCapability capabilities = PropertyCapability::Read;
  };

  PropertyStoreHandler() { RegisterDefaultFormats(); }

  void RegisterDefaultFormats() {
    // Image formats
    RegisterFormat(L".webp", L"WebP Image", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".avif", L"AVIF Image", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".jxl", L"JPEG XL", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".heif", L"HEIF Image", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".heic", L"HEIC Image", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".psd", L"Photoshop", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".exr", L"OpenEXR", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".hdr", L"Radiance HDR", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".dds", L"DirectDraw", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".svg", L"SVG Vector", PropertyCapability::Read,
      ExtractImageProperties);
    RegisterFormat(L".raw", L"Camera RAW", PropertyCapability::Read,
      ExtractImageProperties);
    // 3D formats
    RegisterFormat(L".glb", L"glTF Binary", PropertyCapability::Read,
      Extract3DProperties);
    RegisterFormat(L".gltf", L"glTF", PropertyCapability::Read,
      Extract3DProperties);
    RegisterFormat(L".stl", L"STL Model", PropertyCapability::Read,
      Extract3DProperties);
  }

  void RegisterFormat(const wchar_t* ext, const wchar_t* desc,
    PropertyCapability caps, ExtractFn fn) {
    m_formatHandlers[ext] = { fn, desc, caps };
  }

  /// Extract image properties by parsing format-specific file headers.
  /// Opens the file, reads the first 64 KB, then detects the format via
  /// magic-number signatures and parses dimensions from the header.
  ///
  /// Supported header parsing:
  ///   JPEG  — SOF0/SOF2 markers -> width, height, color depth
  ///   PNG   — IHDR chunk -> width, height, bit depth, color type
  ///   BMP   — BITMAPINFOHEADER -> width, height, bit count
  ///   TIFF  — IFD tags 0x0100 (width), 0x0101 (height), 0x0102 (bps)
  ///   ZIP/RAR/7z — generic archive format label
  ///   Other — fallback "Image" label
  ///
  /// Property mapping:
  ///   System.Image.HorizontalSize -> PropertyID::ImageWidth
  ///   System.Image.VerticalSize   -> PropertyID::ImageHeight
  static std::vector<PropertyValue>
    ExtractImageProperties(const wchar_t* filePath) {
    std::vector<PropertyValue> props;
    if (!filePath) return props;

    // Open the file and read the first 64 KB header block
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
      nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return props;

    constexpr DWORD HEADER_SIZE = 65536;
    uint8_t header[HEADER_SIZE];
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, header, HEADER_SIZE, &bytesRead, nullptr);
    CloseHandle(hFile);
    if (!ok || bytesRead < 8) return props;

    uint32_t width = 0, height = 0, colorDepth = 0;
    std::wstring formatName;

    // ---- JPEG: starts with FF D8 ----
    if (bytesRead >= 2 && header[0] == 0xFF && header[1] == 0xD8) {
      formatName = L"JPEG";
      // Scan for SOF0 (0xFFC0) or SOF2 (0xFFC2) marker
      for (DWORD off = 2; off + 1 < bytesRead; ) {
        if (header[off] != 0xFF) { ++off; continue; }
        uint8_t marker = header[off + 1];
        if (marker == 0xFF) { ++off; continue; }
        if (marker == 0x00 || marker == 0x01 ||
          (marker >= 0xD0 && marker <= 0xD9)) {
          off += 2; continue;
        }
        if (off + 4 > bytesRead) break;
        uint16_t segLen = (static_cast<uint16_t>(header[off + 2]) << 8)
          | header[off + 3];
        if (marker == 0xC0 || marker == 0xC2) {
          if (off + 10 <= bytesRead) {
            height = (static_cast<uint32_t>(header[off + 5]) << 8) | header[off + 6];
            width = (static_cast<uint32_t>(header[off + 7]) << 8) | header[off + 8];
            colorDepth = static_cast<uint32_t>(header[off + 4]) * header[off + 9];
          }
          break;
        }
        off += 2 + segLen;
      }
    }
    // ---- PNG: 89 50 4E 47 0D 0A 1A 0A ----
    else if (bytesRead >= 26 &&
      header[0] == 0x89 && header[1] == 0x50 &&
      header[2] == 0x4E && header[3] == 0x47) {
      formatName = L"PNG";
      // IHDR at byte 8: [len 4][type 4][W 4][H 4][depth 1][color 1]
      width = (static_cast<uint32_t>(header[16]) << 24) |
        (static_cast<uint32_t>(header[17]) << 16) |
        (static_cast<uint32_t>(header[18]) << 8) | header[19];
      height = (static_cast<uint32_t>(header[20]) << 24) |
        (static_cast<uint32_t>(header[21]) << 16) |
        (static_cast<uint32_t>(header[22]) << 8) | header[23];
      uint8_t bitDepth = header[24];
      uint8_t colorType = header[25];
      int channels = 1;
      if (colorType == 2) channels = 3;
      else if (colorType == 4) channels = 2;
      else if (colorType == 6) channels = 4;
      colorDepth = bitDepth * static_cast<uint32_t>(channels);
    }
    // ---- BMP: 42 4D ("BM") ----
    else if (bytesRead >= 54 && header[0] == 0x42 && header[1] == 0x4D) {
      formatName = L"BMP";
      // BITMAPINFOHEADER at offset 14: biWidth(4) biHeight(4) ... biBitCount(2)
      width = *reinterpret_cast<const uint32_t*>(&header[18]);
      int32_t h = *reinterpret_cast<const int32_t*>(&header[22]);
      height = static_cast<uint32_t>(h < 0 ? -h : h);
      colorDepth = *reinterpret_cast<const uint16_t*>(&header[28]);
    }
    // ---- TIFF: II (LE) or MM (BE) with magic 0x002A ----
    else if (bytesRead >= 8 &&
      ((header[0] == 0x49 && header[1] == 0x49 &&
        header[2] == 0x2A && header[3] == 0x00) ||
        (header[0] == 0x4D && header[1] == 0x4D &&
          header[2] == 0x00 && header[3] == 0x2A))) {
      formatName = L"TIFF";
      bool bigEndian = (header[0] == 0x4D);
      auto rd16 = [&](DWORD o) -> uint16_t {
        if (o + 1 >= bytesRead) return 0;
        return bigEndian
          ? (static_cast<uint16_t>(header[o]) << 8 | header[o + 1])
          : (header[o] | static_cast<uint16_t>(header[o + 1]) << 8);
        };
      auto rd32 = [&](DWORD o) -> uint32_t {
        if (o + 3 >= bytesRead) return 0;
        return bigEndian
          ? (static_cast<uint32_t>(header[o]) << 24 |
            static_cast<uint32_t>(header[o + 1]) << 16 |
            static_cast<uint32_t>(header[o + 2]) << 8 | header[o + 3])
          : (header[o] | static_cast<uint32_t>(header[o + 1]) << 8 |
            static_cast<uint32_t>(header[o + 2]) << 16 |
            static_cast<uint32_t>(header[o + 3]) << 24);
        };
      DWORD ifdOff = rd32(4);
      if (ifdOff < bytesRead) {
        uint16_t tagCount = rd16(ifdOff);
        DWORD pos = ifdOff + 2;
        for (uint16_t t = 0; t < tagCount && pos + 12 <= bytesRead; ++t, pos += 12) {
          uint16_t tagId = rd16(pos);
          uint16_t tagType = rd16(pos + 2);
          uint32_t val = (tagType == 3) ? rd16(pos + 8) : rd32(pos + 8);
          if (tagId == 0x0100) width = val;
          if (tagId == 0x0101) height = val;
          if (tagId == 0x0102) colorDepth = val;
        }
      }
    }
    // ---- Archive signatures: ZIP (PK), RAR (Rar!), 7z ----
    else if (bytesRead >= 4 && header[0] == 0x50 && header[1] == 0x4B) {
      formatName = L"ZIP Archive";
    }
    else if (bytesRead >= 7 && header[0] == 0x52 && header[1] == 0x61 &&
      header[2] == 0x72 && header[3] == 0x21) {
      formatName = L"RAR Archive";
    }
    else if (bytesRead >= 6 && header[0] == 0x37 && header[1] == 0x7A &&
      header[2] == 0xBC && header[3] == 0xAF) {
      formatName = L"7z Archive";
    }
    // ---- Fallback ----
    else {
      formatName = L"Image";
    }

    // Populate extracted properties
    if (!formatName.empty()) {
      PropertyValue fmtProp;
      fmtProp.id = PropertyID::FileFormatName;
      fmtProp.type = PropertyType::String;
      fmtProp.stringValue = formatName;
      props.push_back(fmtProp);
    }
    // System.Image.HorizontalSize
    if (width > 0) {
      PropertyValue wProp;
      wProp.id = PropertyID::ImageWidth;
      wProp.type = PropertyType::UInt32;
      wProp.numeric.u32 = width;
      props.push_back(wProp);
    }
    // System.Image.VerticalSize
    if (height > 0) {
      PropertyValue hProp;
      hProp.id = PropertyID::ImageHeight;
      hProp.type = PropertyType::UInt32;
      hProp.numeric.u32 = height;
      props.push_back(hProp);
    }
    if (colorDepth > 0) {
      PropertyValue cdProp;
      cdProp.id = PropertyID::ColorDepth;
      cdProp.type = PropertyType::UInt32;
      cdProp.numeric.u32 = colorDepth;
      props.push_back(cdProp);
    }
    return props;
  }

  static std::vector<PropertyValue>
    Extract3DProperties(const wchar_t* filePath) {
    (void)filePath;
    std::vector<PropertyValue> props;
    PropertyValue fmt;
    fmt.id = PropertyID::FileFormatName;
    fmt.type = PropertyType::String;
    fmt.stringValue = L"3D Model";
    props.push_back(fmt);
    return props;
  }

  static std::wstring GetExtension(const wchar_t* filePath) {
    std::wstring path(filePath);
    auto dot = path.rfind(L'.');
    if (dot == std::wstring::npos)
      return L"";
    std::wstring ext = path.substr(dot);
    for (auto& c : ext)
      c = static_cast<wchar_t>(towlower(c));
    return ext;
  }

  std::unordered_map<std::wstring, FormatHandler> m_formatHandlers;
};

inline PropertyCapability operator|(PropertyCapability a,
  PropertyCapability b) {
  return static_cast<PropertyCapability>(static_cast<uint32_t>(a) |
    static_cast<uint32_t>(b));
}
inline PropertyCapability operator&(PropertyCapability a,
  PropertyCapability b) {
  return static_cast<PropertyCapability>(static_cast<uint32_t>(a) &
    static_cast<uint32_t>(b));
}

} // namespace Engine
} // namespace ExplorerLens
