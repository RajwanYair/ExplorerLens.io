#pragma once
///////////////////////////////////////////////////////////////////////////////
// LENSArchive.h — Core shell extension implementation
//
// Implements the CLENSArchive class which provides IThumbnailProvider /
// IExtractImage / IQueryInfo COM interfaces for Windows Explorer thumbnail
// generation. Supports 200+ file formats across archives, ebooks, images,
// video, audio, documents, fonts, and 3D models.
//
// Key classes:
//   CUnzip         — Lightweight ZIP archive reader wrapper (minizip)
//   CUnRar         — RAR archive reader wrapper (unrar DLL)
//   CLENSArchive   — Main COM object; routes file extensions to decoders
//                    and produces scaled HBITMAP thumbnails
///////////////////////////////////////////////////////////////////////////////

#ifndef _LENSARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_
    #define _LENSARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_

    #ifndef STRICT
        #define STRICT
    #endif

    #include <shlObj.h>
    #include <shlwapi.h>

    #include <algorithm>  // For std::min, std::max
    #pragma comment(lib, "shlwapi.lib")

    #include "DarkModeHelper.h"
    #include "ModernCppHelper.h"
    #include "thumbnail_cache.h"
    #include "thumbnail_collage.h"

// Global icon handle — defined in LENSShell.cpp, loaded in DllMain
extern HICON zipIcon;

    // ============================================================================
    // Legacy Decoders — deprecated; all formats now handled by Engine pipeline
    // (ThumbnailPipeline → DecoderRegistry). Gated behind LENSShell_LEGACY_DECODERS
    // preprocessor define, excluded from production builds.
    // ============================================================================
    #ifdef LENSShell_LEGACY_DECODERS
        #include "webp_decoder.h"
        #ifdef ENABLE_HEIF_SUPPORT
            #include "heif_decoder_native.h"
        #endif
        #ifdef ENABLE_AVIF_SUPPORT
            #include "avif_decoder.h"
        #endif
        #ifdef ENABLE_JXL_SUPPORT
            #include "jxl_decoder.h"
        #endif
        #ifdef ENABLE_RAW_SUPPORT
            #include "raw_decoder.h"
        #endif
        #ifdef ENABLE_PDF_SUPPORT
            #include "pdf_decoder.h"
        #endif
        #ifdef ENABLE_SVG_SUPPORT
            #include "svg_decoder.h"
        #endif
        #ifdef ENABLE_VIDEO_SUPPORT
            #include "video_thumbnail.h"
        #endif
        #ifdef ENABLE_AUDIO_SUPPORT
            #include "audio_thumbnail.h"
        #endif
        #ifdef ENABLE_DOCUMENT_SUPPORT
            #include "document_thumbnail.h"
        #endif
        #ifdef ENABLE_FONT_SUPPORT
            #include "font_preview.h"
        #endif
    #endif  // LENSShell_LEGACY_DECODERS

    #ifdef ENABLE_LIBARCHIVE_SUPPORT
        #include "libarchive_wrapper.h"
    #endif

    // ATL headers
    #include <atlfile.h>
    #include <atlimage.h>

    // WTL headers
    #include <atlgdi.h>

    // RAR support can be disabled via DISABLE_RAR_SUPPORT (defined in vcxproj)

    // UnRAR includes
    #ifndef DISABLE_RAR_SUPPORT
        #include "unrar.h"
        #ifdef _WIN64
            #pragma comment(lib, "unrar64.lib")
        #else
            #pragma comment(lib, "unrar.lib")
        #endif
    #endif

    #include <string>

    #include "LENSTypes.h"  // Extracted type constants
    #include "unzip.h"
// LENSTYPE_*, IMGTYPE_*, LENS_APP_KEY — now defined in LENSTypes.h

namespace __LENS {

// ========================================================================
// CUnzip — Lightweight ZIP archive reader
//
// Wraps the minizip library to provide sequential item enumeration and
// in-memory extraction from ZIP archives. Used to read images out of
// .zip, .cbz, .epub, .phz and other ZIP-based container formats.
//
// Usage:
//   CUnzip z;
//   z.Open(L"archive.cbz");
//   for (int i = 0; i < z.GetItemCount(); i++) {
//       z.GetItem(i);
//       if (!z.ItemIsDirectory())
//           z.UnzipItemToMembuffer(i, buf, z.GetItemUnpackedSize());
//   }
// ========================================================================
class CUnzip
{
  public:
    CUnzip()
    {
        hz = NULL;
    }
    virtual ~CUnzip()
    {
        ::CloseZip(hz);
    }

  public:
    bool Open(LPCTSTR zfile)
    {
        if (zfile == NULL)
            return false;
        HZIP temp_hz = ::OpenZip(zfile, NULL);
        if (temp_hz == NULL)
            return false;
        Close();
        hz = temp_hz;
        if (ZR_OK != ::GetZipItem(hz, -1, &maindirEntry))
            return false;
        return true;
    }

    bool GetItem(int zi)
    {
        zr = ::GetZipItem(hz, zi, &ZipEntry);
        return (ZR_OK == zr);
    }

    bool UnzipItemToMembuffer(int index, void* z, unsigned int len)
    {
        zr = ::UnzipItem(hz, index, z, len);
        return (ZR_OK == zr);
    }

    void Close()
    {
        CloseZip(hz);
        hz = NULL;
    }

    inline BOOL ItemIsDirectory()
    {
        return (BOOL)(CUnzip::GetItemAttributes() & 0x0010);
    }
    int GetItemCount() const
    {
        return maindirEntry.index;
    }
    long GetItemPackedSize() const
    {
        return ZipEntry.comp_size;
    }
    long GetItemUnpackedSize() const
    {
        return ZipEntry.unc_size;
    }
    DWORD GetItemAttributes() const
    {
        return ZipEntry.attr;
    }
    LPCTSTR GetItemName()
    {
        return ZipEntry.name;
    }

  private:
    ZIPENTRY ZipEntry, maindirEntry;
    HZIP hz;
    ZRESULT zr;
};

    #ifndef DISABLE_RAR_SUPPORT
// ========================================================================
// CUnRar — RAR archive reader wrapper
//
// Wraps the unrar DLL to enumerate and extract entries from RAR archives.
// Supports callback-based streaming extraction via IStream, which allows
// piping decompressed image data directly for thumbnail generation.
//
// Skips solid archives, multi-volume sets, and encrypted-header archives
// since those require full sequential extraction or passwords.
// ========================================================================
typedef const RARHeaderDataEx* LPCRARHeaderDataEx;
typedef const RAROpenArchiveDataEx* LPCRAROpenArchiveDataEx;

class CUnRar
{
  public:
    CUnRar()
    {
        if (RAR_DLL_VERSION > RARGetDllVersion())
            throw RAR_DLL_VERSION;
        _init();
    }
    virtual ~CUnRar()
    {
        Close();
        _init();
    }

  public:
    BOOL Open(LPCTSTR rarfile, BOOL bListingOnly = TRUE, char* cmtBuf = NULL, UINT cmtBufSize = 0, char* password = NULL)
    {
        if (m_harc)
            return FALSE;  // must close old first

        SecureZeroMemory(&m_arcinfo, sizeof(RAROpenArchiveDataEx));
        #ifndef UNICODE
        m_arcinfo.ArcName = (PTCHAR)rarfile;
        #else
        m_arcinfo.ArcNameW = (PTCHAR)rarfile;
        #endif
        m_arcinfo.OpenMode = bListingOnly ? RAR_OM_LIST : RAR_OM_EXTRACT;
        m_arcinfo.CmtBuf = cmtBuf;
        m_arcinfo.CmtBufSize = cmtBufSize;

        m_harc = RAROpenArchiveEx(&m_arcinfo);
        if (m_harc == NULL || m_arcinfo.OpenResult != 0)
            return FALSE;

        if (password)
            RARSetPassword(m_harc, password);
        RARSetCallback(m_harc, __rarCallbackProc, (LPARAM)this);
        return TRUE;
    }

    BOOL Close()
    {
        if (m_harc)
            m_ret = RARCloseArchive(m_harc);
        return (m_ret != 0);
    }

