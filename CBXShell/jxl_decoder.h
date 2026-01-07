/******************************************************************************
 * jxl_decoder.h
 * JPEG XL Image Decoder for DarkThumbs
 * 
 * Add this file to CBXShell project and include from cbxArchive.h
 * Part of Sprint 3 - Modern Image Formats (JPEG XL)
 ******************************************************************************/

#pragma once

#include <windows.h>

// Forward declarations from libjxl
struct JxlDecoderStruct;
typedef struct JxlDecoderStruct JxlDecoder;

namespace DarkThumbs {

class JXLDecoder {
public:
    /**************************************************************************
     * Decode JPEG XL image data into a Windows HBITMAP
     * 
     * @param data - Raw JXL file data
     * @param size - Size of data in bytes
     * @param phBitmap - Output bitmap handle (caller owns, must DeleteObject)
     * @return S_OK on success, E_FAIL on decode error, E_INVALIDARG if null
     **************************************************************************/
    static HRESULT DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap);

    /**************************************************************************
     * Check if data is valid JPEG XL format
     * 
     * @param data - File header data
     * @param size - Size of data (minimum 12 bytes required)
     * @return true if JXL signature found (naked or containerized)
     **************************************************************************/
    static bool IsJXLFormat(const BYTE* data, size_t size);

    /**************************************************************************
     * Get JPEG XL image dimensions without full decode
     * 
     * @param data - JXL file data
     * @param size - Size of data
     * @param width - Output width
     * @param height - Output height
     * @return true if dimensions extracted successfully
     **************************************************************************/
    static bool GetDimensions(const BYTE* data, size_t size, int* width, int* height);

private:
    // Convert decoded RGBA to Windows DIB format
    static HBITMAP CreateBitmapFromRGBA(const BYTE* rgba, int width, int height);
    
    // Helper to check naked JXL signature
    static bool IsNakedJXL(const BYTE* data, size_t size);
    
    // Helper to check containerized JXL (ISO BMFF)
    static bool IsContainerizedJXL(const BYTE* data, size_t size);
};

} // namespace DarkThumbs
