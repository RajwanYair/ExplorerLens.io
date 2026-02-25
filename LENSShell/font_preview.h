#pragma once

#include <windows.h>
#include <string>
#include <d2d1.h>
#include <dwrite.h>

// font_preview.h — Font File Preview Rendering
// Supports: TTF, OTF, WOFF, WOFF2
//
// Uses DirectWrite for high-quality font rendering
// Shows sample glyphs and font metadata

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace ExplorerLens
{

    class FontPreview
    {
    public:
        // Generate font preview thumbnail
        // Returns HBITMAP handle or NULL on failure
        // Caller must delete bitmap with DeleteObject()
        static HBITMAP GenerateFontPreview(
            const std::wstring &fontPath,
            int width = 256,
            int height = 256);

        // Get font metadata
        struct FontMetadata
        {
            std::wstring familyName;
            std::wstring styleName;
            std::wstring fullName;
            std::wstring designer;
            std::wstring version;
            std::wstring copyright;
            int weight; // 100-900 (400=normal, 700=bold)
            bool isItalic;
            bool isMonospace;
            int glyphCount;
        };

        static bool GetFontMetadata(
            const std::wstring &fontPath,
            FontMetadata &metadata);

        // Check if DirectWrite is available
        static bool IsDirectWriteAvailable();

    private:
        // DirectWrite rendering
        static HBITMAP RenderUsingDirectWrite(
            const std::wstring &fontPath,
            int width,
            int height);

        // GDI+ fallback rendering
        static HBITMAP RenderUsingGDI(
            const std::wstring &fontPath,
            int width,
            int height);

        // Load font file into DirectWrite
        static IDWriteFontFace *LoadFontFile(
            IDWriteFactory *factory,
            const std::wstring &fontPath);

        // Render sample text with given font
        static void RenderSampleText(
            ID2D1RenderTarget *renderTarget,
            IDWriteTextFormat *textFormat,
            const std::wstring &sampleText,
            const D2D1_RECT_F &rect,
            ID2D1SolidColorBrush *brush);

        // Draw glyph grid (character map preview)
        static void DrawGlyphGrid(
            ID2D1RenderTarget *renderTarget,
            IDWriteTextFormat *textFormat,
            const D2D1_RECT_F &rect,
            ID2D1SolidColorBrush *brush);

        // Draw font metadata overlay
        static void DrawMetadataOverlay(
            ID2D1RenderTarget *renderTarget,
            const FontMetadata &metadata,
            const D2D1_RECT_F &rect,
            IDWriteTextFormat *metadataFormat,
            ID2D1SolidColorBrush *brush);

        // Convert D2D bitmap to HBITMAP
        static HBITMAP ConvertToHBITMAP(ID2D1Bitmap *d2dBitmap);

        // Sample text for different font types
        static const wchar_t *GetSampleText(bool isMonospace);
    };

} // namespace ExplorerLens

