/******************************************************************************
 * webp_decoder.h
 * WebP Image Decoder for DarkThumbs
 * 
 * Add this file to CBXShell project and include from cbxArchive.h
 ******************************************************************************/

#pragma once

#include <windows.h>
#include <vector>

// Forward declarations from libwebp
struct WebPDecoderConfig;

namespace DarkThumbs {

class WebPDecoder {
public:
    /**************************************************************************
     * Decode WebP image data into a Windows HBITMAP
     * 
     * @param data - Raw WebP file data
     * @param size - Size of data in bytes
     * @param phBitmap - Output bitmap handle (caller owns, must DeleteObject)
     * @return S_OK on success, E_FAIL on decode error, E_INVALIDARG if null
     **************************************************************************/
    static HRESULT DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap);

    /**************************************************************************
     * Check if data is valid WebP format
     * 
     * @param data - File header data
     * @param size - Size of data (minimum 12 bytes required)
     * @return true if "RIFF....WEBP" signature found
     **************************************************************************/
    static bool IsWebPFormat(const BYTE* data, size_t size);

    /**************************************************************************
     * Get WebP image dimensions without full decode
     * 
     * @param data - WebP file data
     * @param size - Size of data
     * @param width - Output width
     * @param height - Output height
     * @return true if dimensions extracted successfully
     **************************************************************************/
    static bool GetDimensions(const BYTE* data, size_t size, int* width, int* height);

private:
    // Convert decoded RGBA to Windows DIB format
    static HBITMAP CreateBitmapFromRGBA(const BYTE* rgba, int width, int height);
};

} // namespace DarkThumbs


/******************************************************************************
 * Implementation (add to webp_decoder.cpp)
 ******************************************************************************/
// WebP decoder implementation is in webp_decoder.cpp
// Integration is already done in CBXShellClass.cpp

/******************************************************************************
 * Update CBXShell.vcxproj
 * 
 * Add to <AdditionalIncludeDirectories>:
 ******************************************************************************/
// ..\external\image-libs\libwebp-1.4.0\src;

/******************************************************************************
 * Add to <AdditionalLibraryDirectories>:
 ******************************************************************************/
// ..\external\image-libs\libwebp-1.4.0\build-vs\Release;

/******************************************************************************
 * Add to <AdditionalDependencies>:
 ******************************************************************************/
// libwebp.lib;

/******************************************************************************
 * Testing Checklist:
 * 
 * 1. Create test WebP file:
 *    - Use online converter: https://cloudconvert.com/jpg-to-webp
 *    - Or GIMP: File > Export As > .webp
 * 
 * 2. Create test archive:
 *    - Create folder: test-webp-archive
 *    - Add test.webp
 *    - Compress to: test-webp-archive.cbz (zip)
 * 
 * 3. Test thumbnail:
 *    - Open Windows Explorer
 *    - Navigate to test-webp-archive.cbz
 *    - Check thumbnail preview (Large Icons view)
 *    - Should show first WebP image
 * 
 * 4. Verify in CBXManager:
 *    - Run CBXManager.exe
 *    - Check format statistics
 *    - Should show WebP support enabled
 ******************************************************************************/
