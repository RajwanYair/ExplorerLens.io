/******************************************************************************
 * heif_decoder_native.h
 * Native Windows HEIF/HEIC Decoder using Windows Imaging Component (WIC)
 * For DarkThumbs v5.0 - Sprint 2 Alternative Implementation
 ******************************************************************************/

#pragma once

#include <windows.h>

namespace DarkThumbs {

class HEIFDecoderNative {
public:
    // Check if data is HEIF/HEIC format
    static bool IsHEIFFormat(const BYTE* data, size_t size);
    
    // Decode HEIF/HEIC to HBITMAP
    // Uses Windows 10/11 built-in HEIF codec via WIC
    static HRESULT DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap, bool isDarkMode = false);
    
private:
    // Apply dark mode background for transparent pixels
    static void ApplyDarkModeBackground(BYTE* pPixels, UINT width, UINT height);
};

} // namespace DarkThumbs
