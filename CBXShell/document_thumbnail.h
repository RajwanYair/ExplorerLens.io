#pragma once

#include <windows.h>
#include <string>
#include <memory>

// Sprint 9 Phase 1: Office Document Thumbnail Extraction
// Supports: DOCX, PPTX, XLSX, PDF (enhanced)
//
// Uses multiple strategies:
// 1. Windows.Storage.Thumbnails API (Windows 8+)
// 2. IShellItemImageFactory (Windows Vista+)
// 3. Embedded thumbnail extraction from Office files
// 4. First page rendering fallback

namespace DarkThumbs
{

    class DocumentThumbnail
    {
    public:
        // Extract thumbnail from Office document
        // Returns HBITMAP handle or NULL on failure
        // Caller must delete bitmap with DeleteObject()
        static HBITMAP ExtractDocumentThumbnail(
            const std::wstring &filePath,
            int width = 256,
            int height = 256);

        // Get document metadata
        struct DocumentMetadata
        {
            std::wstring title;
            std::wstring author;
            std::wstring subject;
            std::wstring keywords;
            std::wstring comments;
            std::wstring lastModifiedBy;
            int pageCount;
            int wordCount;
            FILETIME creationTime;
            FILETIME lastModifiedTime;
        };

        static bool GetMetadata(
            const std::wstring &filePath,
            DocumentMetadata &metadata);

        // Check if Windows.Storage API is available (Windows 8+)
        static bool IsStorageThumbnailAvailable();

        // Detect document type from extension
        enum class DocumentType
        {
            Unknown,
            Word,       // docx, doc
            Excel,      // xlsx, xls
            PowerPoint, // pptx, ppt
            PDF,        // pdf
            Text,       // txt, rtf
            XPS         // xps, oxps
        };

        static DocumentType GetDocumentType(const std::wstring &filePath);

    private:
        // Strategy 1: Windows.Storage.Thumbnails API (Windows 8+)
        static HBITMAP ExtractUsingStorageAPI(
            const std::wstring &filePath,
            int width,
            int height);

        // Strategy 2: IShellItemImageFactory (Windows Vista+)
        static HBITMAP ExtractUsingShellItem(
            const std::wstring &filePath,
            int width,
            int height);

        // Strategy 3: Embedded thumbnail from Office Open XML
        static HBITMAP ExtractEmbeddedThumbnail(
            const std::wstring &filePath);

        // Strategy 4: Generate generic document icon with metadata
        static HBITMAP GenerateGenericThumbnail(
            const std::wstring &filePath,
            DocumentType type,
            int width,
            int height);

        // Helper: Extract thumbnail from ZIP-based Office file (docx, pptx, xlsx)
        static HBITMAP ExtractFromOpenXML(
            const std::wstring &filePath,
            DocumentType type);

        // Helper: Read document properties using OLE Automation
        static bool ReadOLEProperties(
            const std::wstring &filePath,
            DocumentMetadata &metadata);

        // Helper: Convert IStream to HBITMAP
        static HBITMAP StreamToHBITMAP(IStream *stream);

        // Helper: Create thumbnail background with document icon
        static HBITMAP CreateDocumentBackground(
            int width,
            int height,
            DocumentType type);

        // Helper: Draw metadata overlay on thumbnail
        static void DrawMetadataOverlay(
            HDC hdc,
            const RECT &rect,
            const DocumentMetadata &metadata,
            DocumentType type);
    };

} // namespace DarkThumbs
