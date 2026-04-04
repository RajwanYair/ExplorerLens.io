// FreeTypeIntegration.h — FreeType 2.x Font Rendering Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides high‐quality font preview rendering using FreeType + HarfBuzz.
// Replaces GDI-based font preview for richer glyph rendering with OpenType
// feature support (ligatures, kerning, variable fonts).

#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Font rendering backend
enum class FontBackend : uint8_t {
    GDI = 0,         ///< Windows GDI (legacy fallback)
    FreeType = 1,    ///< FreeType 2.x (preferred)
    DirectWrite = 2  ///< DirectWrite (Windows 7+)
};

/// Font format detected from file
enum class FontFormat : uint8_t {
    Unknown = 0,
    TrueType = 1,   ///< .ttf
    OpenType = 2,   ///< .otf
    WOFF = 3,       ///< .woff
    WOFF2 = 4,      ///< .woff2
    Type1 = 5,      ///< .pfb/.pfm
    BDF = 6,        ///< Bitmap Distribution Format
    PCF = 7,        ///< Portable Compiled Format
    Collection = 8  ///< .ttc/.otc (font collection)
};

/// Font style flags
enum class FontStyle : uint32_t {
    Regular = 0,
    Bold = 1 << 0,
    Italic = 1 << 1,
    BoldItalic = Bold | Italic,
    Monospace = 1 << 2,
    Serif = 1 << 3,
    SansSerif = 1 << 4,
    Variable = 1 << 5,  ///< OpenType variable font
    ColorFont = 1 << 6  ///< COLR/CPAL color font
};

/// Font render mode (anti-aliasing strategy)
enum class FontRenderMode : uint8_t {
    Normal = 0,      ///< Standard grayscale anti-aliasing
    Subpixel = 1,    ///< LCD subpixel rendering (ClearType-like)
    Monochrome = 2,  ///< No anti-aliasing (bitmap)
    Light = 3,       ///< Light auto-hinting
    COUNT
};

/// Font metadata extracted from file
struct FontInfo
{
    std::wstring familyName;
    std::wstring styleName;
    std::wstring fullName;
    std::wstring designer;
    std::wstring description;
    FontFormat format = FontFormat::Unknown;
    FontStyle style = FontStyle::Regular;
    uint32_t numGlyphs = 0;
    uint32_t unitsPerEm = 0;
    uint32_t numFaces = 1;  ///< For collections (.ttc)
    bool hasKerning = false;
    bool hasLigatures = false;
    bool isVariable = false;
    uint32_t numVariationAxes = 0;
};

/// Font render result
struct FontRenderResult
{
    bool success = false;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> pixelData;  ///< BGRA8
    FontInfo info;
    std::string errorMessage;
};

/// Font rendering configuration
struct FontRenderConfig
{
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    float fontSize = 24.0f;
    uint32_t backgroundColor = 0xFFFFFFFF;  ///< BGRA white
    uint32_t textColor = 0xFF000000;        ///< BGRA black
    std::wstring sampleText = L"The quick brown fox jumps over the lazy dog\n"
                              L"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
                              L"abcdefghijklmnopqrstuvwxyz\n"
                              L"0123456789 !@#$%^&*()";
    bool showFontName = true;
    bool showGlyphGrid = false;
    bool enableAntiAlias = true;
    bool enableHinting = true;
    FontBackend backend = FontBackend::FreeType;
};

/// FreeType integration for font thumbnail generation
class FreeTypeIntegration
{
  public:
    static FreeTypeIntegration& Instance()
    {
        static FreeTypeIntegration instance;
        return instance;
    }

    /// Check if FreeType is available
    bool IsAvailable() const
    {
#ifdef HAS_FREETYPE
        return true;
#else
        return false;
#endif
    }

    /// Get library version string
    const char* GetVersion() const
    {
#ifdef HAS_FREETYPE
        return "2.13.3";
#else
        return "not available";
#endif
    }

    /// Get active rendering backend
    FontBackend GetActiveBackend() const
    {
#ifdef HAS_FREETYPE
        return FontBackend::FreeType;
#else
        return FontBackend::GDI;
#endif
    }

    /// Extract font metadata
    FontInfo GetFontInfo(const wchar_t* filePath)
    {
        FontInfo info;
        if (!filePath)
            return info;
#ifdef HAS_FREETYPE
        info = ExtractFontInfoFreeType(filePath);
#else
        info = ExtractFontInfoGDI(filePath);
#endif
        return info;
    }

    /// Render font preview to pixel buffer
    FontRenderResult RenderPreview(const wchar_t* filePath, const FontRenderConfig& config = {})
    {
        FontRenderResult result;
#ifdef HAS_FREETYPE
        result = RenderWithFreeType(filePath, config);
#else
        result = RenderWithGDI(filePath, config);
#endif
        return result;
    }

