/******************************************************************************
 * pdf_decoder.h
 * PDF Thumbnail Extraction for ExplorerLens
 * Sprint C4: Extract first page thumbnail from PDF files using Windows.Data.Pdf
 ******************************************************************************/

#pragma once

#include <windows.h>

namespace ExplorerLens {

class PDFDecoder {
public:
    /**
     * Extract first page of PDF as HBITMAP thumbnail
     * 
     * @param data - Raw PDF file data
     * @param size - Size of data in bytes
     * @param phBitmap - Output bitmap handle (caller owns, must DeleteObject)
     * @param maxWidth - Maximum width for thumbnail (default 256)
     * @param maxHeight - Maximum height for thumbnail (default 256)
     * @return S_OK on success, E_FAIL on error
     * 
     * Note: Requires Windows 10 1803+
     */
    static HRESULT DecodeToHBITMAP(
        const BYTE* data,
        size_t size,
        HBITMAP* phBitmap,
        int maxWidth = 256,
        int maxHeight = 256
    );

    /**
     * Check if data is valid PDF format
     * 
     * @param data - File header data
     * @param size - Size of data (minimum 5 bytes required)
     * @return true if "%PDF-" signature found
     */
    static bool IsPDFFormat(const BYTE* data, size_t size);

    /**
     * Get PDF page count
     * 
     * @param data - PDF file data
     * @param size - Size of data
     * @param pageCount - Output page count
     * @return true if page count retrieved successfully
     */
    static bool GetPageCount(const BYTE* data, size_t size, int* pageCount);

    /**
     * Check if Windows PDF Platform is available
     * 
     * @return true if Windows.Data.Pdf API is available
     */
    static bool IsPDFPlatformAvailable();
};

} // namespace ExplorerLens

