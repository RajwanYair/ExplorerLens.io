// PropertyStoreHandler.h — IPropertyStore COM Handler for File Metadata
// ExplorerLens Engine v15.0.0 "Zenith" — Sprints 358-359
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
    ImageWidth      = 0x1001,
    ImageHeight     = 0x1002,
    ColorDepth      = 0x1003,
    CodecName       = 0x1004,
    FormatVersion   = 0x1005,
    HasAlpha        = 0x1006,
    IsAnimated      = 0x1007,
    FrameCount      = 0x1008,
    DPI             = 0x1009,
    ColorSpace      = 0x100A,
    CompressionType = 0x100B,
    FileFormatName  = 0x100C,
    ThumbnailSize   = 0x100D,
    GPUAccelerated  = 0x100E,
    DecoderName     = 0x100F,
    DecodeTimeMs    = 0x1010,
};

/// Property value type
enum class PropertyType : uint8_t {
    UInt32   = 0,
    UInt64   = 1,
    String   = 2,
    Bool     = 3,
    Float    = 4,
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
    None        = 0,
    Read        = 1 << 0,   ///< Can read properties
    Write       = 1 << 1,   ///< Can write properties (metadata editing)
    Enumerate   = 1 << 2,   ///< Can enumerate all properties
    Initialize  = 1 << 3,   ///< Supports IInitializeWithStream
    All         = 0x0F
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
        if (!filePath) return props;

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
        if (!extension) return false;
        std::wstring ext(extension);
        for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
        return m_formatHandlers.count(ext) > 0;
    }

    /// Get property display name
    static const wchar_t* GetPropertyDisplayName(PropertyID id) {
        switch (id) {
            case PropertyID::ImageWidth:      return L"Width";
            case PropertyID::ImageHeight:     return L"Height";
            case PropertyID::ColorDepth:      return L"Color Depth";
            case PropertyID::CodecName:       return L"Codec";
            case PropertyID::FormatVersion:   return L"Format Version";
            case PropertyID::HasAlpha:        return L"Has Alpha";
            case PropertyID::IsAnimated:      return L"Animated";
            case PropertyID::FrameCount:      return L"Frame Count";
            case PropertyID::DPI:             return L"DPI";
            case PropertyID::ColorSpace:      return L"Color Space";
            case PropertyID::CompressionType: return L"Compression";
            case PropertyID::FileFormatName:  return L"Format";
            case PropertyID::ThumbnailSize:   return L"Thumbnail Size";
            case PropertyID::GPUAccelerated:  return L"GPU Accelerated";
            case PropertyID::DecoderName:     return L"Decoder";
            case PropertyID::DecodeTimeMs:    return L"Decode Time (ms)";
            default:                          return L"Unknown";
        }
    }

    /// Get the COM CLSID for property handler registration
    /// Uses a separate CLSID from the thumbnail provider
    static const GUID& GetPropertyHandlerCLSID() {
        // {B2E7F4A1-3C8D-4E9F-A5B6-1D2E3F4A5B6C}
        static const GUID clsid = {
            0xB2E7F4A1, 0x3C8D, 0x4E9F,
            { 0xA5, 0xB6, 0x1D, 0x2E, 0x3F, 0x4A, 0x5B, 0x6C }
        };
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
            case PropertyID::ImageWidth:      return "ImageWidth";
            case PropertyID::ImageHeight:     return "ImageHeight";
            case PropertyID::ColorDepth:      return "ColorDepth";
            case PropertyID::CodecName:       return "CodecName";
            case PropertyID::FormatVersion:   return "FormatVersion";
            case PropertyID::HasAlpha:        return "HasAlpha";
            case PropertyID::IsAnimated:      return "IsAnimated";
            case PropertyID::FrameCount:      return "FrameCount";
            case PropertyID::DPI:             return "DPI";
            case PropertyID::ColorSpace:      return "ColorSpace";
            case PropertyID::CompressionType: return "CompressionType";
            case PropertyID::FileFormatName:  return "FileFormatName";
            case PropertyID::ThumbnailSize:   return "ThumbnailSize";
            case PropertyID::GPUAccelerated:  return "GPUAccelerated";
            case PropertyID::DecoderName:     return "DecoderName";
            case PropertyID::DecodeTimeMs:    return "DecodeTimeMs";
            default:                          return "Unknown";
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

    PropertyStoreHandler() {
        RegisterDefaultFormats();
    }

    void RegisterDefaultFormats() {
        // Image formats
        RegisterFormat(L".webp",  L"WebP Image",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".avif",  L"AVIF Image",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".jxl",   L"JPEG XL",     PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".heif",  L"HEIF Image",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".heic",  L"HEIC Image",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".psd",   L"Photoshop",   PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".exr",   L"OpenEXR",     PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".hdr",   L"Radiance HDR",PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".dds",   L"DirectDraw",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".svg",   L"SVG Vector",  PropertyCapability::Read, ExtractImageProperties);
        RegisterFormat(L".raw",   L"Camera RAW",  PropertyCapability::Read, ExtractImageProperties);
        // 3D formats
        RegisterFormat(L".glb",   L"glTF Binary", PropertyCapability::Read, Extract3DProperties);
        RegisterFormat(L".gltf",  L"glTF",        PropertyCapability::Read, Extract3DProperties);
        RegisterFormat(L".stl",   L"STL Model",   PropertyCapability::Read, Extract3DProperties);
    }

    void RegisterFormat(const wchar_t* ext, const wchar_t* desc,
                        PropertyCapability caps, ExtractFn fn) {
        m_formatHandlers[ext] = { fn, desc, caps };
    }

    static std::vector<PropertyValue> ExtractImageProperties(const wchar_t* filePath) {
        (void)filePath;
        std::vector<PropertyValue> props;
        // Stub — real implementation reads headers via Engine decoders
        PropertyValue fmt;
        fmt.id = PropertyID::FileFormatName;
        fmt.type = PropertyType::String;
        fmt.stringValue = L"Image";
        props.push_back(fmt);
        return props;
    }

    static std::vector<PropertyValue> Extract3DProperties(const wchar_t* filePath) {
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
        if (dot == std::wstring::npos) return L"";
        std::wstring ext = path.substr(dot);
        for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
        return ext;
    }

    std::unordered_map<std::wstring, FormatHandler> m_formatHandlers;
};

inline PropertyCapability operator|(PropertyCapability a, PropertyCapability b) {
    return static_cast<PropertyCapability>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline PropertyCapability operator&(PropertyCapability a, PropertyCapability b) {
    return static_cast<PropertyCapability>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

} // namespace Engine
} // namespace ExplorerLens
