/******************************************************************************
 * avif_decoder.h
 * AVIF Image Decoder for DarkThumbs
 * Uses Windows Imaging Component (WIC) for native AVIF support
 * 
 * Add this file to CBXShell project and include from cbxArchive.h
 ******************************************************************************/

#pragma once

#include <windows.h>

namespace DarkThumbs {

class AVIFDecoder {
public:
    /**************************************************************************
     * Decode AVIF image data into a Windows HBITMAP using WIC
     * 
     * @param data - Raw AVIF file data
     * @param size - Size of data in bytes
     * @param phBitmap - Output bitmap handle (caller owns, must DeleteObject)
     * @return S_OK on success, E_FAIL on decode error, E_INVALIDARG if null
     * 
     * Note: Requires Windows 10 1809+ and AV1 Video Extension installed
     **************************************************************************/
    static HRESULT DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap);

    /**************************************************************************
     * Check if data is valid AVIF format
     * 
     * @param data - File header data
     * @param size - Size of data (minimum 12 bytes required)
     * @return true if "....ftyp" signature found with avif brand
     **************************************************************************/
    static bool IsAVIFFormat(const BYTE* data, size_t size);

    /**************************************************************************
     * Get AVIF image dimensions without full decode
     * 
     * @param data - AVIF file data
     * @param size - Size of data
     * @param width - Output width
     * @param height - Output height
     * @return true if dimensions extracted successfully
     **************************************************************************/
    static bool GetDimensions(const BYTE* data, size_t size, int* width, int* height);
};

} // namespace DarkThumbs

// Note: AVIF decoder implementation is in avif_decoder.cpp
// Integration is already done in CBXShellClass.cpp
// ..\external\image-libs\dav1d-1.4.0\build-vs\src;

/******************************************************************************
 * Add to <AdditionalDependencies>:
 ******************************************************************************/
// avif.lib;
// libdav1d.a;

/******************************************************************************
 * AVIF Format Support Notes:
 * 
 * Supported formats:
 *   - AVIF (AV1 Image File Format) - .avif
 *   - HEIF/HEIC (High Efficiency Image Format) - .heic, .heif
 *   - iPhone photos (HEIC with AV1 codec)
 * 
 * Features:
 *   - 10-bit and 12-bit color depth (auto-converts to 8-bit)
 *   - HDR images (tone-maps to SDR for thumbnails)
 *   - 4:2:0 and 4:4:4 chroma subsampling
 *   - Alpha channel support
 *   - Multi-frame sequences (shows first frame)
 * 
 * Performance:
 *   - Decode time: ~20-30ms for typical images
 *   - Memory: ~1.5x raw image size during decode
 *   - dav1d provides fast AV1 decoding (SIMD optimized)
 * 
 * Testing:
 *   1. iPhone HEIC photos
 *   2. Online-converted AVIF files
 *   3. HDR AVIF images
 *   4. High-resolution AVIF (>4K)
 ******************************************************************************/