    /// Generate HBITMAP thumbnail
    HBITMAP GenerateThumbnail(const wchar_t* filePath, uint32_t cx, uint32_t cy)
    {
        FontRenderConfig config;
        config.targetWidth = cx;
        config.targetHeight = cy;
        auto result = RenderPreview(filePath, config);
        if (!result.success || result.pixelData.empty())
            return nullptr;
        return CreateBitmapFromPixels(result.pixelData.data(), result.width, result.height);
    }

    /// Detect font format from file header
    static FontFormat DetectFormat(const uint8_t* header, size_t len)
    {
        if (len < 4)
            return FontFormat::Unknown;
        // TrueType: 00 01 00 00
        if (header[0] == 0x00 && header[1] == 0x01 && header[2] == 0x00 && header[3] == 0x00)
            return FontFormat::TrueType;
        // OpenType: OTTO
        if (header[0] == 'O' && header[1] == 'T' && header[2] == 'T' && header[3] == 'O')
            return FontFormat::OpenType;
        // TTC collection: ttcf
        if (header[0] == 't' && header[1] == 't' && header[2] == 'c' && header[3] == 'f')
            return FontFormat::Collection;
        // WOFF: wOFF
        if (header[0] == 'w' && header[1] == 'O' && header[2] == 'F' && header[3] == 'F')
            return FontFormat::WOFF;
        // WOFF2: wOF2
        if (header[0] == 'w' && header[1] == 'O' && header[2] == 'F' && header[3] == '2')
            return FontFormat::WOFF2;
        return FontFormat::Unknown;
    }

    /// Format name lookup
    static const char* FormatName(FontFormat f)
    {
        switch (f) {
            case FontFormat::Unknown:
                return "Unknown";
            case FontFormat::TrueType:
                return "TrueType";
            case FontFormat::OpenType:
                return "OpenType";
            case FontFormat::WOFF:
                return "WOFF";
            case FontFormat::WOFF2:
                return "WOFF2";
            case FontFormat::Type1:
                return "Type1";
            case FontFormat::BDF:
                return "BDF";
            case FontFormat::PCF:
                return "PCF";
            case FontFormat::Collection:
                return "Collection";
            default:
                return "?";
        }
    }

    /// Backend name lookup
    static const char* BackendName(FontBackend b)
    {
        switch (b) {
            case FontBackend::GDI:
                return "GDI";
            case FontBackend::FreeType:
                return "FreeType";
            case FontBackend::DirectWrite:
                return "DirectWrite";
            default:
                return "Unknown";
        }
    }

    static constexpr uint32_t GetFormatCount()
    {
        return 9;
    }
    static constexpr uint32_t GetBackendCount()
    {
        return 3;
    }

    /// Font render mode queries
    static constexpr size_t RenderModeCount()
    {
        return static_cast<size_t>(FontRenderMode::COUNT);
    }

    static const wchar_t* RenderModeName(FontRenderMode m)
    {
        switch (m) {
            case FontRenderMode::Normal:
                return L"Normal";
            case FontRenderMode::Subpixel:
                return L"Subpixel";
            case FontRenderMode::Monochrome:
                return L"Monochrome";
            case FontRenderMode::Light:
                return L"Light";
            default:
                return L"Unknown";
        }
    }

  private:
    FreeTypeIntegration() = default;

#ifdef HAS_FREETYPE
    FontInfo ExtractFontInfoFreeType(const wchar_t* filePath);
    FontRenderResult RenderWithFreeType(const wchar_t* filePath, const FontRenderConfig& config);
#endif
    FontInfo ExtractFontInfoGDI(const wchar_t* filePath)
    {
        FontInfo info;
        (void)filePath;
        info.format = FontFormat::Unknown;
        return info;
    }
    FontRenderResult RenderWithGDI(const wchar_t* filePath, const FontRenderConfig& config)
    {
        FontRenderResult result;
        (void)filePath;
        (void)config;
        result.errorMessage = "GDI fallback — limited rendering";
        return result;
    }

    HBITMAP CreateBitmapFromPixels(const uint8_t* pixels, uint32_t w, uint32_t h)
    {
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(w);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(h);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        void* bits = nullptr;
        HBITMAP hbmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (hbmp && bits)
            memcpy(bits, pixels, static_cast<size_t>(w) * h * 4);
        return hbmp;
    }
};

inline FontStyle operator|(FontStyle a, FontStyle b)
{
    return static_cast<FontStyle>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline FontStyle operator&(FontStyle a, FontStyle b)
{
    return static_cast<FontStyle>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

}  // namespace Engine
}  // namespace ExplorerLens