    inline LPCRAROpenArchiveDataEx GetArchiveInfo()
    {
        return &m_arcinfo;
    }
    inline LPCRARHeaderDataEx GetItemInfo()
    {
        return &m_iteminfo;
    }
    inline void SetPassword(char* password)
    {
        RARSetPassword(m_harc, password);
    }
    inline int GetLastError()
    {
        return m_ret;
    }
    inline BOOL IsArchiveVolume()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0001);
    }
    inline BOOL IsArchiveComment()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0002);
    }
    inline BOOL IsArchiveLocked()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0004);
    }
    inline BOOL IsArchiveSolid()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0008);
    }
    inline BOOL IsArchivePartN()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0010);
    }
    inline BOOL IsArchiveSigned()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0020);
    }
    inline BOOL IsArchiveRecoveryRecord()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0040);
    }
    inline BOOL IsArchiveEncryptedHeaders()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0080);
    }
    inline BOOL IsArchiveFirstVolume()
    {
        return (BOOL)(m_arcinfo.Flags & 0x0100);
    }

    BOOL ReadItemInfo()
    {
        SecureZeroMemory(&m_iteminfo, sizeof(RARHeaderDataEx));
        m_ret = RARReadHeaderEx(m_harc, &m_iteminfo);
        return (m_ret == 0);
    }

    inline BOOL IsItemDirectory()
    {
        return ((m_iteminfo.Flags & 0x00E0) == 0x00E0);
    }

    inline LPCTSTR GetItemName()
    {
        #ifdef UNICODE
        return (LPCTSTR)(GetItemInfo()->FileNameW);
        #else
        return (LPCTSTR)CUnRar::GetItemInfo()->FileName;
        #endif
    }

    inline UINT64 GetItemPackedSize64()
    {
        return ((((UINT64)m_iteminfo.PackSizeHigh) << 32) | m_iteminfo.PackSize);
    }
    inline UINT64 GetItemUnpackedSize64()
    {
        return ((((UINT64)m_iteminfo.UnpSizeHigh) << 32) | m_iteminfo.UnpSize);
    }

    virtual BOOL ProcessItem()
    {
        m_ret = RARProcessFileW(m_harc, RAR_TEST, NULL, NULL);
        if (m_ret != 0)
            return FALSE;
        return TRUE;
    }

    virtual BOOL SkipItem()
    {
        m_ret = RARProcessFile(m_harc, RAR_SKIP, NULL, NULL);
        return (m_ret == 0);
    }

    virtual BOOL SkipItems(UINT64 si)
    {
        UINT64 _i;
        for (_i = 0; _i < si; _i++) {
            if (!ReadItemInfo() || !SkipItem())
                return FALSE;
        }
        return TRUE;
    }

    virtual int CallbackProc(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
    {
        if (msg == UCM_PROCESSDATA)
            return ProcessItemData((LPBYTE)P1, (ULONG)P2);
        return -1;
    }

    virtual int ProcessItemData(LPBYTE pBuf, ULONG dwBufSize)
    {
        if (m_pIs) {
            ULONG br = 0;
            if (S_OK == m_pIs->Write(pBuf, dwBufSize, &br))
                if (br == dwBufSize)
                    return 1;
        }
        return -1;
    }

    void SetIStream(IStream* pIs)
    {
        _ASSERTE(pIs);
        m_pIs = pIs;
    }

  private:
    HANDLE m_harc;
    RARHeaderDataEx m_iteminfo;
    RAROpenArchiveDataEx m_arcinfo;
    int m_ret;
    IStream* m_pIs;
    void _init()
    {
        m_harc = NULL;
        m_pIs = NULL;
        m_ret = 0;
        SecureZeroMemory(&m_arcinfo, sizeof(RAROpenArchiveDataEx));
        SecureZeroMemory(&m_iteminfo, sizeof(RARHeaderDataEx));
    }

    // Static callback trampoline for unrar DLL
  public:
    static int PASCAL __rarCallbackProc(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
    {
        CUnRar* _pc = (CUnRar*)UserData;
        return _pc->CallbackProc(msg, UserData, P1, P2);
    }
};
    #endif  // DISABLE_RAR_SUPPORT

// ========================================================================
// CLENSArchive — Main COM shell extension thumbnail provider
//
// Implements IPersistFile::Load, IExtractImage::GetLocation/Extract,
// IExtractImage2::GetDateStamp, and IQueryInfo::GetInfoTip to generate
// thumbnails for archives, ebooks, images, video, audio, documents,
// fonts, and 3D models in Windows Explorer.
//
// Lifecycle:
//   1. Explorer calls OnLoad() with the file path
//   2. OnGetLocation() receives the requested thumbnail size
//   3. OnExtract() produces an HBITMAP thumbnail via format-specific
//      decoding (ZIP extraction, RAR extraction, Engine pipeline, etc.)
//   4. OnGetInfoTip() provides hover tooltip text
// ========================================================================
class CLENSArchive
{
  public:
    CLENSArchive()
    {
        m_bSort = TRUE;
        m_thumbSize.cx = m_thumbSize.cy = 0;
        m_LENSTYPE = LENSTYPE_NONE;
        m_pIs = NULL;
    }

    virtual ~CLENSArchive()
    {
        if (m_pIs) {
            m_pIs->Release();
            m_pIs = NULL;
        }
    }

  public:
    // ----------------------------------------------------------------
    // OnLoad — IPersistFile::Load handler
    //
    // Called by the shell with the full path of the file being
    // thumbnailed. Extracts the file extension, converts it to
    // lowercase, and resolves the corresponding LENSTYPE.
    //
    // Input:  wszFile — null-terminated wide-string file path
    // Output: S_OK always; populates m_LENSFile and m_LENSTYPE
    // ----------------------------------------------------------------
    HRESULT OnLoad(LPCOLESTR wszFile)
    {
    #ifndef UNICODE
        USES_CONVERSION;
        m_LENSFile = OLE2T((WCHAR*)wszFile);
    #else
        m_LENSFile = wszFile;
    #endif
        fs::path filePath(wszFile);
        std::wstring ext = ModernCpp::GetFileExtensionLower(filePath);
        m_LENSTYPE = GetLENSTYPE(ext.c_str());
        return S_OK;
    }

    // ----------------------------------------------------------------
    // OnGetLocation — IExtractImage::GetLocation handler
    //
    // Stores the requested thumbnail dimensions and sets cache flags.
    //
    // Input:  prgSize  — desired thumbnail width/height
    //         pdwFlags — extraction flags (modified: IEIFLAG_CACHE|REFRESH)
    // Output: NOERROR always
    // ----------------------------------------------------------------
    HRESULT OnGetLocation(const SIZE* prgSize, DWORD* pdwFlags)
    {
        m_thumbSize.cx = prgSize->cx;
        m_thumbSize.cy = prgSize->cy;
        *pdwFlags |= (IEIFLAG_CACHE | IEIFLAG_REFRESH);
    #ifdef _DEBUG
        if (*pdwFlags & IEIFLAG_ASYNC)
            ATLTRACE("\nIExtractImage::GetLocation : IEIFLAG_ASYNC flag set\n");
    #endif
        return NOERROR;
    }

    // ----------------------------------------------------------------
    // OnGetDateStamp — IExtractImage2::GetDateStamp handler
    //
    // Returns the file's last-modification timestamp so the shell
    // cache can invalidate stale thumbnails. Tries std::filesystem
    // first, falls back to Win32 GetFileTime on failure.
    //
    // Input:  pDateStamp — receives the FILETIME to write
    // Output: NOERROR on success, E_FAIL if the file cannot be read
    // ----------------------------------------------------------------
    HRESULT OnGetDateStamp(FILETIME* pDateStamp)
    {
        // Modern C++17: Use filesystem for file time
        fs::path filePath(m_LENSFile.operator LPCTSTR());
        auto modTime = ModernCpp::GetFileModificationTime(filePath);

        if (modTime.has_value()) {
            *pDateStamp = ModernCpp::FileTimeToWin32(*modTime);
            return NOERROR;
        }

        // Fallback to legacy API if filesystem fails
        FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
        CAtlFile _f;
        if (S_OK != _f.Create(m_LENSFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0))
            return E_FAIL;
        if (!GetFileTime(_f, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime))
            return E_FAIL;
        *pDateStamp = ftLastWriteTime;
        return NOERROR;
    }

    // ----------------------------------------------------------------
    // ReplaceStringInPlace — Simple in-place substring replacement
    //
    // Replaces all occurrences of 'search' with 'replace' in 'subject'.
    // ----------------------------------------------------------------
    void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    // ----------------------------------------------------------------
    // GetEpubRootFile — Locate the OPF root file in an EPUB archive
    //
    // Opens the EPUB (which is a ZIP), reads META-INF/container.xml,
    // and parses the <rootfile full-path="..."> attribute to find the
    // path to the OPF package document inside the archive.
    //
    // Input:  ePubFile — path to the .epub file on disk
    // Output: path string to the OPF file (e.g. "OEBPS/content.opf"),
    //         or empty string on failure
    // ----------------------------------------------------------------
    std::string GetEpubRootFile(LPCTSTR ePubFile)
    {
        CUnzip _z;
        if (!_z.Open(ePubFile))
            return std::string();
        j = _z.GetItemCount();
        if (j == 0)
            return std::string();

        size_t posStart, posEnd;

        USES_CONVERSION;

        std::string xmlContent, rootfile;

        // Find
        for (i = 0; i < j; i++) {
            if (!_z.GetItem(i))
                break;
            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                continue;

            std::string name = T2A(_z.GetItemName());

            if (_stricmp(name.c_str(), "META-INF/container.xml") == 0) {
                // Extract container.xml and retrieve rootfile location

                HGLOBAL hGContainer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
                if (hGContainer) {
                    bool b = false;
                    LPVOID pBuf = ::GlobalLock(hGContainer);
                    if (pBuf)
                        b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

                    if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR) {
                        if (b) {
                            xmlContent = (char*)pBuf;

                            posStart = xmlContent.find("rootfile ");

                            if (posStart == std::string::npos) {
                                break;
                            }

                            posStart = xmlContent.find("full-path=\"", posStart);

                            if (posStart == std::string::npos) {
                                break;
                            }

                            posStart += 11;
                            posEnd = xmlContent.find("\"", posStart);

                            rootfile = xmlContent.substr(posStart, posEnd - posStart);

                            break;
                        }
                    }
                }
            }
        }

        return rootfile;
    }

    // ----------------------------------------------------------------
    // ExtractMultipleImagesFromZIP — Pull N images from a ZIP for collage
    //
    // Scans the ZIP for image entries (filtered by IsImage()), optionally
    // sorted alphabetically, and extracts up to maxImages as HBITMAPs.
    // Used by the thumbnail collage mode to compose a multi-image
    // preview grid.
    //
    // Input:  _z        — already-opened CUnzip archive handle
    //         maxImages — maximum number of images to extract
    // Output: vector of HBITMAP handles (caller owns and must delete)
    // ----------------------------------------------------------------
    std::vector<HBITMAP> ExtractMultipleImagesFromZIP(CUnzip& _z, int maxImages)
    {
        std::vector<HBITMAP> images;
        std::vector<int> imageIndices;

        // Find all image files
        int j = _z.GetItemCount();
        CString prevname;

        for (int i = 0; i < j && imageIndices.size() < (size_t)maxImages; i++) {
            if (!_z.GetItem(i))
                break;
            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                continue;
            if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
                continue;

            if (IsImage(_z.GetItemName())) {
                if (m_bSort) {
                    // Collect for sorting
                    imageIndices.push_back(i);
                    if (prevname.IsEmpty())
                        prevname = _z.GetItemName();
                    // Take only first alphabetical names
                    if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname)) {
                        imageIndices.back() = i;
                        prevname = _z.GetItemName();
                    }
                } else {
                    // No sort, just collect first N
                    imageIndices.push_back(i);
                }
            }
        }

        // Extract each image
        for (int idx : imageIndices) {
            if (!_z.GetItem(idx))
                continue;

            HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
            if (hG) {
                bool b = false;
                LPVOID pBuf = ::GlobalLock(hG);
                if (pBuf)
                    b = _z.UnzipItemToMembuffer(idx, pBuf, _z.GetItemUnpackedSize());

                if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR) {
                    if (b) {
                        IStream* pIs = NULL;
                        if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs)) {
                            HBITMAP hBmp = ThumbnailFromIStream(pIs, &m_thumbSize);
                            if (hBmp) {
                                images.push_back(hBmp);
                            }
                            pIs->Release();
                            pIs = NULL;
                        }
                    }
                }
            }
        }

        return images;
    }

    // ================================================================
    // OnExtract — IExtractImage::Extract handler (core thumbnail entry)
    //
    // Produces an HBITMAP thumbnail for the file loaded by OnLoad().
    // Routes to the appropriate extraction strategy based on m_LENSTYPE:
    //  - ZIP/CBZ/CB7/CBT/PHZ/MOBI/AZW/FB2: extract first image from
    //    archive, optionally compose a multi-image collage
    //  - EPUB: parse OPF to locate the cover image, extract it
    //  - 7Z: delegate to 7-Zip extraction
    //  - LibArchive formats (TAR/ISO/CPIO/etc.): use libarchive wrapper
    //  - VIDEO/AUDIO: extract embedded art or representative frame
    //  - DOCX/PPTX/XLSX: extract embedded thumbnail from Office XML
    //  - FONT: render a font preview bitmap
    //  - Standalone images: decode via ThumbnailFromIStream
    //
    // All extracted bitmaps are scaled to m_thumbSize and cached by
    // ThumbnailCache for fast subsequent lookups.
    //
    // Input:  phBmpThumbnail — receives the generated HBITMAP
    // Output: S_OK on success (bitmap set), E_FAIL on decode failure
    // ================================================================
    HRESULT OnExtract(HBITMAP* phBmpThumbnail)
    {
        *phBmpThumbnail = NULL;

        // Initialize the on-disk thumbnail cache (once per process)
        static bool cacheInitialized = false;
        if (!cacheInitialized) {
            ExplorerLens::ThumbnailCache::Initialize();
            cacheInitialized = true;
        }

        // Get file metadata for cache key
        __int64 fileSize = 0;
        GetFileSizeCrt(m_LENSFile, fileSize);

        FILETIME lastModified = {0};
        OnGetDateStamp(&lastModified);

        std::wstring archivePath(m_LENSFile.operator LPCTSTR());
        // Include archive type and requested size in cache key for better cache
        // hits
        WCHAR imageName[64];
        swprintf_s(imageName, L"thumb_%d_%dx%d", m_LENSTYPE, m_thumbSize.cx, m_thumbSize.cy);

        // Check cache first (fast path)
        if (ExplorerLens::ThumbnailCache::IsCached(archivePath, imageName, (ULONGLONG)fileSize, lastModified)) {
            HBITMAP cachedBitmap =
                ExplorerLens::ThumbnailCache::LoadFromCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified);
            if (cachedBitmap) {
                *phBmpThumbnail = cachedBitmap;
                return S_OK;
            }
        }

        try {
            switch (m_LENSTYPE) {
                case LENSTYPE_EPUB: {
                    std::string xmlContent, rootpath, rootfile, coverfile;

                    rootfile = GetEpubRootFile(m_LENSFile);

                    CUnzip _z;
                    if (!_z.Open(m_LENSFile))
                        return E_FAIL;
                    j = _z.GetItemCount();
                    if (j == 0)
                        return E_FAIL;

                    size_t posStart, posEnd;

                    CString prevname;  // helper vars
                    int thumbindex = -1;

                    USES_CONVERSION;

                    if (rootfile.length() > 0) {
                        posStart = rootfile.find('/');
                        if (posStart != std::string::npos) {
                            rootpath = rootfile.substr(0, posStart + 1);
                        }

                        for (i = 0; i < j; i++) {
                            if (!_z.GetItem(i))
                                break;
                            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                                continue;

                            std::string name;

                            name = CT2A(_z.GetItemName());

                            if (name.compare(rootfile) == 0) {
                                HGLOBAL hGContainer =
                                    GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
                                if (hGContainer) {
                                    bool b = false;
                                    LPVOID pBuf = ::GlobalLock(hGContainer);
                                    if (pBuf)
                                        b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

                                    if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR) {
                                        if (b) {
                                            // Find meta tag for cover

                                            std::string xmlContent, coverTag, coverId, itemTag;

                                            xmlContent = (char*)pBuf;

                                            posStart = xmlContent.find("name=\"cover\"");

                                            if (posStart == std::string::npos) {
                                                break;
                                            }

                                            posStart = xmlContent.find_last_of("<", posStart);
                                            posEnd = xmlContent.find(">", posStart);

                                            coverTag = xmlContent.substr(posStart, posEnd - posStart + 1);

                                            // Find cover item id

                                            posStart = coverTag.find("content=\"");

                                            if (posStart == std::string::npos) {
                                                break;
                                            }

                                            posStart += 9;
                                            posEnd = coverTag.find("\"", posStart);

                                            coverId = coverTag.substr(posStart, posEnd - posStart);

                                            // Find item tag in original opf file contents

                                            posStart = xmlContent.find("id=\"" + coverId + "\"");

                                            if (posStart == std::string::npos) {
                                                break;
                                            }

                                            posStart = xmlContent.find_last_of("<", posStart);
                                            posEnd = xmlContent.find(">", posStart);

                                            itemTag = xmlContent.substr(posStart, posEnd - posStart + 1);

                                            // Find cover path in item tag

                                            posStart = itemTag.find("href=\"");

                                            if (posStart == std::string::npos) {
                                                break;
                                            }

                                            posStart += 6;
                                            posEnd = itemTag.find("\"", posStart);

                                            if (posEnd == std::string::npos) {
                                                break;
                                            }

                                            coverfile = rootpath + itemTag.substr(posStart, posEnd - posStart);
                                            ReplaceStringInPlace(coverfile, "%20", " ");
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (coverfile.empty()) {
                        for (i = 0; i < j; i++) {
                            if (!_z.GetItem(i))
                                break;
                            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                                continue;
                            if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
                                continue;

                            if (IsImage(_z.GetItemName())) {
                                std::string imgFilename(T2A(_z.GetItemName()));

                                if (imgFilename.find("cover") != std::string::npos) {
                                    coverfile = imgFilename;
                                } else if (imgFilename.find("COVER") != std::string::npos) {
                                    coverfile = imgFilename;
                                } else if (imgFilename.find("Cover") != std::string::npos) {
                                    coverfile = imgFilename;
                                }
                            }
                        }
                    }

                    if (coverfile.length() > 0) {
                        for (i = 0; i < j; i++) {
                            if (!_z.GetItem(i))
                                break;
                            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                                continue;
                            if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
                                continue;

                            if (_stricmp(T2A(_z.GetItemName()), coverfile.c_str()) == 0 && IsImage(_z.GetItemName())) {
                                if (thumbindex < 0)
                                    thumbindex = i;  // assign thumbindex if already sorted

                                if (!m_bSort)
                                    break;  // if NoSort

                                if (prevname.IsEmpty())
                                    prevname = _z.GetItemName();  // can't compare empty string
                                // take only first alphabetical name
                                if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname)) {
                                    thumbindex = i;
                                    prevname = _z.GetItemName();
                                }
                            }
                        }  // for loop

                        if (thumbindex < 0)
                            return E_FAIL;
                        // go to thumb index
                        if (!_z.GetItem(thumbindex))
                            return E_FAIL;

                        // create thumb			//GHND
                        HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
                        if (hG) {
                            bool b = false;
                            LPVOID pBuf = ::GlobalLock(hG);
                            if (pBuf)
                                b = _z.UnzipItemToMembuffer(thumbindex, pBuf, _z.GetItemUnpackedSize());

                            if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR) {
                                if (b) {
                                    IStream* pIs = NULL;
                                    if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs)) {
                                        *phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
                                        pIs->Release();
                                        pIs = NULL;
                                    }
                                }
                            }
                        }
                        return ((*phBmpThumbnail) ? S_OK : E_FAIL);
                    }

                    // something wrong with the epub, try falling back on first image in zip
                }
                case LENSTYPE_ZIP:
                case LENSTYPE_CBZ:
                case LENSTYPE_7Z:
                case LENSTYPE_CB7:
                case LENSTYPE_CBT:
                case LENSTYPE_MOBI:
                case LENSTYPE_AZW:
                case LENSTYPE_AZW3:
                case LENSTYPE_FB2:
                case LENSTYPE_PHZ: {
                    CUnzip _z;
                    if (!_z.Open(m_LENSFile))
                        return E_FAIL;
                    j = _z.GetItemCount();
                    if (j == 0)
                        return E_FAIL;

                    // Check if collage mode is enabled via registry
                    ExplorerLens::ThumbnailCollage::CollageMode collageMode =
                        ExplorerLens::ThumbnailCollage::GetCollageModeFromRegistry();

                    // Extract multiple images if collage mode is enabled
                    if (collageMode != ExplorerLens::ThumbnailCollage::MODE_SINGLE) {
                        int maxImages = static_cast<int>(collageMode);  // MODE_2X2=4, MODE_3X3=9, MODE_4X4=16
                        std::vector<HBITMAP> images = ExtractMultipleImagesFromZIP(_z, maxImages);

                        if (!images.empty()) {
                            // Create collage from multiple images
                            *phBmpThumbnail = ExplorerLens::ThumbnailCollage::CreateCollage(
                                images, m_thumbSize.cx, m_thumbSize.cy, collageMode);

                            // Clean up individual images
                            for (HBITMAP hBmp : images) {
                                if (hBmp && hBmp != *phBmpThumbnail) {
                                    DeleteObject(hBmp);
                                }
                            }

                            // Save to cache
                            if (*phBmpThumbnail) {
                                ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                          lastModified, *phBmpThumbnail);
                            }

                            return ((*phBmpThumbnail) ? S_OK : E_FAIL);
                        }
                    }

                    // Fallback to single image mode (original logic)
                    CString prevname;  // helper vars
                    int thumbindex = -1;

                    for (i = 0; i < j; i++) {
                        if (!_z.GetItem(i))
                            break;
                        if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                            continue;
                        if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
                            continue;

                        if (IsImage(_z.GetItemName())) {
                            if (thumbindex < 0)
                                thumbindex = i;  // assign thumbindex if already sorted

                            if (!m_bSort)
                                break;  // if NoSort

                            if (prevname.IsEmpty())
                                prevname = _z.GetItemName();  // can't compare empty string
                            // take only first alphabetical name
                            if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname)) {
                                thumbindex = i;
                                prevname = _z.GetItemName();
                            }
                        }
                    }  // for loop

                    if (thumbindex < 0)
                        return E_FAIL;
                    // go to thumb index
                    if (!_z.GetItem(thumbindex))
                        return E_FAIL;

                    // create thumb			//GHND
                    HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
                    if (hG) {
                        bool b = false;
                        LPVOID pBuf = ::GlobalLock(hG);
                        if (pBuf)
                            b = _z.UnzipItemToMembuffer(thumbindex, pBuf, _z.GetItemUnpackedSize());

                        if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR) {
                            if (b) {
                                IStream* pIs = NULL;
                                if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs)) {
                                    *phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
                                    pIs->Release();
                                    pIs = NULL;
                                }
                            }
                        }
                    }

                    return ((*phBmpThumbnail) ? S_OK : E_FAIL);
                } break;

    #ifdef ENABLE_LIBARCHIVE_SUPPORT
                    // LibArchive formats: TAR, TAR.GZ, TAR.BZ2, TAR.XZ, TAR.ZST, CPIO, ISO,
                    // XAR, AR, DEB, CAB
                case LENSTYPE_TAR:
                case LENSTYPE_TAR_GZ:
                case LENSTYPE_TAR_BZ2:
                case LENSTYPE_TAR_XZ:
                case LENSTYPE_TAR_ZST:
                case LENSTYPE_CPIO:
                case LENSTYPE_ISO:
                case LENSTYPE_XAR:
                case LENSTYPE_AR:
                case LENSTYPE_DEB:
                case LENSTYPE_CAB: {
                    // Open archive with libarchive
                    if (!ExplorerLens::LibArchiveWrapper::OpenArchive(m_LENSFile)) {
                        return E_FAIL;
                    }

                    CString prevname;
                    int thumbindex = -1;
                    int currentIndex = 0;
                    std::vector<std::pair<std::string, int>> imageEntries;  // name, index

                    // Enumerate entries to find images
                    while (ExplorerLens::LibArchiveWrapper::ReadNextEntry()) {
                        if (ExplorerLens::LibArchiveWrapper::IsEntryDirectory()) {
                            currentIndex++;
                            continue;
                        }

                        int64_t entrySize = ExplorerLens::LibArchiveWrapper::GetEntrySize();
                        if (entrySize <= 0 || entrySize > LENSMEM_MAXBUFFER_SIZE) {
                            currentIndex++;
                            continue;
                        }

                        const char* entryName = ExplorerLens::LibArchiveWrapper::GetEntryName();
                        if (!entryName) {
                            currentIndex++;
                            continue;
                        }

                        // Convert to wide string for IsImage check
                        USES_CONVERSION;
                        LPCTSTR wEntryName = A2CT(entryName);

                        if (IsImage(wEntryName)) {
                            imageEntries.push_back(std::make_pair(std::string(entryName), currentIndex));

                            if (!m_bSort) {
                                thumbindex = currentIndex;
                                break;  // Take first image if not sorting
                            }
                        }
                        currentIndex++;
                    }

                    // If sorting, find alphabetically first image
                    if (m_bSort && !imageEntries.empty()) {
                        std::string firstName = imageEntries[0].first;
                        thumbindex = imageEntries[0].second;

                        for (size_t i = 1; i < imageEntries.size(); i++) {
                            if (imageEntries[i].first < firstName) {
                                firstName = imageEntries[i].first;
                                thumbindex = imageEntries[i].second;
                            }
                        }
                    }

                    if (thumbindex < 0) {
                        ExplorerLens::LibArchiveWrapper::CloseArchive();
                        return E_FAIL;
                    }

                    // Re-open and seek to thumbnail entry
                    ExplorerLens::LibArchiveWrapper::CloseArchive();
                    if (!ExplorerLens::LibArchiveWrapper::OpenArchive(m_LENSFile)) {
                        return E_FAIL;
                    }

                    currentIndex = 0;
                    while (ExplorerLens::LibArchiveWrapper::ReadNextEntry()) {
                        if (currentIndex == thumbindex) {
                            int64_t entrySize = ExplorerLens::LibArchiveWrapper::GetEntrySize();

                            // Extract entry to memory
                            HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)entrySize);
                            if (hG) {
                                LPVOID pBuf = GlobalLock(hG);
                                if (pBuf) {
                                    SSIZE_T bytesRead =
                                        ExplorerLens::LibArchiveWrapper::ExtractEntryToMemory(pBuf, (size_t)entrySize);

                                    if (GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR) {
                                        if (bytesRead > 0) {
                                            IStream* pIs = NULL;
                                            if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs)) {
                                                *phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
                                                pIs->Release();
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        currentIndex++;
                    }

                    ExplorerLens::LibArchiveWrapper::CloseArchive();
                    return ((*phBmpThumbnail) ? S_OK : E_FAIL);
                } break;
    #endif  // ENABLE_LIBARCHIVE_SUPPORT

                case LENSTYPE_VIDEO: {
    #if defined(LENSShell_LEGACY_DECODERS) && defined(ENABLE_VIDEO_SUPPORT)
                    // Extract video thumbnail using DirectShow
                    std::wstring videoPath(m_LENSFile.operator LPCTSTR());

                    // Extract frame at 10% into video (skip intros)
                    HBITMAP hVideoFrame = ExplorerLens::VideoThumbnail::ExtractFrame(videoPath, 0.1);

                    if (hVideoFrame) {
                        // Scale to thumbnail size
                        *phBmpThumbnail = ScaleBitmapToThumbnail(hVideoFrame, &m_thumbSize);

                        // Clean up original if different from scaled
                        if (*phBmpThumbnail != hVideoFrame) {
                            DeleteObject(hVideoFrame);
                        }

                        // Save to cache
                        if (*phBmpThumbnail) {
                            ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                      lastModified, *phBmpThumbnail);
                        }

                        return S_OK;
                    }
    #endif
                    return E_FAIL;
                } break;

                case LENSTYPE_AUDIO: {
    #if defined(LENSShell_LEGACY_DECODERS) && defined(ENABLE_AUDIO_SUPPORT)
                    // Extract audio album art or generate waveform thumbnail
                    std::wstring audioPath(m_LENSFile.operator LPCTSTR());

                    // Try to extract album art first
                    HBITMAP hAlbumArt = ExplorerLens::AudioThumbnail::ExtractAlbumArt(audioPath);

                    if (hAlbumArt) {
                        // Scale to thumbnail size
                        *phBmpThumbnail = ScaleBitmapToThumbnail(hAlbumArt, &m_thumbSize);

                        // Clean up original if different from scaled
                        if (*phBmpThumbnail != hAlbumArt) {
                            DeleteObject(hAlbumArt);
                        }

                        // Save to cache
                        if (*phBmpThumbnail) {
                            ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                      lastModified, *phBmpThumbnail);
                        }

                        return S_OK;
                    }

                    // No album art, generate waveform visualization
                    HBITMAP hWaveform =
                        ExplorerLens::AudioThumbnail::GenerateWaveform(audioPath, m_thumbSize.cx, m_thumbSize.cy);

                    if (hWaveform) {
                        *phBmpThumbnail = hWaveform;

                        // Save to cache
                        ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                  lastModified, *phBmpThumbnail);

                        return S_OK;
                    }
    #endif
                    return E_FAIL;
                } break;

                case LENSTYPE_DOCX:
                case LENSTYPE_PPTX:
                case LENSTYPE_XLSX:
                case LENSTYPE_DOC:
                case LENSTYPE_PPT:
                case LENSTYPE_XLS: {
    #if defined(LENSShell_LEGACY_DECODERS) && defined(ENABLE_DOCUMENT_SUPPORT)
                    // Extract embedded document thumbnail from Office XML
                    std::wstring docPath(m_LENSFile.operator LPCTSTR());

                    HBITMAP hDocThumbnail = ExplorerLens::DocumentThumbnail::ExtractDocumentThumbnail(
                        docPath, m_thumbSize.cx, m_thumbSize.cy);

                    if (hDocThumbnail) {
                        *phBmpThumbnail = hDocThumbnail;

                        // Save to cache
                        ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                  lastModified, *phBmpThumbnail);

                        return S_OK;
                    }
    #endif
                    return E_FAIL;
                } break;

                case LENSTYPE_FONT: {
    #if defined(LENSShell_LEGACY_DECODERS) && defined(ENABLE_FONT_SUPPORT)
                    // Render a font sample preview bitmap
                    std::wstring fontPath(m_LENSFile.operator LPCTSTR());

                    HBITMAP hFontPreview =
                        ExplorerLens::FontPreview::GenerateFontPreview(fontPath, m_thumbSize.cx, m_thumbSize.cy);

                    if (hFontPreview) {
                        *phBmpThumbnail = hFontPreview;

                        // Save to cache
                        ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize,
                                                                  lastModified, *phBmpThumbnail);

                        return S_OK;
                    }
    #endif
                    return E_FAIL;
                } break;

                    // Standalone image/document formats (WebP, JPEG XL, AVIF, HEIF, SVG, RAW,
                    // TIFF, PDF)
                default: {
                    // Handle standalone files by loading them into a stream
                    // ThumbnailFromIStream will detect and decode the format

                    CAtlFile file;
                    HRESULT hr = file.Create(m_LENSFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0);
                    if (FAILED(hr))
                        return E_FAIL;

                    ULONGLONG fileSize64 = 0;
                    hr = file.GetSize(fileSize64);
                    if (FAILED(hr) || fileSize64 == 0 || fileSize64 > LENSMEM_MAXBUFFER_SIZE) {
                        return E_FAIL;
                    }

                    // Read file into memory
                    HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)fileSize64);
                    if (!hG)
                        return E_FAIL;

                    LPVOID pBuf = GlobalLock(hG);
                    if (!pBuf) {
                        GlobalFree(hG);
                        return E_FAIL;
                    }

                    DWORD bytesRead = 0;
                    hr = file.Read(pBuf, (DWORD)fileSize64, bytesRead);

                    if (GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR) {
                        if (SUCCEEDED(hr) && bytesRead == fileSize64) {
                            IStream* pIs = NULL;
                            if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs)) {
                                *phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
                                pIs->Release();

                                // Save to cache
                                if (*phBmpThumbnail) {
                                    ExplorerLens::ThumbnailCache::SaveToCache(
                                        archivePath, imageName, (ULONGLONG)fileSize64, lastModified, *phBmpThumbnail);
                                }

                                return ((*phBmpThumbnail) ? S_OK : E_FAIL);
                            }
                        }
                    }

                    GlobalFree(hG);
                    return E_FAIL;
                } break;
            }
        } catch (...) {
            ATLTRACE("exception in IExtractImage::Extract\n");
            return S_FALSE;
        }

        // Save generated thumbnail to cache
        if (*phBmpThumbnail) {
            ExplorerLens::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified,
                                                      *phBmpThumbnail);
        }

        return S_OK;
    }

    // ----------------------------------------------------------------
    // GetEpubTitle — Extract the book title from an EPUB
    //
    // Opens the EPUB, locates the OPF package file via
    // GetEpubRootFile(), then parses the <dc:title> element.
    //
    // Input:  ePubFile — path to the .epub file
    // Output: title string, or empty string if not found
    // ----------------------------------------------------------------
    std::string GetEpubTitle(LPCTSTR ePubFile)
    {
        std::string rootfile, title;

        rootfile = GetEpubRootFile(ePubFile);

        if (rootfile.empty()) {
            return std::string();
        }

        CUnzip _z;
        if (!_z.Open(m_LENSFile))
            return std::string();
        j = _z.GetItemCount();
        if (j == 0)
            return std::string();

        size_t posStart, posEnd;

        for (i = 0; i < j; i++) {
            if (!_z.GetItem(i))
                break;
            if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > LENSMEM_MAXBUFFER_SIZE))
                continue;

            std::string name;

            name = CT2A(_z.GetItemName());

            if (name.compare(rootfile) == 0) {
                HGLOBAL hGContainer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
                if (hGContainer) {
                    bool b = false;
                    LPVOID pBuf = ::GlobalLock(hGContainer);
                    if (pBuf)
                        b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

                    if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR) {
                        if (b) {
                            // Find <dc:title> tag

                            std::string xmlContent, coverTag, coverId, itemTag;

                            xmlContent = (char*)pBuf;

                            posStart = xmlContent.find("<dc:title>");

                            if (posStart == std::string::npos) {
                                break;
                            }

                            posStart += 10;

                            posEnd = xmlContent.find("</dc:title>", posStart);

                            title = xmlContent.substr(posStart, posEnd - posStart);

                            break;
                        }
                    }
                }
            }
        }

        return title;
    }

    // ----------------------------------------------------------------
    // OnGetInfoTip — IQueryInfo::GetInfoTip handler
    //
    // Generates a tooltip string for Explorer hover preview.
    // Displays format name, file size, and format-specific metadata
    // (e.g. image count for archives, EPUB title for ebooks).
    //
    // Input:  ppwszTip — receives CoTaskMemAlloc'd wide string
    // Output: S_OK on success, E_FAIL on error
    // ----------------------------------------------------------------
    HRESULT OnGetInfoTip(LPWSTR* ppwszTip)
    {
        try {
            CString tip;

            __int64 _fs;
            if (!GetFileSizeCrt(m_LENSFile, _fs))
                return E_FAIL;

            TCHAR _tf[16];  // SecureZeroMemory?

            switch (m_LENSTYPE) {
                case LENSTYPE_ZIP:
                    if (_fs == 0)
                        tip = _T("ZIP Archive\nSize: 0 bytes");
                    else {
                        if (GetImageCountZIP(m_LENSFile, i, j))
                            tip.Format(_T("ZIP Archive\n%d Images\n%d Files\nSize: %s"), i, j,
                                       StrFormatByteSize64(_fs, _tf, 16));
                        else
                            tip.Format(_T("ZIP Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    }
                    break;
                case LENSTYPE_CBZ:
                    if (_fs == 0)
                        tip = _T("ZIP Image Archive\nSize: 0 bytes");
                    else {
                        if (GetImageCountZIP(m_LENSFile, i, j))
                            tip.Format(_T("ZIP Image Archive\n%d Images\n%d Files\nSize: %s"), i, j,
                                       StrFormatByteSize64(_fs, _tf, 16));
                        else
                            tip.Format(_T("ZIP Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    }
                    break;
                case LENSTYPE_EPUB:
                    if (_fs == 0)
                        tip = _T("EPUB File\nSize: 0 bytes");
                    else {
                        std::string title = GetEpubTitle(m_LENSFile);
                        ATL::CAtlStringW titleW(CA2W(title.c_str()));

                        if (title.length() > 0)
                            tip.Format(_T("EPUB File\n%s\nSize: %s"), titleW, StrFormatByteSize64(_fs, _tf, 16));
                        else
                            tip.Format(_T("EPUB File\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    }
                    break;
                case LENSTYPE_7Z:
                    tip.Format(_T("7-Zip Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_CB7:
                    if (GetImageCountZIP(m_LENSFile, i, j))
                        tip.Format(_T("7-Zip Image Archive\n%d Images\n%d Files\nSize: %s"), i, j,
                                   StrFormatByteSize64(_fs, _tf, 16));
                    else
                        tip.Format(_T("7-Zip Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_TAR:
                    tip.Format(_T("TAR Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_CBT:
                    if (GetImageCountZIP(m_LENSFile, i, j))
                        tip.Format(_T("TAR Image Archive\n%d Images\n%d Files\nSize: %s"), i, j,
                                   StrFormatByteSize64(_fs, _tf, 16));
                    else
                        tip.Format(_T("TAR Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_MOBI:
                    tip.Format(_T("MOBI E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_AZW:
                    tip.Format(_T("Kindle (AZW) E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_AZW3:
                    tip.Format(_T("Kindle (AZW3) E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_FB2:
                    tip.Format(_T("FictionBook File\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;
                case LENSTYPE_PHZ:
                    if (GetImageCountZIP(m_LENSFile, i, j))
                        tip.Format(_T("Photo Archive\n%d Images\n%d Files\nSize: %s"), i, j,
                                   StrFormatByteSize64(_fs, _tf, 16));
                    else
                        tip.Format(_T("Photo Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
                    break;

                default:
                    ATLTRACE("IQueryInfo::GetInfoTip : LENSTYPE_NONE\n");
                    return E_FAIL;
            }

            *ppwszTip =
                (WCHAR*)::CoTaskMemAlloc((tip.GetLength() + 1) * sizeof(WCHAR));  // caller must call CoTaskMemFree
            if (*ppwszTip == NULL)
                return E_FAIL;
            if (0 != ::wcscpy_s(*ppwszTip, tip.GetLength() + 1, tip))
                return E_FAIL;

            return S_OK;
        } catch (...) {
            ATLTRACE("exception in IQueryInfo::GetInfoTip\n");
            return S_FALSE;
        }

        return S_FALSE;
    }

  private:
    CString m_LENSFile;
    SIZE m_thumbSize;
    int i, j;
    LENSTYPE m_LENSTYPE;
    IStream* m_pIs;
    BOOL m_bSort;
    BOOL m_showIcon;

  private:
    // ----------------------------------------------------------------
    // GetFileSizeCrt — Retrieve file size using CRT _stat64
    //
    // Input:  pszFile — file path, fsize — receives file size in bytes
    // Output: TRUE on success, FALSE if stat fails
    // ----------------------------------------------------------------
    inline BOOL GetFileSizeCrt(LPCTSTR pszFile, __int64& fsize)
    {
        struct _stat64 _s;
        _s.st_size = 0;
        if (0 != ::_tstat64(pszFile, &_s))
            return FALSE;
        fsize = _s.st_size;
        return TRUE;
    }

    inline BOOL StrEqual(LPCTSTR psz1, LPCTSTR psz2)
    {
        return (::StrCmpI(psz1, psz2) == 0);
    }

    // ----------------------------------------------------------------
    // IsImage — Check if a filename has a recognized image extension
    //
    // Tests the file extension (case-insensitive) against all supported
    // image formats: traditional (BMP/GIF/JPG/PNG/TIF), modern
    // (WebP/AVIF/JXL/HEIF), vector (SVG), and RAW camera formats.
    //
    // Input:  szFile — filename or path with extension
    // Output: TRUE if the extension matches a known image format
    // ----------------------------------------------------------------
    BOOL IsImage(LPCTSTR szFile)
    {
        LPWSTR _e = PathFindExtension(szFile);
        // Traditional raster formats
        if (StrEqual(_e, _T(".bmp")))
            return TRUE;
        if (StrEqual(_e, _T(".ico")))
            return TRUE;
        if (StrEqual(_e, _T(".gif")))
            return TRUE;
        if (StrEqual(_e, _T(".jpg")))
            return TRUE;
        if (StrEqual(_e, _T(".jpe")))
            return TRUE;
        if (StrEqual(_e, _T(".jfif")))
            return TRUE;
        if (StrEqual(_e, _T(".jpeg")))
            return TRUE;
        if (StrEqual(_e, _T(".png")))
            return TRUE;
        if (StrEqual(_e, _T(".tif")))
            return TRUE;
        if (StrEqual(_e, _T(".tiff")))
            return TRUE;
        // Modern image formats
        if (StrEqual(_e, _T(".webp")))
            return TRUE;
        if (StrEqual(_e, _T(".avif")))
            return TRUE;
        if (StrEqual(_e, _T(".jxl")))
            return TRUE;
        if (StrEqual(_e, _T(".jxr")))
            return TRUE;
        if (StrEqual(_e, _T(".heif")))
            return TRUE;
        if (StrEqual(_e, _T(".heic")))
            return TRUE;
        // Vector formats
        if (StrEqual(_e, _T(".svg")))
            return TRUE;
        // RAW camera formats
        if (StrEqual(_e, _T(".dng")))
            return TRUE;
        if (StrEqual(_e, _T(".cr2")))
            return TRUE;
        if (StrEqual(_e, _T(".cr3")))
            return TRUE;
        if (StrEqual(_e, _T(".nef")))
            return TRUE;
        if (StrEqual(_e, _T(".arw")))
            return TRUE;
        if (StrEqual(_e, _T(".orf")))
            return TRUE;
        if (StrEqual(_e, _T(".rw2")))
            return TRUE;
        if (StrEqual(_e, _T(".pef")))
            return TRUE;
        if (StrEqual(_e, _T(".raf")))
            return TRUE;
        return FALSE;
    }

    // ----------------------------------------------------------------
    // IsVideo — Check if a filename has a recognized video extension
    //
    // Input:  szFile — filename or path with extension
    // Output: TRUE if the extension matches a known video format
    // ----------------------------------------------------------------
    BOOL IsVideo(LPCTSTR szFile)
    {
        LPWSTR _e = PathFindExtension(szFile);
        if (StrEqual(_e, _T(".mp4")))
            return TRUE;
        if (StrEqual(_e, _T(".avi")))
            return TRUE;
        if (StrEqual(_e, _T(".mkv")))
            return TRUE;
        if (StrEqual(_e, _T(".mov")))
            return TRUE;
        if (StrEqual(_e, _T(".wmv")))
            return TRUE;
        if (StrEqual(_e, _T(".flv")))
            return TRUE;
        if (StrEqual(_e, _T(".webm")))
            return TRUE;
        if (StrEqual(_e, _T(".m4v")))
            return TRUE;
        if (StrEqual(_e, _T(".mpg")))
            return TRUE;
        if (StrEqual(_e, _T(".mpeg")))
            return TRUE;
        if (StrEqual(_e, _T(".3gp")))
            return TRUE;
        if (StrEqual(_e, _T(".3g2")))
            return TRUE;
        if (StrEqual(_e, _T(".asf")))
            return TRUE;
        if (StrEqual(_e, _T(".m1v")))
            return TRUE;
        if (StrEqual(_e, _T(".m2v")))
            return TRUE;
        if (StrEqual(_e, _T(".ts")))
            return TRUE;
        if (StrEqual(_e, _T(".m2ts")))
            return TRUE;
        if (StrEqual(_e, _T(".mts")))
            return TRUE;
        if (StrEqual(_e, _T(".m2t")))
            return TRUE;
        if (StrEqual(_e, _T(".mp4v")))
            return TRUE;
        if (StrEqual(_e, _T(".3gp2")))
            return TRUE;
        if (StrEqual(_e, _T(".3gpp")))
            return TRUE;
        if (StrEqual(_e, _T(".mk3d")))
            return TRUE;
        if (StrEqual(_e, _T(".f4v")))
            return TRUE;
        if (StrEqual(_e, _T(".ogm")))
            return TRUE;
        if (StrEqual(_e, _T(".ogv")))
            return TRUE;
        if (StrEqual(_e, _T(".rm")))
            return TRUE;
        if (StrEqual(_e, _T(".rmvb")))
            return TRUE;
        if (StrEqual(_e, _T(".dv")))
            return TRUE;
        if (StrEqual(_e, _T(".mxf")))
            return TRUE;
        if (StrEqual(_e, _T(".ivf")))
            return TRUE;
        if (StrEqual(_e, _T(".evo")))
            return TRUE;
        if (StrEqual(_e, _T(".264")))
            return TRUE;
        if (StrEqual(_e, _T(".video")))
            return TRUE;
        return FALSE;
    }

    // ----------------------------------------------------------------
    // IsAudio — Check if a filename has a recognized audio extension
    //
    // Input:  szFile — filename or path with extension
    // Output: TRUE if the extension matches a known audio format
    // ----------------------------------------------------------------
    BOOL IsAudio(LPCTSTR szFile)
    {
        LPWSTR _e = PathFindExtension(szFile);
        if (StrEqual(_e, _T(".mp3")))
            return TRUE;
        if (StrEqual(_e, _T(".wav")))
            return TRUE;
        if (StrEqual(_e, _T(".m4a")))
            return TRUE;
        if (StrEqual(_e, _T(".ape")))
            return TRUE;
        if (StrEqual(_e, _T(".flac")))
            return TRUE;
        if (StrEqual(_e, _T(".ogg")))
            return TRUE;
        if (StrEqual(_e, _T(".mka")))
            return TRUE;
        if (StrEqual(_e, _T(".opus")))
            return TRUE;
        if (StrEqual(_e, _T(".tak")))
            return TRUE;
        if (StrEqual(_e, _T(".wv")))
            return TRUE;
        if (StrEqual(_e, _T(".mpc")))
            return TRUE;
        return FALSE;
    }

    // ----------------------------------------------------------------
    // IsDocument — Check if a filename has an Office document extension
    //
    // Input:  szFile — filename or path with extension
    // Output: TRUE if the extension matches a known Office format
    // ----------------------------------------------------------------
    BOOL IsDocument(LPCTSTR szFile)
    {
        LPWSTR _e = PathFindExtension(szFile);
        if (StrEqual(_e, _T(".docx")))
            return TRUE;
        if (StrEqual(_e, _T(".pptx")))
            return TRUE;
        if (StrEqual(_e, _T(".xlsx")))
            return TRUE;
        if (StrEqual(_e, _T(".doc")))
            return TRUE;
        if (StrEqual(_e, _T(".ppt")))
            return TRUE;
        if (StrEqual(_e, _T(".xls")))
            return TRUE;
        return FALSE;
    }

    // ----------------------------------------------------------------
    // IsFont — Check if a filename has a font file extension
    //
    // Input:  szFile — filename or path with extension
    // Output: TRUE if the extension matches TTF/OTF/WOFF/WOFF2
    // ----------------------------------------------------------------
    BOOL IsFont(LPCTSTR szFile)
    {
        LPWSTR _e = PathFindExtension(szFile);
        if (StrEqual(_e, _T(".ttf")))
            return TRUE;
        if (StrEqual(_e, _T(".otf")))
            return TRUE;
        if (StrEqual(_e, _T(".woff")))
            return TRUE;
        if (StrEqual(_e, _T(".woff2")))
            return TRUE;
        return FALSE;
    }

    // ================================================================
    // GetLENSTYPE — Map file extension to LENSTYPE constant
    //
    // Performs case-insensitive matching of file extensions to the
    // corresponding LENSTYPE_* constant, covering all supported
    // archive, image, video, audio, document, font, and 3D model
    // formats. Gated sections are enabled/disabled by preprocessor
    // defines (ENABLE_LIBARCHIVE_SUPPORT, ENABLE_WEBP_SUPPORT, etc.).
    //
    // Input:  szExt — lowercase file extension (e.g. L".cbz")
    // Output: LENSTYPE constant, or LENSTYPE_NONE if unrecognized
    // ================================================================
    inline LENSTYPE GetLENSTYPE(LPCTSTR szExt)
    {
        // Comic book archives
        if (StrEqual(szExt, _T(".cbz")))
            return LENSTYPE_CBZ;
        if (StrEqual(szExt, _T(".cb7")))
            return LENSTYPE_CB7;
        if (StrEqual(szExt, _T(".cbt")))
            return LENSTYPE_CBT;
    #ifndef DISABLE_RAR_SUPPORT
        if (StrEqual(szExt, _T(".cbr")))
            return LENSTYPE_CBR;
    #endif

        // Generic archives
        if (StrEqual(szExt, _T(".zip")))
            return LENSTYPE_ZIP;
        if (StrEqual(szExt, _T(".7z")))
            return LENSTYPE_7Z;
        if (StrEqual(szExt, _T(".tar")))
            return LENSTYPE_TAR;
    #ifndef DISABLE_RAR_SUPPORT
        if (StrEqual(szExt, _T(".rar")))
            return LENSTYPE_RAR;
    #endif

        // LibArchive-backed compressed archives
    #ifdef ENABLE_LIBARCHIVE_SUPPORT
        if (StrEqual(szExt, _T(".tar.gz")))
            return LENSTYPE_TAR_GZ;
        if (StrEqual(szExt, _T(".tgz")))
            return LENSTYPE_TAR_GZ;
        if (StrEqual(szExt, _T(".tar.bz2")))
            return LENSTYPE_TAR_BZ2;
        if (StrEqual(szExt, _T(".tbz")))
            return LENSTYPE_TAR_BZ2;
        if (StrEqual(szExt, _T(".tb2")))
            return LENSTYPE_TAR_BZ2;
        if (StrEqual(szExt, _T(".tar.xz")))
            return LENSTYPE_TAR_XZ;
        if (StrEqual(szExt, _T(".txz")))
            return LENSTYPE_TAR_XZ;
        if (StrEqual(szExt, _T(".tar.zst")))
            return LENSTYPE_TAR_ZST;
        if (StrEqual(szExt, _T(".tzst")))
            return LENSTYPE_TAR_ZST;
        if (StrEqual(szExt, _T(".cpio")))
            return LENSTYPE_CPIO;
        if (StrEqual(szExt, _T(".iso")))
            return LENSTYPE_ISO;
        if (StrEqual(szExt, _T(".xar")))
            return LENSTYPE_XAR;
        if (StrEqual(szExt, _T(".ar")))
            return LENSTYPE_AR;
        if (StrEqual(szExt, _T(".deb")))
            return LENSTYPE_DEB;
        if (StrEqual(szExt, _T(".cab")))
            return LENSTYPE_CAB;
    #endif

        // Ebook formats
        if (StrEqual(szExt, _T(".epub")))
            return LENSTYPE_EPUB;
        if (StrEqual(szExt, _T(".mobi")))
            return LENSTYPE_MOBI;
        if (StrEqual(szExt, _T(".azw")))
            return LENSTYPE_AZW;
        if (StrEqual(szExt, _T(".azw3")))
            return LENSTYPE_AZW3;
        if (StrEqual(szExt, _T(".fb2")))
            return LENSTYPE_FB2;

        // Photo archives
        if (StrEqual(szExt, _T(".phz")))
            return LENSTYPE_PHZ;

        // Video formats
        if (StrEqual(szExt, _T(".mp4")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".avi")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mkv")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mov")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".wmv")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".flv")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".webm")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".m4v")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mpg")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mpeg")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".3gp")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".3g2")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".asf")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".m1v")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".m2v")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".ts")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".m2ts")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mts")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".m2t")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mp4v")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".3gp2")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".3gpp")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mk3d")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".f4v")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".ogm")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".ogv")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".rm")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".rmvb")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".dv")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".mxf")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".ivf")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".evo")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".264")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".video")))
            return LENSTYPE_VIDEO;

        // Audio formats
        if (StrEqual(szExt, _T(".mp3")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".wav")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".m4a")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".ape")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".flac")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".ogg")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".mka")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".opus")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".tak")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".wv")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".mpc")))
            return LENSTYPE_AUDIO;

        // Document formats
        if (StrEqual(szExt, _T(".docx")))
            return LENSTYPE_DOCX;
        if (StrEqual(szExt, _T(".pptx")))
            return LENSTYPE_PPTX;
        if (StrEqual(szExt, _T(".xlsx")))
            return LENSTYPE_XLSX;
        if (StrEqual(szExt, _T(".doc")))
            return LENSTYPE_DOC;
        if (StrEqual(szExt, _T(".ppt")))
            return LENSTYPE_PPT;
        if (StrEqual(szExt, _T(".xls")))
            return LENSTYPE_XLS;

        // Font formats
        if (StrEqual(szExt, _T(".ttf")))
            return LENSTYPE_FONT;
        if (StrEqual(szExt, _T(".otf")))
            return LENSTYPE_FONT;
        if (StrEqual(szExt, _T(".woff")))
            return LENSTYPE_FONT;
        if (StrEqual(szExt, _T(".woff2")))
            return LENSTYPE_FONT;

        // PDF format
        if (StrEqual(szExt, _T(".pdf")))
            return LENSTYPE_PDF;

        // Modern image formats
    #ifdef ENABLE_WEBP_SUPPORT
        if (StrEqual(szExt, _T(".webp")))
            return LENSTYPE_WEBP;
    #endif
    #ifdef ENABLE_AVIF_SUPPORT
        if (StrEqual(szExt, _T(".avif")))
            return LENSTYPE_AVIF;
        if (StrEqual(szExt, _T(".avifs")))
            return LENSTYPE_AVIF;
    #endif
    #ifdef ENABLE_HEIF_SUPPORT
        if (StrEqual(szExt, _T(".heic")))
            return LENSTYPE_HEIC;
        if (StrEqual(szExt, _T(".heif")))
            return LENSTYPE_HEIF;
        if (StrEqual(szExt, _T(".heics")))
            return LENSTYPE_HEIC;
        if (StrEqual(szExt, _T(".heifs")))
            return LENSTYPE_HEIF;
        if (StrEqual(szExt, _T(".hif")))
            return LENSTYPE_HEIC;
        if (StrEqual(szExt, _T(".avci")))
            return LENSTYPE_HEIC;
        if (StrEqual(szExt, _T(".avcs")))
            return LENSTYPE_HEIF;
    #endif
    #ifdef ENABLE_JXL_SUPPORT
        if (StrEqual(szExt, _T(".jxl")))
            return LENSTYPE_JXL;
    #endif
        if (StrEqual(szExt, _T(".tif")))
            return LENSTYPE_TIFF;
        if (StrEqual(szExt, _T(".tiff")))
            return LENSTYPE_TIFF;
    #ifdef ENABLE_SVG_SUPPORT
        if (StrEqual(szExt, _T(".svg")))
            return LENSTYPE_SVG;
        if (StrEqual(szExt, _T(".svgz")))
            return LENSTYPE_SVG;
    #endif
    #ifdef ENABLE_RAW_SUPPORT
        // Canon RAW formats
        if (StrEqual(szExt, _T(".cr2")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".cr3")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".crw")))
            return LENSTYPE_RAW;
        // Nikon RAW
        if (StrEqual(szExt, _T(".nef")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".nrw")))
            return LENSTYPE_RAW;
        // Sony RAW
        if (StrEqual(szExt, _T(".arw")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".srf")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".sr2")))
            return LENSTYPE_RAW;
        // Other common RAW formats
        if (StrEqual(szExt, _T(".dng")))
            return LENSTYPE_RAW;  // Adobe Digital Negative
        if (StrEqual(szExt, _T(".orf")))
            return LENSTYPE_RAW;  // Olympus
        if (StrEqual(szExt, _T(".rw2")))
            return LENSTYPE_RAW;  // Panasonic
        if (StrEqual(szExt, _T(".pef")))
            return LENSTYPE_RAW;  // Pentax
        if (StrEqual(szExt, _T(".raf")))
            return LENSTYPE_RAW;  // Fujifilm
        if (StrEqual(szExt, _T(".dcr")))
            return LENSTYPE_RAW;  // Kodak
        if (StrEqual(szExt, _T(".mrw")))
            return LENSTYPE_RAW;  // Minolta
        if (StrEqual(szExt, _T(".x3f")))
            return LENSTYPE_RAW;  // Sigma
        // Samsung
        if (StrEqual(szExt, _T(".srw")))
            return LENSTYPE_RAW;
        // Leica
        if (StrEqual(szExt, _T(".rwl")))
            return LENSTYPE_RAW;
        // Hasselblad
        if (StrEqual(szExt, _T(".3fr")))
            return LENSTYPE_RAW;
        // Phase One
        if (StrEqual(szExt, _T(".iiq")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".cap")))
            return LENSTYPE_RAW;
        // Leaf
        if (StrEqual(szExt, _T(".mos")))
            return LENSTYPE_RAW;
        // Epson
        if (StrEqual(szExt, _T(".erf")))
            return LENSTYPE_RAW;
        // Kodak
        if (StrEqual(szExt, _T(".kdc")))
            return LENSTYPE_RAW;
        if (StrEqual(szExt, _T(".dcr")))
            return LENSTYPE_RAW;
        // Mamiya
        if (StrEqual(szExt, _T(".mef")))
            return LENSTYPE_RAW;
        // Casio
        if (StrEqual(szExt, _T(".bay")))
            return LENSTYPE_RAW;
        // GoPro
        if (StrEqual(szExt, _T(".gpr")))
            return LENSTYPE_RAW;
    #endif

        // Photoshop/Design formats
    #ifdef ENABLE_PSD_SUPPORT
        if (StrEqual(szExt, _T(".psd")))
            return LENSTYPE_PSD;
        if (StrEqual(szExt, _T(".psb")))
            return LENSTYPE_PSD;
    #endif

        // DirectX/Game textures
    #ifdef ENABLE_DDS_SUPPORT
        if (StrEqual(szExt, _T(".dds")))
            return LENSTYPE_DDS;
    #endif

        // HDR image formats
    #ifdef ENABLE_HDR_SUPPORT
        if (StrEqual(szExt, _T(".hdr")))
            return LENSTYPE_HDR;
    #endif
    #ifdef ENABLE_EXR_SUPPORT
        if (StrEqual(szExt, _T(".exr")))
            return LENSTYPE_EXR;
    #endif

        // Netpbm portable image formats
    #ifdef ENABLE_PPM_SUPPORT
        if (StrEqual(szExt, _T(".ppm")))
            return LENSTYPE_PPM;
        if (StrEqual(szExt, _T(".pgm")))
            return LENSTYPE_PPM;
        if (StrEqual(szExt, _T(".pbm")))
            return LENSTYPE_PPM;
        if (StrEqual(szExt, _T(".pnm")))
            return LENSTYPE_PPM;
        if (StrEqual(szExt, _T(".pam")))
            return LENSTYPE_PPM;
        if (StrEqual(szExt, _T(".pfm")))
            return LENSTYPE_PPM;
    #endif

        // Additional video formats
    #ifdef ENABLE_VIDEO_SUPPORT
        if (StrEqual(szExt, _T(".vob")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".divx")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".h264")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".h265")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".hevc")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".av1")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".vp9")))
            return LENSTYPE_VIDEO;
        if (StrEqual(szExt, _T(".y4m")))
            return LENSTYPE_VIDEO;
    #endif

        // Additional audio formats
    #ifdef ENABLE_AUDIO_SUPPORT
        if (StrEqual(szExt, _T(".aac")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".wma")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".aiff")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".aif")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".dsf")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".dff")))
            return LENSTYPE_AUDIO;
        if (StrEqual(szExt, _T(".alac")))
            return LENSTYPE_AUDIO;
    #endif

        // Additional document formats
    #ifdef ENABLE_DOCUMENT_SUPPORT
        if (StrEqual(szExt, _T(".rtf")))
            return LENSTYPE_DOC;
        if (StrEqual(szExt, _T(".odt")))
            return LENSTYPE_DOCX;
        if (StrEqual(szExt, _T(".odp")))
            return LENSTYPE_PPTX;
        if (StrEqual(szExt, _T(".ods")))
            return LENSTYPE_XLSX;
        if (StrEqual(szExt, _T(".xps")))
            return LENSTYPE_DOC;
    #endif

        // eBook formats
        if (StrEqual(szExt, _T(".djvu")))
            return LENSTYPE_DJVU;
        if (StrEqual(szExt, _T(".djv")))
            return LENSTYPE_DJVU;

        // 3D model formats
        if (StrEqual(szExt, _T(".obj")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".stl")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".gltf")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".glb")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".fbx")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".3ds")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".dae")))
            return LENSTYPE_MODEL;
        if (StrEqual(szExt, _T(".ply")))
            return LENSTYPE_MODEL;

        // Additional image formats
        if (StrEqual(szExt, _T(".wmf")))
            return LENSTYPE_WMF;
        if (StrEqual(szExt, _T(".emf")))
            return LENSTYPE_EMF;
        if (StrEqual(szExt, _T(".pcx")))
            return LENSTYPE_PCX;
        if (StrEqual(szExt, _T(".ff")))
            return LENSTYPE_FARBFELD;

        // JPEG 2000 formats
        if (StrEqual(szExt, _T(".jp2")))
            return LENSTYPE_JP2;
        if (StrEqual(szExt, _T(".j2k")))
            return LENSTYPE_JP2;
        if (StrEqual(szExt, _T(".j2c")))
            return LENSTYPE_JP2;
        if (StrEqual(szExt, _T(".jpx")))
            return LENSTYPE_JP2;
        if (StrEqual(szExt, _T(".jpf")))
            return LENSTYPE_JP2;
        if (StrEqual(szExt, _T(".jph")))
            return LENSTYPE_JP2;

        // Vector/PostScript formats
        if (StrEqual(szExt, _T(".eps")))
            return LENSTYPE_EPS;
        if (StrEqual(szExt, _T(".epsf")))
            return LENSTYPE_EPS;
        if (StrEqual(szExt, _T(".ps")))
            return LENSTYPE_EPS;

        // Adobe Illustrator files — most are PDF-based
        if (StrEqual(szExt, _T(".ai")))
            return LENSTYPE_PDF;

        // Game texture formats
        if (StrEqual(szExt, _T(".ktx")))
            return LENSTYPE_KTX;
        if (StrEqual(szExt, _T(".ktx2")))
            return LENSTYPE_KTX;
        if (StrEqual(szExt, _T(".vtf")))
            return LENSTYPE_VTF;

        // Open image editor formats
        if (StrEqual(szExt, _T(".ora")))
            return LENSTYPE_ORA;
        if (StrEqual(szExt, _T(".xcf")))
            return LENSTYPE_XCF;

        // Legacy image formats
        if (StrEqual(szExt, _T(".sgi")))
            return LENSTYPE_SGI;
        if (StrEqual(szExt, _T(".rgb")))
            return LENSTYPE_SGI;
        if (StrEqual(szExt, _T(".rgba")))
            return LENSTYPE_SGI;
        if (StrEqual(szExt, _T(".bw")))
            return LENSTYPE_SGI;
        if (StrEqual(szExt, _T(".xpm")))
            return LENSTYPE_XPM;

        // JPEG XR / HD Photo formats
        if (StrEqual(szExt, _T(".jxr")))
            return LENSTYPE_JXR;
        if (StrEqual(szExt, _T(".wdp")))
            return LENSTYPE_JXR;
        if (StrEqual(szExt, _T(".hdp")))
            return LENSTYPE_JXR;

        return LENSTYPE_NONE;
    }

    BOOL GetImageCountZIP(LPCTSTR cbzFile, int& imagecount, int& filecount)
    {
        imagecount = 0;
        filecount = 0;

        CUnzip _z;
        if (!_z.Open(cbzFile))
            return FALSE;
        // empty zip is still valid
        if (_z.GetItemCount() == 0)
            return TRUE;

        int _i;
        for (_i = 0; _i < _z.GetItemCount(); _i++) {
            if (!_z.GetItem(_i))
                return FALSE;
            // skip dirs
            if (_z.ItemIsDirectory())
                continue;
            if (IsImage(_z.GetItemName()))
                imagecount += 1;
            filecount += 1;
        }
        return TRUE;
    }

    #ifndef DISABLE_RAR_SUPPORT
    BOOL GetImageCountRAR(LPCTSTR cbrFile, int& imagecount, int& filecount)
    {
        imagecount = 0;
        filecount = 0;

        CUnRar cr;
        if (!cr.Open(cbrFile))
            return FALSE;

        // enum solid / skip volumes or encrypted file header archives
        if (cr.IsArchiveVolume() || cr.IsArchiveEncryptedHeaders())
            return FALSE;

        while (cr.ReadItemInfo()) {
            // skip dirs
            if (!cr.IsItemDirectory()) {
                if (IsImage(cr.GetItemName()))
                    imagecount += 1;
                filecount += 1;
            }
            if (!cr.SkipItem())
                return FALSE;
        }
        return TRUE;
    }
    #endif  // DISABLE_RAR_SUPPORT

    static inline BOOL Draw(CImage ci, _In_ HDC hDestDC, _In_ int xDest, _In_ int yDest, _In_ int nDestWidth,
                            _In_ int nDestHeight, _In_ int xSrc, _In_ int ySrc, _In_ int nSrcWidth, _In_ int nSrcHeight,
                            _In_ Gdiplus::InterpolationMode interpolationMode) throw()
    {
        Gdiplus::Bitmap bm((HBITMAP)ci, NULL);
        if (bm.GetLastStatus() != Gdiplus::Ok) {
            return FALSE;
        }

        Gdiplus::Graphics dcDst(hDestDC);
        dcDst.SetInterpolationMode(interpolationMode);

        Gdiplus::Rect destRec(xDest, yDest, nDestWidth, nDestHeight);

        Gdiplus::Status status =
            dcDst.DrawImage(&bm, destRec, xSrc, ySrc, nSrcWidth, nSrcHeight, Gdiplus::Unit::UnitPixel);

        return status == Gdiplus::Ok;
    }

    // ================================================================
    // ThumbnailFromIStream — Decode an image stream to a thumbnail HBITMAP
    //
    // Attempts modern format decoders (WebP, HEIF, AVIF, JXL, PDF, SVG,
    // RAW) in the legacy decoder path (gated by LENSShell_LEGACY_DECODERS),
    // then falls back to CImage (GDI+ — handles JPEG, PNG, BMP, GIF).
    // The resulting bitmap is scaled to fit pThumbSize.
    //
    // Input:  pIs        — seekable IStream containing image data
    //         pThumbSize — requested thumbnail dimensions
    // Output: HBITMAP scaled to pThumbSize, or NULL on failure
    // ================================================================
    HBITMAP ThumbnailFromIStream(IStream* pIs, const LPSIZE pThumbSize)
    {
        ATLASSERT(pIs);

        // Windows 11 Optimization: Try modern image formats first (delay-loaded
        // DLLs) This provides better performance and smaller memory footprint
        HBITMAP hModernBitmap = NULL;

        // LEGACY: Modern formats tried inline. Now handled by Engine pipeline.
        // Only compiled when LENSShell_LEGACY_DECODERS is defined.
    #if defined(LENSShell_LEGACY_DECODERS) && defined(ENABLE_WEBP_SUPPORT)
        {
            // Read stream data for format detection
            STATSTG stat = {};
            if (SUCCEEDED(pIs->Stat(&stat, STATFLAG_NONAME))) {
                size_t streamSize = static_cast<size_t>(stat.cbSize.QuadPart);

                if (streamSize > 0 && streamSize < LENSMEM_MAXBUFFER_SIZE) {
                    std::vector<BYTE> buffer(static_cast<size_t>(streamSize));

                    LARGE_INTEGER li = {};
                    pIs->Seek(li, STREAM_SEEK_SET, NULL);

                    ULONG bytesRead = 0;
                    if (SUCCEEDED(pIs->Read(buffer.data(), static_cast<ULONG>(streamSize), &bytesRead))
                        && bytesRead == streamSize) {
                        // Try modern formats in order of likelihood and efficiency

                        // Try WebP decoder (delay-loaded)
                        if (ExplorerLens::WebPDecoder::IsWebPFormat(buffer.data(), streamSize)) {
                            if (SUCCEEDED(ExplorerLens::WebPDecoder::DecodeToHBITMAP(buffer.data(), streamSize,
                                                                                     &hModernBitmap))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }

        #ifdef ENABLE_HEIF_SUPPORT
                        // Try HEIF/HEIC decoder (Windows native via WIC)
                        if (ExplorerLens::HEIFDecoderNative::IsHEIFFormat(buffer.data(), streamSize)) {
                            bool isDarkMode = DarkMode::IsSystemDarkMode();
                            if (SUCCEEDED(ExplorerLens::HEIFDecoderNative::DecodeToHBITMAP(
                                    buffer.data(), streamSize, &hModernBitmap, isDarkMode))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_AVIF_SUPPORT
                        // Try AVIF decoder (WIC-based, Windows 10+ with AV1 extension)
                        if (ExplorerLens::AVIFDecoder::IsAVIFFormat(buffer.data(), streamSize)) {
                            if (SUCCEEDED(ExplorerLens::AVIFDecoder::DecodeToHBITMAP(buffer.data(), streamSize,
                                                                                     &hModernBitmap))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_JXL_SUPPORT
                        // Try JPEG XL decoder (delay-loaded)
                        if (ExplorerLens::JXLDecoder::IsJXLFormat(buffer.data(), streamSize)) {
                            if (SUCCEEDED(ExplorerLens::JXLDecoder::DecodeToHBITMAP(buffer.data(), streamSize,
                                                                                    &hModernBitmap))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_PDF_SUPPORT
                        // Try PDF decoder (Windows.Data.Pdf)
                        if (ExplorerLens::PDFDecoder::IsPDFFormat(buffer.data(), streamSize)) {
                            // Render first page as thumbnail
                            if (SUCCEEDED(ExplorerLens::PDFDecoder::DecodeToHBITMAP(
                                    buffer.data(), streamSize, &hModernBitmap, pThumbSize ? pThumbSize->cx : 256,
                                    pThumbSize ? pThumbSize->cy : 256))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_SVG_SUPPORT
                        // Try SVG decoder (WIC-based)
                        if (ExplorerLens::SVGDecoder::IsSVGFormat(buffer.data(), streamSize)) {
                            if (SUCCEEDED(ExplorerLens::SVGDecoder::DecodeToHBITMAP(
                                    buffer.data(), streamSize, &hModernBitmap, pThumbSize ? pThumbSize->cx : 256,
                                    pThumbSize ? pThumbSize->cy : 256))) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_RAW_SUPPORT
                        // Try RAW camera decoder (LibRaw-based)
                        {
                            // RAW formats: Canon (CR2, CR3, CRW), Nikon (NEF, NRW), Sony
                            // (ARW, SRF, SR2), Olympus (ORF), Panasonic (RW2), Pentax (PEF),
                            // Fujifilm (RAF), DNG, etc.
                            HRESULT hrRaw = ExplorerLens::RAWDecoder::DecodeToHBITMAP(
                                buffer.data(), streamSize, &hModernBitmap, pThumbSize ? pThumbSize->cx : 512,
                                pThumbSize ? pThumbSize->cy : 512);
                            if (SUCCEEDED(hrRaw)) {
                                return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
                            }
                        }
        #endif

        #ifdef ENABLE_RAW_SUPPORT
                        // Try RAW decoder (WIC with Camera Codec Pack)
                        // Note: Requires Windows Camera Codec Pack or manufacturer codecs
                        if (RawDecoder::IsRAWFormat(nullptr, buffer.data(), streamSize)) {
                            HBITMAP hRawBitmap = RawDecoder::DecodeToHBITMAP(pIs, pThumbSize ? pThumbSize->cx : 256);
                            if (hRawBitmap) {
                                return ScaleBitmapToThumbnail(hRawBitmap, pThumbSize);
                            }
                        }
        #endif

                        // Reset stream for fallback to CImage
                        pIs->Seek(li, STREAM_SEEK_SET, NULL);
                    }
                }
            }
        }
    #endif  // ENABLE_WEBP_SUPPORT		// Fallback to standard CImage (handles
        // JPEG, PNG, BMP, GIF)
        CImage ci;  // uses gdi+ internally
        if (S_OK != ci.Load(pIs))
            return NULL;

        return ScaleCImageToThumbnail(ci, pThumbSize);
    }

    // Helper: Scale HBITMAP to thumbnail size (for WebP/AVIF)
    HBITMAP ScaleBitmapToThumbnail(HBITMAP hSourceBitmap, const LPSIZE pThumbSize)
    {
        if (!hSourceBitmap)
            return NULL;

        // Get source bitmap dimensions
        BITMAP bm;
        if (!GetObject(hSourceBitmap, sizeof(bm), &bm)) {
            DeleteObject(hSourceBitmap);
            return NULL;
        }

        int tw = bm.bmWidth;
        int th = bm.bmHeight;
        float cx = (float)pThumbSize->cx;
        float cy = (float)pThumbSize->cy;
        float rx = cx / (float)tw;
        float ry = cy / (float)th;

        // If doesn't need scaling, return as-is
        if (rx >= 1 && ry >= 1) {
            return hSourceBitmap;
        }

        // Create scaled bitmap
        tw = (int)(min(rx, ry) * tw);
        th = (int)(min(rx, ry) * th);

        HDC hdcScreen = GetDC(NULL);
        HDC hdcSrc = CreateCompatibleDC(hdcScreen);
        HDC hdcDst = CreateCompatibleDC(hdcScreen);

        HBITMAP hScaledBitmap = CreateCompatibleBitmap(hdcScreen, tw, th);

        HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, hSourceBitmap);
        HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hScaledBitmap);

        // High-quality scaling
        SetStretchBltMode(hdcDst, HALFTONE);
        SetBrushOrgEx(hdcDst, 0, 0, NULL);

        // Use dark mode aware background
        COLORREF bgColor = DarkMode::GetThumbnailBackgroundColor();
        HBRUSH hBrush = CreateSolidBrush(bgColor);
        RECT rc = {0, 0, tw, th};
        FillRect(hdcDst, &rc, hBrush);
        DeleteObject(hBrush);

        StretchBlt(hdcDst, 0, 0, tw, th, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        SelectObject(hdcSrc, hOldSrc);
        SelectObject(hdcDst, hOldDst);
        DeleteDC(hdcSrc);
        DeleteDC(hdcDst);
        ReleaseDC(NULL, hdcScreen);
        DeleteObject(hSourceBitmap);

        return hScaledBitmap;
    }

    // Helper: Scale CImage to thumbnail size (refactored from original code)
    HBITMAP ScaleCImageToThumbnail(CImage& ci, const LPSIZE pThumbSize)
    {
        // check size
        int tw = ci.GetWidth();
        int th = ci.GetHeight();
        float cx = (float)pThumbSize->cx;
        float cy = (float)pThumbSize->cy;
        float rx = cx / (float)tw;
        float ry = cy / (float)th;

        // if bigger size
        if ((rx < 1) || (ry < 1)) {
            CDC hdcNew = ::CreateCompatibleDC(NULL);
            if (hdcNew.IsNull())
                return NULL;

            hdcNew.SetStretchBltMode(HALFTONE);
            hdcNew.SetBrushOrg(0, 0, NULL);
            // Use std::min to avoid macro conflicts
            tw = (int)((std::min)(rx, ry) * tw);
            th = (int)((std::min)(rx, ry) * th);

            CBitmap hbmpNew;
            hbmpNew.CreateCompatibleBitmap(ci.GetDC(), tw, th);
            ci.ReleaseDC();  // don't forget!
            if (hbmpNew.IsNull())
                return NULL;

            HBITMAP hbmpOld = hdcNew.SelectBitmap(hbmpNew);
            // Use dark mode aware background color
            COLORREF bgColor = DarkMode::GetThumbnailBackgroundColor();
            hdcNew.FillSolidRect(0, 0, tw, th, bgColor);

            Draw(ci, hdcNew, 0, 0, tw, th, 0, 0, ci.GetWidth(), ci.GetHeight(),
                 Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic);  // too late for error checks
            if (m_showIcon)
                DrawIcon(hdcNew, 0, 0, zipIcon);

            hdcNew.SelectBitmap(hbmpOld);

            return hbmpNew.Detach();
        }

        return ci.Detach();
    }

    ////unused
    // HBITMAP ThumbnailFromBuffer(LPCBYTE pBuf, const ULONG dwBufSize, const
    // LPSIZE pThumbSize)
    //{
    //	HBITMAP hBmp = NULL;
    //	IStream* pIs = NULL;
    //	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBufSize);
    //	if (hG)
    //	{
    //		if (S_OK==CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs))
    //		{
    //			ULONG br;
    //			if (S_OK==pIs->Write(pBuf, dwBufSize, &br))//transfer
    // buffer data
    //			{
    //				if (br==dwBufSize)
    // hBmp=ThumbnailFromIStream(pIs, pThumbSize);
    //			}
    //		}
    //	}
    //	GlobalFree(hG);
    //	pIs->Release();
    // return hBmp;
    // }

    #ifndef DISABLE_RAR_SUPPORT
    __int64 FindThumbnailSortRAR(LPCTSTR pszFile)
    {
        CUnRar _r;
        if (!_r.Open(pszFile))
            return -1;
        // skip solid (long processing time), volumes or encrypted file headers
        if (_r.IsArchiveSolid() || _r.IsArchiveVolume() || _r.IsArchiveEncryptedHeaders())
            return -1;

        UINT64 _ps, _us;  // my speed optimization?
        CString prevname;
        __int64 thumbindex = -1;
        __int64 i = -1;  // start at none (-1)

        while (_r.ReadItemInfo()) {
            i += 1;
            _ps = _r.GetItemPackedSize64();
            _us = _r.GetItemUnpackedSize64();

            // skip directory/emtpy file/bigger than 32mb
            if (_r.IsItemDirectory() || (_us > LENSMEM_MAXBUFFER_SIZE) || (_ps == 0) || (_us == 0)) {
                _r.SkipItem();
                continue;
            }

            // take only index of first alphabetical name
            if (IsImage(_r.GetItemName())) {
                // can't compare empty string
                if (prevname.IsEmpty())
                    prevname = _r.GetItemName();
                if (thumbindex < 0)
                    thumbindex = i;  // assign thumbindex if already sorted
                // sort by name
                if (-1 == StrCmpLogicalW(_r.GetItemName(), prevname)) {
                    thumbindex = i;
                    prevname = _r.GetItemName();
                }
            }
            _r.SkipItem();  // don't forget
        }

        return thumbindex;
    }
    #endif  // DISABLE_RAR_SUPPORT

  public:
    void LoadRegistrySettings()
    {
        DWORD _d;
        CRegKey _rk;
        if (ERROR_SUCCESS == _rk.Open(HKEY_CURRENT_USER, LENS_APP_KEY, KEY_READ)) {
            if (ERROR_SUCCESS == _rk.QueryDWORDValue(_T("NoSort"), _d))
                m_bSort = (_d == FALSE);
            if (ERROR_SUCCESS == _rk.QueryDWORDValue(_T("ShowIcon"), _d))
                m_showIcon = (_d == TRUE);
        }
    }

    // Get current file path (for Engine integration)
    const wchar_t* GetFilePath() const
    {
        return m_LENSFile;
    }

    // Get detected file type (for PropertyStore)
    LENSTYPE GetFileType() const
    {
        return m_LENSTYPE;
    }

    #ifdef _DEBUG
  public:
    void debug_SetSort(BOOL bS = TRUE)
    {
        m_bSort = bS;
    }
    #endif

};  // class _CLENSArchive

}  // namespace __LENS

#endif  //_LENSARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_
