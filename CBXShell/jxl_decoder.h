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
    static HRESULT DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap)
    {
        // Stub: libjxl not yet linked. Will be implemented in Sprint 1.
        if (phBitmap) *phBitmap = nullptr;
        return E_NOTIMPL;
    }

    /**************************************************************************
     * Check if data is valid JPEG XL format
     * 
     * @param data - File header data
     * @param size - Size of data (minimum 12 bytes required)
     * @return true if JXL signature found (naked or containerized)
     **************************************************************************/
    static bool IsJXLFormat(const BYTE* data, size_t size)
    {
        if (!data || size < 2) return false;
        // Naked codestream: starts with 0xFF 0x0A
        if (data[0] == 0xFF && data[1] == 0x0A) return true;
        // ISO BMFF container: "JXL " at offset 4
        if (size >= 12 && data[4] == 0x4A && data[5] == 0x58 &&
            data[6] == 0x4C && data[7] == 0x20) return true;
        return false;
    }

    /**************************************************************************
     * Get JPEG XL image dimensions without full decode
     * 
     * @param data - JXL file data
     * @param size - Size of data
     * @param width - Output width
     * @param height - Output height
     * @return true if dimensions extracted successfully
     **************************************************************************/
    static bool GetDimensions(const BYTE* data, size_t size, int* width, int* height)
    {
        // Stub: requires libjxl
        return false;
    }

private:
    // Convert decoded RGBA to Windows DIB format
    static HBITMAP CreateBitmapFromRGBA(const BYTE* rgba, int width, int height)
    {
        return nullptr;
    }
    
    // Helper to check naked JXL signature
    static bool IsNakedJXL(const BYTE* data, size_t size)
    {
        return (data && size >= 2 && data[0] == 0xFF && data[1] == 0x0A);
    }
    
    // Helper to check containerized JXL (ISO BMFF)
    static bool IsContainerizedJXL(const BYTE* data, size_t size)
    {
        return (data && size >= 12 && data[4] == 0x4A && data[5] == 0x58 &&
                data[6] == 0x4C && data[7] == 0x20);
    }
};

} // namespace DarkThumbs
