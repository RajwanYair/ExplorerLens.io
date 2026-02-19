///////////////////////////////////////////////
// v8.4.0 (Sprint 175+)
//////////////////////////////////////////////
// CCBXShell functionality implementation

#ifndef _CBXARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_
#define _CBXARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_

#ifndef STRICT
#define STRICT
#endif

#include <algorithm>  // For std::min, std::max
#include <shlObj.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "DarkModeHelper.h"
#include "ModernCppHelper.h"
#include "thumbnail_cache.h"   // Performance caching (v5.1.0)
#include "thumbnail_collage.h" // Multi-page collage (v5.1.0)

// ============================================================================
// LEGACY DECODERS (v5.0 era) — DEPRECATED as of v6.2.0
// All formats are now handled by Engine pipeline (ThumbnailPipeline → DecoderRegistry).
// Legacy decoders are gated behind CBXSHELL_LEGACY_DECODERS and excluded from build by default.
// To re-enable for debugging: add CBXSHELL_LEGACY_DECODERS to Preprocessor Definitions.
// ============================================================================
#ifdef CBXSHELL_LEGACY_DECODERS
// Modern image format support (v5.0+)
#include "webp_decoder.h" // WebP support via libwebp
#ifdef ENABLE_HEIF_SUPPORT
#include "heif_decoder_native.h" // HEIF/HEIC support using Windows Imaging Component
#endif
#ifdef ENABLE_AVIF_SUPPORT
#include "avif_decoder.h" // AVIF support using Windows Imaging Component
#endif
#ifdef ENABLE_JXL_SUPPORT
#include "jxl_decoder.h" // JPEG XL support via libjxl 0.11.1
#endif
#ifdef ENABLE_RAW_SUPPORT
#include "raw_decoder.h" // RAW camera support via Windows WIC
#endif
#ifdef ENABLE_PDF_SUPPORT
#include "pdf_decoder.h" // PDF thumbnail extraction
#endif
#ifdef ENABLE_SVG_SUPPORT
#include "svg_decoder.h" // SVG rendering via WIC
#endif
#ifdef ENABLE_VIDEO_SUPPORT
#include "video_thumbnail.h" // Video frame extraction using DirectShow
#endif
#ifdef ENABLE_AUDIO_SUPPORT
#include "audio_thumbnail.h" // Audio album art and waveform (Sprint 8)
#endif
#ifdef ENABLE_DOCUMENT_SUPPORT
#include "document_thumbnail.h" // Office document thumbnails (Sprint 9 Phase 1)
#endif
#ifdef ENABLE_FONT_SUPPORT
#include "font_preview.h" // Font file previews (Sprint 9 Phase 1)
#endif
#endif // CBXSHELL_LEGACY_DECODERS

#ifdef ENABLE_LIBARCHIVE_SUPPORT
#include "libarchive_wrapper.h" // Advanced archive support (TAR, ISO, CPIO, XAR, AR)
#endif

// ATL headers
#include <atlimage.h>
#include <atlfile.h>

// WTL headers
#include <atlgdi.h>

// RAR support disabled for 100% static linking - defined in vcxproj
// #define DISABLE_RAR_SUPPORT

// UnRAR includes
#ifndef DISABLE_RAR_SUPPORT
#include "unrar.h"
#ifdef _WIN64
#pragma comment(lib, "unrar64.lib")
#else
#pragma comment(lib, "unrar.lib")
#endif
#endif

#include "unzip.h"
#include <string>

#define CBXMEM_MAXBUFFER_SIZE 33554432 // 32mb
#define CBXTYPE int
#define CBXTYPE_NONE 0
#define CBXTYPE_ZIP 1
#define CBXTYPE_CBZ 2
#define CBXTYPE_EPUB 5
#define CBXTYPE_7Z 6
#define CBXTYPE_CB7 7
#define CBXTYPE_TAR 8
#define CBXTYPE_CBT 9
#define CBXTYPE_MOBI 10
#define CBXTYPE_FB2 11
#define CBXTYPE_AZW 12
#define CBXTYPE_AZW3 13
#define CBXTYPE_PHZ 14

// Modern compression formats (v4.6+)
#define CBXTYPE_BZIP2 15 // .bz2 BZIP2 compressed archives
#define CBXTYPE_ZSTD 16	 // .zst Zstandard compressed archives
#define CBXTYPE_LZMA 17	 // .xz/.lzma LZMA compressed archives

// Media and document formats (v5.0+)
#define CBXTYPE_VIDEO 18 // Video files (.mp4, .avi, .mkv, etc.)
#define CBXTYPE_PDF 19	 // PDF documents
#define CBXTYPE_AUDIO 20 // Audio files (.mp3, .flac, .wav, etc.)

// Extended format support (v8.4+)
#define CBXTYPE_LZ4 21	// .lz4 LZ4 compressed archives
#define CBXTYPE_DJVU 22 // .djvu/.djv DjVu scanned documents
#define CBXTYPE_CHM 23	// .chm Compiled HTML Help
#define CBXTYPE_ODT 24	// .odt OpenDocument Text
#define CBXTYPE_ODP 25	// .odp OpenDocument Presentation

// Document formats (v5.0+)
#define CBXTYPE_DOCX 60 // .docx Microsoft Word
#define CBXTYPE_PPTX 61 // .pptx Microsoft PowerPoint
#define CBXTYPE_XLSX 62 // .xlsx Microsoft Excel
#define CBXTYPE_DOC 63	// .doc Legacy Microsoft Word
#define CBXTYPE_PPT 64	// .ppt Legacy Microsoft PowerPoint
#define CBXTYPE_XLS 65	// .xls Legacy Microsoft Excel

// Font formats (v5.0+)
#define CBXTYPE_FONT 70 // .ttf/.otf/.woff/.woff2 Font files

// LibArchive formats (v5.2+)
#define CBXTYPE_TAR_GZ 26  // .tar.gz / .tgz
#define CBXTYPE_TAR_BZ2 27 // .tar.bz2 / .tbz
#define CBXTYPE_TAR_XZ 28  // .tar.xz / .txz
#define CBXTYPE_TAR_ZST 29 // .tar.zst
#define CBXTYPE_CPIO 30	   // .cpio
#define CBXTYPE_ISO 50	   // .iso ISO 9660 CD/DVD images
#define CBXTYPE_XAR 51	   // .xar macOS archives
#define CBXTYPE_AR 52	   // .ar Unix archives
#define CBXTYPE_DEB 53	   // .deb Debian packages
#define CBXTYPE_CAB 54	   // .cab Microsoft Cabinet

// Video formats (v5.0+) - Deprecated individual types, use CBXTYPE_VIDEO
// #define CBXTYPE_MP4 31	// .mp4 MPEG-4 video - use CBXTYPE_VIDEO
// #define CBXTYPE_MKV 32	// .mkv Matroska video - use CBXTYPE_VIDEO
// #define CBXTYPE_AVI 33	// .avi Audio Video Interleave - use CBXTYPE_VIDEO
// #define CBXTYPE_MOV 34	// .mov QuickTime movie - use CBXTYPE_VIDEO
// #define CBXTYPE_WEBM 35 // .webm WebM video - use CBXTYPE_VIDEO

// Modern image formats (standalone - v5.0+)
#define CBXTYPE_WEBP 40 // .webp Google WebP image
#define CBXTYPE_AVIF 41 // .avif AV1 Image Format
#define CBXTYPE_HEIC 42 // .heic High Efficiency Image Format
#define CBXTYPE_HEIF 43 // .heif HEIF container
#define CBXTYPE_JXL 44	// .jxl JPEG XL
#define CBXTYPE_TIFF 45 // .tif/.tiff Tagged Image File Format
#define CBXTYPE_SVG 46	// .svg Scalable Vector Graphics
#define CBXTYPE_RAW 47	// .dng/.cr2/.cr3/.nef/.arw Camera RAW formats

// Professional/specialty image formats (v6.1+)
#define CBXTYPE_PSD 48	// .psd/.psb Adobe Photoshop
#define CBXTYPE_DDS 49	// .dds DirectDraw Surface (game textures)
#define CBXTYPE_HDR 55	// .hdr Radiance RGBE HDR
#define CBXTYPE_EXR 56	// .exr OpenEXR HDR image
#define CBXTYPE_PPM 57	// .ppm/.pgm/.pbm/.pnm Netpbm portable formats

// Standalone image formats (v7.0+)
#define CBXTYPE_ICO 58	// .ico/.cur Windows icon/cursor
#define CBXTYPE_QOI 59	// .qoi Quite OK Image format
#define CBXTYPE_TGA 75	// .tga Targa image format
#define CBXTYPE_BMP 76	// .bmp/.dib Windows bitmap
#define CBXTYPE_GIF 77	// .gif Graphics Interchange Format

// 3D model formats (v7.0+)
#define CBXTYPE_MODEL 80	// .obj/.stl/.fbx/.gltf/.glb 3D model files

// Document formats (v7.0+) — generic category
#define CBXTYPE_DOCUMENT 81	// Generic document type (DOCX/PPTX/etc. fall here if not specific)

// Additional image formats (v8.4+, Sprint 180)
#define CBXTYPE_WMF 82	// .wmf Windows Metafile
#define CBXTYPE_EMF 83	// .emf Enhanced Metafile
#define CBXTYPE_PCX 84	// .pcx ZSoft PCX image
#define CBXTYPE_FARBFELD 85	// .ff Farbfeld image
#define CBXTYPE_JP2 86	// .jp2/.j2k/.j2c/.jpx JPEG 2000 (Sprint 181)

// Image format identifiers (for detection within archives)
#define IMGTYPE_UNKNOWN 0
#define IMGTYPE_BMP 1
#define IMGTYPE_GIF 2
#define IMGTYPE_JPG 3
#define IMGTYPE_PNG 4
#define IMGTYPE_TIF 5
#define IMGTYPE_WEBP 6
#define IMGTYPE_AVIF 7
#define IMGTYPE_HEIC 8
#define IMGTYPE_HEIF 9
#define IMGTYPE_JXL 10
#define IMGTYPE_ICO 11
#define IMGTYPE_PSD 12
#define IMGTYPE_SVG 13
#define IMGTYPE_DDS 14
#define IMGTYPE_TGA 15
#define IMGTYPE_PCX 16

#define CBX_APP_KEY _T("Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}")

namespace __cbx
{

	// unused
	// template <class T> class CBuffer
	//{
	// public:
	//	CBuffer(){m_buf=NULL;}
	//	virtual ~CBuffer(){ ::CoTaskMemFree(m_buf); m_buf=NULL;}
	// public:
	//	T* Allocate(SIZE_T s, BOOL bAutozero=FALSE)
	//	{
	//		m_buf=::CoTaskMemAlloc(s*sizeof(T));//COM compatible //'new' throws
	//		if (m_buf && bAutozero) SecureZeroMemory(m_buf, s*sizeof(T));
	//	return (T*)m_buf;
	//	}
	//	operator LPVOID () {return m_buf;}
	//	operator T* () {return (T*)m_buf;}
	// private:
	//	LPVOID m_buf;
	//};
	// typedef CBuffer<BYTE> CByteBuffer;

	class CUnzip
	{
	public:
		CUnzip() { hz = NULL; }
		virtual ~CUnzip() { ::CloseZip(hz); }

	public:
		bool Open(LPCTSTR zfile)
		{
			if (zfile == NULL)
				return false;
			HZIP temp_hz = ::OpenZip(zfile, NULL); // try new
			if (temp_hz == NULL)
				return false;
			Close(); // close old
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

		bool UnzipItemToMembuffer(int index, void *z, unsigned int len)
		{
			zr = ::UnzipItem(hz, index, z, len);
			return (ZR_OK == zr);
		}

		void Close()
		{
			CloseZip(hz);
			hz = NULL; // critical!
		}

		inline BOOL ItemIsDirectory() { return (BOOL)(CUnzip::GetItemAttributes() & 0x0010); }
		int GetItemCount() const { return maindirEntry.index; }
		long GetItemPackedSize() const { return ZipEntry.comp_size; }
		long GetItemUnpackedSize() const { return ZipEntry.unc_size; }
		DWORD GetItemAttributes() const { return ZipEntry.attr; }
		LPCTSTR GetItemName() { return ZipEntry.name; }

	private:
		ZIPENTRY ZipEntry, maindirEntry;
		HZIP hz;
		ZRESULT zr;
	};

#ifndef DISABLE_RAR_SUPPORT
	// unrar wrapper
	typedef const RARHeaderDataEx *LPCRARHeaderDataEx;
	typedef const RAROpenArchiveDataEx *LPCRAROpenArchiveDataEx;

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
		BOOL Open(LPCTSTR rarfile, BOOL bListingOnly = TRUE, char *cmtBuf = NULL, UINT cmtBufSize = 0, char *password = NULL)
		{
			if (m_harc)
				return FALSE; // must close old first

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

		inline LPCRAROpenArchiveDataEx GetArchiveInfo() { return &m_arcinfo; }
		inline LPCRARHeaderDataEx GetItemInfo() { return &m_iteminfo; }
		inline void SetPassword(char *password) { RARSetPassword(m_harc, password); }
		inline int GetLastError() { return m_ret; }

		// inline UINT GetArchiveFlags() {return m_arcinfo.Flags;}
		// inline UINT GetItemFlags(){return m_iteminfo.Flags;}
		inline BOOL IsArchiveVolume() { return (BOOL)(m_arcinfo.Flags & 0x0001); }
		inline BOOL IsArchiveComment() { return (BOOL)(m_arcinfo.Flags & 0x0002); }
		inline BOOL IsArchiveLocked() { return (BOOL)(m_arcinfo.Flags & 0x0004); }
		inline BOOL IsArchiveSolid() { return (BOOL)(m_arcinfo.Flags & 0x0008); }
		inline BOOL IsArchivePartN() { return (BOOL)(m_arcinfo.Flags & 0x0010); }
		inline BOOL IsArchiveSigned() { return (BOOL)(m_arcinfo.Flags & 0x0020); }
		inline BOOL IsArchiveRecoveryRecord() { return (BOOL)(m_arcinfo.Flags & 0x0040); }
		inline BOOL IsArchiveEncryptedHeaders() { return (BOOL)(m_arcinfo.Flags & 0x0080); }
		inline BOOL IsArchiveFirstVolume() { return (BOOL)(m_arcinfo.Flags & 0x0100); }

		BOOL ReadItemInfo()
		{
			SecureZeroMemory(&m_iteminfo, sizeof(RARHeaderDataEx));
			m_ret = RARReadHeaderEx(m_harc, &m_iteminfo);
			return (m_ret == 0);
		}

		inline BOOL IsItemDirectory() { return ((m_iteminfo.Flags & 0x00E0) == 0x00E0); }

		inline LPCTSTR GetItemName()
		{
#ifdef UNICODE
			return (LPCTSTR)(GetItemInfo()->FileNameW);
#else
			return (LPCTSTR)CUnRar::GetItemInfo()->FileName;
#endif
		}

		// MAKEUINT64
		inline UINT64 GetItemPackedSize64() { return ((((UINT64)m_iteminfo.PackSizeHigh) << 32) | m_iteminfo.PackSize); }
		inline UINT64 GetItemUnpackedSize64() { return ((((UINT64)m_iteminfo.UnpSizeHigh) << 32) | m_iteminfo.UnpSize); }

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
			UINT64 _i; // 0 skips?
			for (_i = 0; _i < si; _i++)
			{
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
			if (m_pIs)
			{
				ULONG br = 0;
				if (S_OK == m_pIs->Write(pBuf, dwBufSize, &br))
					if (br == dwBufSize)
						return 1;
			}
			return -1;
		}

		void SetIStream(IStream *pIs)
		{
			_ASSERTE(pIs);
			m_pIs = pIs;
		}

	private:
		HANDLE m_harc;
		RARHeaderDataEx m_iteminfo;
		RAROpenArchiveDataEx m_arcinfo;
		int m_ret;
		IStream *m_pIs;
		void _init()
		{
			m_harc = NULL;
			m_pIs = NULL;
			m_ret = 0;
			SecureZeroMemory(&m_arcinfo, sizeof(RAROpenArchiveDataEx));
			SecureZeroMemory(&m_iteminfo, sizeof(RARHeaderDataEx));
		}

		// raw callback function
	public:
		static int PASCAL __rarCallbackProc(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2)
		{
			CUnRar *_pc = (CUnRar *)UserData;
			return _pc->CallbackProc(msg, UserData, P1, P2);
		}
	};
#endif // DISABLE_RAR_SUPPORT

	class CCBXArchive
	{

	public:
		CCBXArchive()
		{
			m_bSort = TRUE; // default
			// GetRegSettings();
			m_thumbSize.cx = m_thumbSize.cy = 0;
			m_cbxType = CBXTYPE_NONE;
			m_pIs = NULL;
		}

		virtual ~CCBXArchive()
		{
			if (m_pIs)
			{
				m_pIs->Release();
				m_pIs = NULL;
			}
		}

	public:
		////////////////////////////////////////
		// IPersistFile::Load
		HRESULT OnLoad(LPCOLESTR wszFile)
		{
			// ATLTRACE("IPersistFile::Load\n");
#ifndef UNICODE
			USES_CONVERSION;
			m_cbxFile = OLE2T((WCHAR *)wszFile);
#else
			m_cbxFile = wszFile;
#endif
			// Modern C++17: Use filesystem for extension detection
			fs::path filePath(wszFile);
			std::wstring ext = ModernCpp::GetFileExtensionLower(filePath);
			m_cbxType = GetCBXType(ext.c_str());
			return S_OK;
		}

		////////////////////////////////////
		// IExtractImage::GetLocation(LPWSTR pszPathBuffer,	DWORD cchMax, DWORD *pdwPriority, const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags)
		HRESULT OnGetLocation(const SIZE *prgSize, DWORD *pdwFlags)
		{
			// ATLTRACE("IExtractImage2::GetLocation\n");
			m_thumbSize.cx = prgSize->cx;
			m_thumbSize.cy = prgSize->cy;
			*pdwFlags |= (IEIFLAG_CACHE | IEIFLAG_REFRESH); // cache thumbnails
															// if (*pdwFlags & IEIFLAG_ASYNC) return E_PENDING;//Windows XP and earlier
#ifdef _DEBUG
			if (*pdwFlags & IEIFLAG_ASYNC)
				ATLTRACE("\nIExtractImage::GetLocation : IEIFLAG_ASYNC flag set\n");
#endif
			return NOERROR;
		}

		////////////////////////////////////
		// IExtractImage2::GetDateStamp(FILETIME *pDateStamp)
		HRESULT OnGetDateStamp(FILETIME *pDateStamp)
		{
			// ATLTRACE("IExtractImage2::GetDateStamp\n");

			// Modern C++17: Use filesystem for file time
			fs::path filePath(m_cbxFile.operator LPCTSTR());
			auto modTime = ModernCpp::GetFileModificationTime(filePath);

			if (modTime.has_value())
			{
				*pDateStamp = ModernCpp::FileTimeToWin32(*modTime);
				return NOERROR;
			}

			// Fallback to legacy API if filesystem fails
			FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
			CAtlFile _f;
			if (S_OK != _f.Create(m_cbxFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0))
				return E_FAIL;
			if (!GetFileTime(_f, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime))
				return E_FAIL;
			*pDateStamp = ftLastWriteTime;
			return NOERROR;
		}

		void ReplaceStringInPlace(std::string &subject, const std::string &search,
								  const std::string &replace)
		{
			size_t pos = 0;
			while ((pos = subject.find(search, pos)) != std::string::npos)
			{
				subject.replace(pos, search.length(), replace);
				pos += replace.length();
			}
		}

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
			for (i = 0; i < j; i++)
			{
				if (!_z.GetItem(i))
					break;
				if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
					continue;

				std::string name = T2A(_z.GetItemName());

				if (_stricmp(name.c_str(), "META-INF/container.xml") == 0)
				{
					// Extract container.xml and retrieve rootfile location

					HGLOBAL hGContainer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
					if (hGContainer)
					{
						bool b = false;
						LPVOID pBuf = ::GlobalLock(hGContainer);
						if (pBuf)
							b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

						if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR)
						{
							if (b)
							{
								xmlContent = (char *)pBuf;

								posStart = xmlContent.find("rootfile ");

								if (posStart == std::string::npos)
								{
									break;
								}

								posStart = xmlContent.find("full-path=\"", posStart);

								if (posStart == std::string::npos)
								{
									break;
								}

								posStart += 11;
								posEnd = xmlContent.find("\"", posStart);

								rootfile = xmlContent.substr(posStart, posEnd - posStart);

								break;
							}
						}
						// GlobalFree(hGContainer);//autofreed
					}
				}
			}

			return rootfile;
		}

		////////////////////////////////////
		// Helper: Extract multiple images from ZIP archive for collage (Sprint C2)
		std::vector<HBITMAP> ExtractMultipleImagesFromZIP(CUnzip &_z, int maxImages)
		{
			std::vector<HBITMAP> images;
			std::vector<int> imageIndices;

			// Find all image files
			int j = _z.GetItemCount();
			CString prevname;

			for (int i = 0; i < j && imageIndices.size() < (size_t)maxImages; i++)
			{
				if (!_z.GetItem(i))
					break;
				if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
					continue;
				if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
					continue;

				if (IsImage(_z.GetItemName()))
				{
					if (m_bSort)
					{
						// Collect for sorting
						imageIndices.push_back(i);
						if (prevname.IsEmpty())
							prevname = _z.GetItemName();
						// Take only first alphabetical names
						if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname))
						{
							imageIndices.back() = i;
							prevname = _z.GetItemName();
						}
					}
					else
					{
						// No sort, just collect first N
						imageIndices.push_back(i);
					}
				}
			}

			// Extract each image
			for (int idx : imageIndices)
			{
				if (!_z.GetItem(idx))
					continue;

				HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
				if (hG)
				{
					bool b = false;
					LPVOID pBuf = ::GlobalLock(hG);
					if (pBuf)
						b = _z.UnzipItemToMembuffer(idx, pBuf, _z.GetItemUnpackedSize());

					if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR)
					{
						if (b)
						{
							IStream *pIs = NULL;
							if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM *)&pIs))
							{
								HBITMAP hBmp = ThumbnailFromIStream(pIs, &m_thumbSize);
								if (hBmp)
								{
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

		////////////////////////////////////
		// IExtractImage::Extract(HBITMAP* phBmpThumbnail)
		HRESULT OnExtract(HBITMAP *phBmpThumbnail)
		{
			*phBmpThumbnail = NULL;
			// ATLTRACE("IExtractImage::Extract\n");

			// Thumbnail caching for 10-100x performance boost (v5.1.0)
			static bool cacheInitialized = false;
			if (!cacheInitialized)
			{
				DarkThumbs::ThumbnailCache::Initialize();
				cacheInitialized = true;
			}

			// Get file metadata for cache key
			__int64 fileSize = 0;
			GetFileSizeCrt(m_cbxFile, fileSize);

			FILETIME lastModified = {0};
			OnGetDateStamp(&lastModified);

			std::wstring archivePath(m_cbxFile.operator LPCTSTR());
			// Include archive type and requested size in cache key for better cache hits
			WCHAR imageName[64];
			swprintf_s(imageName, L"thumb_%d_%dx%d", m_cbxType, m_thumbSize.cx, m_thumbSize.cy);

			// Check cache first (fast path)
			if (DarkThumbs::ThumbnailCache::IsCached(archivePath, imageName, (ULONGLONG)fileSize, lastModified))
			{
				HBITMAP cachedBitmap = DarkThumbs::ThumbnailCache::LoadFromCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified);
				if (cachedBitmap)
				{
					*phBmpThumbnail = cachedBitmap;
					return S_OK; // Cache hit - 10-100x faster!
				}
			}

			try
			{
				switch (m_cbxType)
				{
				case CBXTYPE_EPUB:
				{
					std::string xmlContent, rootpath, rootfile, coverfile;

					rootfile = GetEpubRootFile(m_cbxFile);

					CUnzip _z;
					if (!_z.Open(m_cbxFile))
						return E_FAIL;
					j = _z.GetItemCount();
					if (j == 0)
						return E_FAIL;

					size_t posStart, posEnd;

					CString prevname; // helper vars
					int thumbindex = -1;

					USES_CONVERSION;

					if (rootfile.length() > 0)
					{

						posStart = rootfile.find('/');
						if (posStart != std::string::npos)
						{
							rootpath = rootfile.substr(0, posStart + 1);
						}

						for (i = 0; i < j; i++)
						{
							if (!_z.GetItem(i))
								break;
							if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
								continue;

							std::string name;

							name = CT2A(_z.GetItemName());

							if (name.compare(rootfile) == 0)
							{
								HGLOBAL hGContainer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
								if (hGContainer)
								{
									bool b = false;
									LPVOID pBuf = ::GlobalLock(hGContainer);
									if (pBuf)
										b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

									if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR)
									{
										if (b)
										{
											// Find meta tag for cover

											std::string xmlContent, coverTag, coverId, itemTag;

											xmlContent = (char *)pBuf;

											posStart = xmlContent.find("name=\"cover\"");

											if (posStart == std::string::npos)
											{
												break;
											}

											posStart = xmlContent.find_last_of("<", posStart);
											posEnd = xmlContent.find(">", posStart);

											coverTag = xmlContent.substr(posStart, posEnd - posStart + 1);

											// Find cover item id

											posStart = coverTag.find("content=\"");

											if (posStart == std::string::npos)
											{
												break;
											}

											posStart += 9;
											posEnd = coverTag.find("\"", posStart);

											coverId = coverTag.substr(posStart, posEnd - posStart);

											// Find item tag in original opf file contents

											posStart = xmlContent.find("id=\"" + coverId + "\"");

											if (posStart == std::string::npos)
											{
												break;
											}

											posStart = xmlContent.find_last_of("<", posStart);
											posEnd = xmlContent.find(">", posStart);

											itemTag = xmlContent.substr(posStart, posEnd - posStart + 1);

											// Find cover path in item tag

											posStart = itemTag.find("href=\"");

											if (posStart == std::string::npos)
											{
												break;
											}

											posStart += 6;
											posEnd = itemTag.find("\"", posStart);

											if (posEnd == std::string::npos)
											{
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

					if (coverfile.empty())
					{

						for (i = 0; i < j; i++)
						{
							if (!_z.GetItem(i))
								break;
							if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
								continue;
							if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
								continue;

							if (IsImage(_z.GetItemName()))
							{
								std::string imgFilename(T2A(_z.GetItemName()));

								if (imgFilename.find("cover") != std::string::npos)
								{
									coverfile = imgFilename;
								}
								else if (imgFilename.find("COVER") != std::string::npos)
								{
									coverfile = imgFilename;
								}
								else if (imgFilename.find("Cover") != std::string::npos)
								{
									coverfile = imgFilename;
								}
							}
						}
					}

					if (coverfile.length() > 0)
					{

						for (i = 0; i < j; i++)
						{
							if (!_z.GetItem(i))
								break;
							if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
								continue;
							if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
								continue;

							if (_stricmp(T2A(_z.GetItemName()), coverfile.c_str()) == 0 && IsImage(_z.GetItemName()))
							{
								if (thumbindex < 0)
									thumbindex = i; // assign thumbindex if already sorted

								if (!m_bSort)
									break; // if NoSort

								if (prevname.IsEmpty())
									prevname = _z.GetItemName(); // can't compare empty string
								// take only first alphabetical name
								if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname))
								{
									thumbindex = i;
									prevname = _z.GetItemName();
								}
							}
						} // for loop

						if (thumbindex < 0)
							return E_FAIL;
						// go to thumb index
						if (!_z.GetItem(thumbindex))
							return E_FAIL;

						// create thumb			//GHND
						HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
						if (hG)
						{
							bool b = false;
							LPVOID pBuf = ::GlobalLock(hG);
							if (pBuf)
								b = _z.UnzipItemToMembuffer(thumbindex, pBuf, _z.GetItemUnpackedSize());

							if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR)
							{
								if (b)
								{
									IStream *pIs = NULL;
									if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM *)&pIs)) // autofree hG
									{
										*phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
										pIs->Release();
										pIs = NULL;
									}
								}
							}
						}
						// GlobalFree(hG);//autofreed
						return ((*phBmpThumbnail) ? S_OK : E_FAIL);
					}

					// something wrong with the epub, try falling back on first image in zip
				}
				case CBXTYPE_ZIP:
				case CBXTYPE_CBZ:
				case CBXTYPE_7Z:
				case CBXTYPE_CB7:
				case CBXTYPE_CBT:
				case CBXTYPE_MOBI:
				case CBXTYPE_AZW:
				case CBXTYPE_AZW3:
				case CBXTYPE_FB2:
				case CBXTYPE_PHZ:
				{
					CUnzip _z;
					if (!_z.Open(m_cbxFile))
						return E_FAIL;
					j = _z.GetItemCount();
					if (j == 0)
						return E_FAIL;

					// Sprint C2: Check collage mode from registry
					DarkThumbs::ThumbnailCollage::CollageMode collageMode =
						DarkThumbs::ThumbnailCollage::GetCollageModeFromRegistry();

					// Extract multiple images if collage mode is enabled
					if (collageMode != DarkThumbs::ThumbnailCollage::MODE_SINGLE)
					{
						int maxImages = static_cast<int>(collageMode); // MODE_2X2=4, MODE_3X3=9, MODE_4X4=16
						std::vector<HBITMAP> images = ExtractMultipleImagesFromZIP(_z, maxImages);

						if (!images.empty())
						{
							// Create collage from multiple images
							*phBmpThumbnail = DarkThumbs::ThumbnailCollage::CreateCollage(
								images,
								m_thumbSize.cx,
								m_thumbSize.cy,
								collageMode);

							// Clean up individual images
							for (HBITMAP hBmp : images)
							{
								if (hBmp && hBmp != *phBmpThumbnail)
								{
									DeleteObject(hBmp);
								}
							}

							// Save to cache
							if (*phBmpThumbnail)
							{
								DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified, *phBmpThumbnail);
							}

							return ((*phBmpThumbnail) ? S_OK : E_FAIL);
						}
					}

					// Fallback to single image mode (original logic)
					CString prevname; // helper vars
					int thumbindex = -1;

					for (i = 0; i < j; i++)
					{
						if (!_z.GetItem(i))
							break;
						if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
							continue;
						if ((_z.GetItemPackedSize() == 0) || (_z.GetItemUnpackedSize() == 0))
							continue;

						if (IsImage(_z.GetItemName()))
						{
							if (thumbindex < 0)
								thumbindex = i; // assign thumbindex if already sorted

							if (!m_bSort)
								break; // if NoSort

							if (prevname.IsEmpty())
								prevname = _z.GetItemName(); // can't compare empty string
							// take only first alphabetical name
							if (-1 == StrCmpLogicalW(_z.GetItemName(), prevname))
							{
								thumbindex = i;
								prevname = _z.GetItemName();
							}
						}
					} // for loop

					if (thumbindex < 0)
						return E_FAIL;
					// go to thumb index
					if (!_z.GetItem(thumbindex))
						return E_FAIL;

					// create thumb			//GHND
					HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
					if (hG)
					{
						bool b = false;
						LPVOID pBuf = ::GlobalLock(hG);
						if (pBuf)
							b = _z.UnzipItemToMembuffer(thumbindex, pBuf, _z.GetItemUnpackedSize());

						if (::GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR)
						{
							if (b)
							{
								IStream *pIs = NULL;
								if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM *)&pIs)) // autofree hG
								{
									*phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
									pIs->Release();
									pIs = NULL;
								}
							}
						}
						// GlobalFree(hG);//autofreed
					}

					return ((*phBmpThumbnail) ? S_OK : E_FAIL);
				} // dtors!
				break;

#ifdef ENABLE_LIBARCHIVE_SUPPORT
				// LibArchive formats: TAR, TAR.GZ, TAR.BZ2, TAR.XZ, TAR.ZST, CPIO, ISO, XAR, AR, DEB, CAB
				case CBXTYPE_TAR:
				case CBXTYPE_TAR_GZ:
				case CBXTYPE_TAR_BZ2:
				case CBXTYPE_TAR_XZ:
				case CBXTYPE_TAR_ZST:
				case CBXTYPE_CPIO:
				case CBXTYPE_ISO:
				case CBXTYPE_XAR:
				case CBXTYPE_AR:
				case CBXTYPE_DEB:
				case CBXTYPE_CAB:
				{
					// Open archive with libarchive
					if (!DarkThumbs::LibArchiveWrapper::OpenArchive(m_cbxFile))
					{
						return E_FAIL;
					}

					CString prevname;
					int thumbindex = -1;
					int currentIndex = 0;
					std::vector<std::pair<std::string, int>> imageEntries; // name, index

					// Enumerate entries to find images
					while (DarkThumbs::LibArchiveWrapper::ReadNextEntry())
					{
						if (DarkThumbs::LibArchiveWrapper::IsEntryDirectory())
						{
							currentIndex++;
							continue;
						}

						int64_t entrySize = DarkThumbs::LibArchiveWrapper::GetEntrySize();
						if (entrySize <= 0 || entrySize > CBXMEM_MAXBUFFER_SIZE)
						{
							currentIndex++;
							continue;
						}

						const char *entryName = DarkThumbs::LibArchiveWrapper::GetEntryName();
						if (!entryName)
						{
							currentIndex++;
							continue;
						}

						// Convert to wide string for IsImage check
						USES_CONVERSION;
						LPCTSTR wEntryName = A2CT(entryName);

						if (IsImage(wEntryName))
						{
							imageEntries.push_back(std::make_pair(std::string(entryName), currentIndex));

							if (!m_bSort)
							{
								thumbindex = currentIndex;
								break; // Take first image if not sorting
							}
						}
						currentIndex++;
					}

					// If sorting, find alphabetically first image
					if (m_bSort && !imageEntries.empty())
					{
						std::string firstName = imageEntries[0].first;
						thumbindex = imageEntries[0].second;

						for (size_t i = 1; i < imageEntries.size(); i++)
						{
							if (imageEntries[i].first < firstName)
							{
								firstName = imageEntries[i].first;
								thumbindex = imageEntries[i].second;
							}
						}
					}

					if (thumbindex < 0)
					{
						DarkThumbs::LibArchiveWrapper::CloseArchive();
						return E_FAIL;
					}

					// Re-open and seek to thumbnail entry
					DarkThumbs::LibArchiveWrapper::CloseArchive();
					if (!DarkThumbs::LibArchiveWrapper::OpenArchive(m_cbxFile))
					{
						return E_FAIL;
					}

					currentIndex = 0;
					while (DarkThumbs::LibArchiveWrapper::ReadNextEntry())
					{
						if (currentIndex == thumbindex)
						{
							int64_t entrySize = DarkThumbs::LibArchiveWrapper::GetEntrySize();

							// Extract entry to memory
							HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)entrySize);
							if (hG)
							{
								LPVOID pBuf = GlobalLock(hG);
								if (pBuf)
								{
									SSIZE_T bytesRead = DarkThumbs::LibArchiveWrapper::ExtractEntryToMemory(pBuf, (size_t)entrySize);

									if (GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR)
									{
										if (bytesRead > 0)
										{
											IStream *pIs = NULL;
											if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM *)&pIs))
											{
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

					DarkThumbs::LibArchiveWrapper::CloseArchive();
					return ((*phBmpThumbnail) ? S_OK : E_FAIL);
				}
				break;
#endif // ENABLE_LIBARCHIVE_SUPPORT

				case CBXTYPE_VIDEO:
				{
#if defined(CBXSHELL_LEGACY_DECODERS) && defined(ENABLE_VIDEO_SUPPORT)
					// Extract video thumbnail using DirectShow (Sprint C3)
					std::wstring videoPath(m_cbxFile.operator LPCTSTR());

					// Extract frame at 10% into video (skip intros)
					HBITMAP hVideoFrame = DarkThumbs::VideoThumbnail::ExtractFrame(videoPath, 0.1);

					if (hVideoFrame)
					{
						// Scale to thumbnail size
						*phBmpThumbnail = ScaleBitmapToThumbnail(hVideoFrame, &m_thumbSize);

						// Clean up original if different from scaled
						if (*phBmpThumbnail != hVideoFrame)
						{
							DeleteObject(hVideoFrame);
						}

						// Save to cache
						if (*phBmpThumbnail)
						{
							DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified, *phBmpThumbnail);
						}

						return S_OK;
					}
#endif
					return E_FAIL;
				}
				break;

				case CBXTYPE_AUDIO:
				{
#if defined(CBXSHELL_LEGACY_DECODERS) && defined(ENABLE_AUDIO_SUPPORT)
					// Extract audio thumbnail (Sprint 8)
					std::wstring audioPath(m_cbxFile.operator LPCTSTR());

					// Try to extract album art first
					HBITMAP hAlbumArt = DarkThumbs::AudioThumbnail::ExtractAlbumArt(audioPath);

					if (hAlbumArt)
					{
						// Scale to thumbnail size
						*phBmpThumbnail = ScaleBitmapToThumbnail(hAlbumArt, &m_thumbSize);

						// Clean up original if different from scaled
						if (*phBmpThumbnail != hAlbumArt)
						{
							DeleteObject(hAlbumArt);
						}

						// Save to cache
						if (*phBmpThumbnail)
						{
							DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified, *phBmpThumbnail);
						}

						return S_OK;
					}

					// No album art, generate waveform visualization
					HBITMAP hWaveform = DarkThumbs::AudioThumbnail::GenerateWaveform(audioPath, m_thumbSize.cx, m_thumbSize.cy);

					if (hWaveform)
					{
						*phBmpThumbnail = hWaveform;

						// Save to cache
						DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified, *phBmpThumbnail);

						return S_OK;
					}
#endif
					return E_FAIL;
				}
				break;

				case CBXTYPE_DOCX:
				case CBXTYPE_PPTX:
				case CBXTYPE_XLSX:
				case CBXTYPE_DOC:
				case CBXTYPE_PPT:
				case CBXTYPE_XLS:
				{
#if defined(CBXSHELL_LEGACY_DECODERS) && defined(ENABLE_DOCUMENT_SUPPORT)
					// Extract document thumbnail (Sprint 9 Phase 1)
					std::wstring docPath(m_cbxFile.operator LPCTSTR());

					HBITMAP hDocThumbnail = DarkThumbs::DocumentThumbnail::ExtractDocumentThumbnail(
						docPath, m_thumbSize.cx, m_thumbSize.cy);

					if (hDocThumbnail)
					{
						*phBmpThumbnail = hDocThumbnail;

						// Save to cache
						DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName,
																(ULONGLONG)fileSize, lastModified, *phBmpThumbnail);

						return S_OK;
					}
#endif
					return E_FAIL;
				}
				break;

				case CBXTYPE_FONT:
				{
#if defined(CBXSHELL_LEGACY_DECODERS) && defined(ENABLE_FONT_SUPPORT)
					// Generate font preview (Sprint 9 Phase 1)
					std::wstring fontPath(m_cbxFile.operator LPCTSTR());

					HBITMAP hFontPreview = DarkThumbs::FontPreview::GenerateFontPreview(
						fontPath, m_thumbSize.cx, m_thumbSize.cy);

					if (hFontPreview)
					{
						*phBmpThumbnail = hFontPreview;

						// Save to cache
						DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName,
																(ULONGLONG)fileSize, lastModified, *phBmpThumbnail);

						return S_OK;
					}
#endif
					return E_FAIL;
				}
				break;

				// Standalone image/document formats (WebP, JPEG XL, AVIF, HEIF, SVG, RAW, TIFF, PDF)
				default:
				{
					// Handle standalone files by loading them into a stream
					// ThumbnailFromIStream will detect and decode the format

					CAtlFile file;
					HRESULT hr = file.Create(m_cbxFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0);
					if (FAILED(hr))
						return E_FAIL;

					ULONGLONG fileSize64 = 0;
					hr = file.GetSize(fileSize64);
					if (FAILED(hr) || fileSize64 == 0 || fileSize64 > CBXMEM_MAXBUFFER_SIZE)
					{
						return E_FAIL;
					}

					// Read file into memory
					HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)fileSize64);
					if (!hG)
						return E_FAIL;

					LPVOID pBuf = GlobalLock(hG);
					if (!pBuf)
					{
						GlobalFree(hG);
						return E_FAIL;
					}

					DWORD bytesRead = 0;
					hr = file.Read(pBuf, (DWORD)fileSize64, bytesRead);

					if (GlobalUnlock(hG) == 0 && GetLastError() == NO_ERROR)
					{
						if (SUCCEEDED(hr) && bytesRead == fileSize64)
						{
							IStream *pIs = NULL;
							if (S_OK == CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM *)&pIs))
							{
								*phBmpThumbnail = ThumbnailFromIStream(pIs, &m_thumbSize);
								pIs->Release();

								// Save to cache
								if (*phBmpThumbnail)
								{
									DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize64, lastModified, *phBmpThumbnail);
								}

								return ((*phBmpThumbnail) ? S_OK : E_FAIL);
							}
						}
					}

					GlobalFree(hG);
					return E_FAIL;
				}
				break;
				}
			}
			catch (...)
			{
				ATLTRACE("exception in IExtractImage::Extract\n");
				return S_FALSE;
			}

			// Save generated thumbnail to cache
			if (*phBmpThumbnail)
			{
				DarkThumbs::ThumbnailCache::SaveToCache(archivePath, imageName, (ULONGLONG)fileSize, lastModified, *phBmpThumbnail);
			}

			return S_OK;
		}

		std::string GetEpubTitle(LPCTSTR ePubFile)
		{
			std::string rootfile, title;

			rootfile = GetEpubRootFile(ePubFile);

			if (rootfile.empty())
			{
				return std::string();
			}

			CUnzip _z;
			if (!_z.Open(m_cbxFile))
				return std::string();
			j = _z.GetItemCount();
			if (j == 0)
				return std::string();

			size_t posStart, posEnd;

			for (i = 0; i < j; i++)
			{
				if (!_z.GetItem(i))
					break;
				if (_z.ItemIsDirectory() || (_z.GetItemUnpackedSize() > CBXMEM_MAXBUFFER_SIZE))
					continue;

				std::string name;

				name = CT2A(_z.GetItemName());

				if (name.compare(rootfile) == 0)
				{
					HGLOBAL hGContainer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (SIZE_T)_z.GetItemUnpackedSize());
					if (hGContainer)
					{
						bool b = false;
						LPVOID pBuf = ::GlobalLock(hGContainer);
						if (pBuf)
							b = _z.UnzipItemToMembuffer(i, pBuf, _z.GetItemUnpackedSize());

						if (::GlobalUnlock(hGContainer) == 0 && GetLastError() == NO_ERROR)
						{
							if (b)
							{
								// Find <dc:title> tag

								std::string xmlContent, coverTag, coverId, itemTag;

								xmlContent = (char *)pBuf;

								posStart = xmlContent.find("<dc:title>");

								if (posStart == std::string::npos)
								{
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

		//////////////////////////////
		// IQueryInfo::GetInfoTip(DWORD dwFlags, LPWSTR *ppwszTip)
		HRESULT OnGetInfoTip(LPWSTR *ppwszTip)
		{
			// ATLTRACE("IQueryInfo::GetInfoTip\n");
			try
			{
				CString tip;

				__int64 _fs;
				if (!GetFileSizeCrt(m_cbxFile, _fs))
					return E_FAIL;

				TCHAR _tf[16]; // SecureZeroMemory?

				switch (m_cbxType)
				{
				case CBXTYPE_ZIP:
					if (_fs == 0)
						tip = _T("ZIP Archive\nSize: 0 bytes");
					else
					{
						if (GetImageCountZIP(m_cbxFile, i, j))
							tip.Format(_T("ZIP Archive\n%d Images\n%d Files\nSize: %s"),
									   i, j, StrFormatByteSize64(_fs, _tf, 16));
						else
							tip.Format(_T("ZIP Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					}
					break;
				case CBXTYPE_CBZ:
					if (_fs == 0)
						tip = _T("ZIP Image Archive\nSize: 0 bytes");
					else
					{
						if (GetImageCountZIP(m_cbxFile, i, j))
							tip.Format(_T("ZIP Image Archive\n%d Images\n%d Files\nSize: %s"),
									   i, j, StrFormatByteSize64(_fs, _tf, 16));
						else
							tip.Format(_T("ZIP Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					}
					break;
				case CBXTYPE_EPUB:
					if (_fs == 0)
						tip = _T("EPUB File\nSize: 0 bytes");
					else
					{
						std::string title = GetEpubTitle(m_cbxFile);
						ATL::CAtlStringW titleW(CA2W(title.c_str()));

						if (title.length() > 0)
							tip.Format(_T("EPUB File\n%s\nSize: %s"),
									   titleW, StrFormatByteSize64(_fs, _tf, 16));
						else
							tip.Format(_T("EPUB File\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					}
					break;
				case CBXTYPE_7Z:
					tip.Format(_T("7-Zip Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_CB7:
					if (GetImageCountZIP(m_cbxFile, i, j))
						tip.Format(_T("7-Zip Image Archive\n%d Images\n%d Files\nSize: %s"),
								   i, j, StrFormatByteSize64(_fs, _tf, 16));
					else
						tip.Format(_T("7-Zip Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_TAR:
					tip.Format(_T("TAR Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_CBT:
					if (GetImageCountZIP(m_cbxFile, i, j))
						tip.Format(_T("TAR Image Archive\n%d Images\n%d Files\nSize: %s"),
								   i, j, StrFormatByteSize64(_fs, _tf, 16));
					else
						tip.Format(_T("TAR Image Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_MOBI:
					tip.Format(_T("MOBI E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_AZW:
					tip.Format(_T("Kindle (AZW) E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_AZW3:
					tip.Format(_T("Kindle (AZW3) E-Book\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_FB2:
					tip.Format(_T("FictionBook File\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;
				case CBXTYPE_PHZ:
					if (GetImageCountZIP(m_cbxFile, i, j))
						tip.Format(_T("Photo Archive\n%d Images\n%d Files\nSize: %s"),
								   i, j, StrFormatByteSize64(_fs, _tf, 16));
					else
						tip.Format(_T("Photo Archive\nSize: %s"), StrFormatByteSize64(_fs, _tf, 16));
					break;

				default:
					ATLTRACE("IQueryInfo::GetInfoTip : CBXTYPE_NONE\n");
					return E_FAIL;
				}

				*ppwszTip = (WCHAR *)::CoTaskMemAlloc((tip.GetLength() + 1) * sizeof(WCHAR)); // caller must call CoTaskMemFree
				if (*ppwszTip == NULL)
					return E_FAIL;
				if (0 != ::wcscpy_s(*ppwszTip, tip.GetLength() + 1, tip))
					return E_FAIL;

				return S_OK;
			}
			catch (...)
			{
				ATLTRACE("exception in IQueryInfo::GetInfoTip\n");
				return S_FALSE;
			}

			return S_FALSE;
		}

	private:
		CString m_cbxFile; // overcome MAX_PATH limit?
		SIZE m_thumbSize;
		int i, j; // helpers
		CBXTYPE m_cbxType;
		IStream *m_pIs;
		BOOL m_bSort;
		BOOL m_showIcon;

	private:
		inline BOOL GetFileSizeCrt(LPCTSTR pszFile, __int64 &fsize)
		{
			struct _stat64 _s;
			_s.st_size = 0;
			if (0 != ::_tstat64(pszFile, &_s))
				return FALSE;
			fsize = _s.st_size;
			return TRUE;
		}

		inline BOOL StrEqual(LPCTSTR psz1, LPCTSTR psz2) { return (::StrCmpI(psz1, psz2) == 0); }

		BOOL IsImage(LPCTSTR szFile)
		{
			LPWSTR _e = PathFindExtension(szFile);
			// Traditional formats
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
			// Modern formats (Windows 11)
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
			// RAW photo formats (v5.3+)
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

		BOOL IsVideo(LPCTSTR szFile)
		{
			LPWSTR _e = PathFindExtension(szFile);
			// Common video formats (v5.0+: 35+ formats)
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
			// Sprint 8: Additional video formats
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

		BOOL IsAudio(LPCTSTR szFile)
		{
			LPWSTR _e = PathFindExtension(szFile);
			// Audio format support (v5.0+: 11 formats)
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

		BOOL IsDocument(LPCTSTR szFile)
		{
			LPWSTR _e = PathFindExtension(szFile);
			// Office document formats (v5.0+: 6 formats)
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

		BOOL IsFont(LPCTSTR szFile)
		{
			LPWSTR _e = PathFindExtension(szFile);
			// Sprint 9 Phase 1: Font formats (4 formats)
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

		inline CBXTYPE GetCBXType(LPCTSTR szExt)
		{
			// Comic book archives
			if (StrEqual(szExt, _T(".cbz")))
				return CBXTYPE_CBZ;
			// if (StrEqual(szExt, _T(".cbr"))) return CBXTYPE_CBR;  // CBR removed
			if (StrEqual(szExt, _T(".cb7")))
				return CBXTYPE_CB7;
			if (StrEqual(szExt, _T(".cbt")))
				return CBXTYPE_CBT;

			// Generic archives
			if (StrEqual(szExt, _T(".zip")))
				return CBXTYPE_ZIP;
			if (StrEqual(szExt, _T(".7z")))
				return CBXTYPE_7Z;
			if (StrEqual(szExt, _T(".tar")))
				return CBXTYPE_TAR;

			// LibArchive formats (v5.2+)
#ifdef ENABLE_LIBARCHIVE_SUPPORT
			if (StrEqual(szExt, _T(".tar.gz")))
				return CBXTYPE_TAR_GZ;
			if (StrEqual(szExt, _T(".tgz")))
				return CBXTYPE_TAR_GZ;
			if (StrEqual(szExt, _T(".tar.bz2")))
				return CBXTYPE_TAR_BZ2;
			if (StrEqual(szExt, _T(".tbz")))
				return CBXTYPE_TAR_BZ2;
			if (StrEqual(szExt, _T(".tb2")))
				return CBXTYPE_TAR_BZ2;
			if (StrEqual(szExt, _T(".tar.xz")))
				return CBXTYPE_TAR_XZ;
			if (StrEqual(szExt, _T(".txz")))
				return CBXTYPE_TAR_XZ;
			if (StrEqual(szExt, _T(".tar.zst")))
				return CBXTYPE_TAR_ZST;
			if (StrEqual(szExt, _T(".tzst")))
				return CBXTYPE_TAR_ZST;
			if (StrEqual(szExt, _T(".cpio")))
				return CBXTYPE_CPIO;
			if (StrEqual(szExt, _T(".iso")))
				return CBXTYPE_ISO;
			if (StrEqual(szExt, _T(".xar")))
				return CBXTYPE_XAR;
			if (StrEqual(szExt, _T(".ar")))
				return CBXTYPE_AR;
			if (StrEqual(szExt, _T(".deb")))
				return CBXTYPE_DEB;
			if (StrEqual(szExt, _T(".cab")))
				return CBXTYPE_CAB;
#endif

			// Ebook formats
			if (StrEqual(szExt, _T(".epub")))
				return CBXTYPE_EPUB;
			if (StrEqual(szExt, _T(".mobi")))
				return CBXTYPE_MOBI;
			if (StrEqual(szExt, _T(".azw")))
				return CBXTYPE_AZW;
			if (StrEqual(szExt, _T(".azw3")))
				return CBXTYPE_AZW3;
			if (StrEqual(szExt, _T(".fb2")))
				return CBXTYPE_FB2;

			// Photo archives
			if (StrEqual(szExt, _T(".phz")))
				return CBXTYPE_PHZ;

			// Video formats (Sprint 8: 35+ formats)
			if (StrEqual(szExt, _T(".mp4")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".avi")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mkv")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mov")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".wmv")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".flv")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".webm")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".m4v")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mpg")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mpeg")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".3gp")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".3g2")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".asf")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".m1v")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".m2v")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".ts")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".m2ts")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mts")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".m2t")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mp4v")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".3gp2")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".3gpp")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mk3d")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".f4v")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".ogm")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".ogv")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".rm")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".rmvb")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".dv")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".mxf")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".ivf")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".evo")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".264")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".video")))
				return CBXTYPE_VIDEO;

			// Audio formats (Sprint 8: 11 formats)
			if (StrEqual(szExt, _T(".mp3")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".wav")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".m4a")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".ape")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".flac")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".ogg")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".mka")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".opus")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".tak")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".wv")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".mpc")))
				return CBXTYPE_AUDIO;

			// Document formats (Sprint 9 Phase 1: 6 formats)
			if (StrEqual(szExt, _T(".docx")))
				return CBXTYPE_DOCX;
			if (StrEqual(szExt, _T(".pptx")))
				return CBXTYPE_PPTX;
			if (StrEqual(szExt, _T(".xlsx")))
				return CBXTYPE_XLSX;
			if (StrEqual(szExt, _T(".doc")))
				return CBXTYPE_DOC;
			if (StrEqual(szExt, _T(".ppt")))
				return CBXTYPE_PPT;
			if (StrEqual(szExt, _T(".xls")))
				return CBXTYPE_XLS;

			// Font formats (v5.0+: 4 formats)
			if (StrEqual(szExt, _T(".ttf")))
				return CBXTYPE_FONT;
			if (StrEqual(szExt, _T(".otf")))
				return CBXTYPE_FONT;
			if (StrEqual(szExt, _T(".woff")))
				return CBXTYPE_FONT;
			if (StrEqual(szExt, _T(".woff2")))
				return CBXTYPE_FONT;

			// PDF format (v5.0+)
			if (StrEqual(szExt, _T(".pdf")))
				return CBXTYPE_PDF;

			// Modern image formats (v5.0+)
#ifdef ENABLE_WEBP_SUPPORT
			if (StrEqual(szExt, _T(".webp")))
				return CBXTYPE_WEBP;
#endif
#ifdef ENABLE_AVIF_SUPPORT
			if (StrEqual(szExt, _T(".avif")))
				return CBXTYPE_AVIF;
			if (StrEqual(szExt, _T(".avifs")))
				return CBXTYPE_AVIF;
#endif
#ifdef ENABLE_HEIF_SUPPORT
			if (StrEqual(szExt, _T(".heic")))
				return CBXTYPE_HEIC;
			if (StrEqual(szExt, _T(".heif")))
				return CBXTYPE_HEIF;
			if (StrEqual(szExt, _T(".heics")))
				return CBXTYPE_HEIC;
			if (StrEqual(szExt, _T(".heifs")))
				return CBXTYPE_HEIF;
			// Sprint 175: Add missing HEIF extensions
			if (StrEqual(szExt, _T(".hif")))
				return CBXTYPE_HEIC;
			if (StrEqual(szExt, _T(".avci")))
				return CBXTYPE_HEIC;
			if (StrEqual(szExt, _T(".avcs")))
				return CBXTYPE_HEIF;
#endif
#ifdef ENABLE_JXL_SUPPORT
			if (StrEqual(szExt, _T(".jxl")))
				return CBXTYPE_JXL;
#endif
			if (StrEqual(szExt, _T(".tif")))
				return CBXTYPE_TIFF;
			if (StrEqual(szExt, _T(".tiff")))
				return CBXTYPE_TIFF;
#ifdef ENABLE_SVG_SUPPORT
			if (StrEqual(szExt, _T(".svg")))
				return CBXTYPE_SVG;
			if (StrEqual(szExt, _T(".svgz")))
				return CBXTYPE_SVG;
#endif
#ifdef ENABLE_RAW_SUPPORT
			// Canon RAW formats
			if (StrEqual(szExt, _T(".cr2")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".cr3")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".crw")))
				return CBXTYPE_RAW;
			// Nikon RAW
			if (StrEqual(szExt, _T(".nef")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".nrw")))
				return CBXTYPE_RAW;
			// Sony RAW
			if (StrEqual(szExt, _T(".arw")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".srf")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".sr2")))
				return CBXTYPE_RAW;
			// Other common RAW formats
			if (StrEqual(szExt, _T(".dng")))
				return CBXTYPE_RAW; // Adobe Digital Negative
			if (StrEqual(szExt, _T(".orf")))
				return CBXTYPE_RAW; // Olympus
			if (StrEqual(szExt, _T(".rw2")))
				return CBXTYPE_RAW; // Panasonic
			if (StrEqual(szExt, _T(".pef")))
				return CBXTYPE_RAW; // Pentax
			if (StrEqual(szExt, _T(".raf")))
				return CBXTYPE_RAW; // Fujifilm
			if (StrEqual(szExt, _T(".dcr")))
				return CBXTYPE_RAW; // Kodak
			if (StrEqual(szExt, _T(".mrw")))
				return CBXTYPE_RAW; // Minolta
			if (StrEqual(szExt, _T(".x3f")))
				return CBXTYPE_RAW; // Sigma
			// Samsung
			if (StrEqual(szExt, _T(".srw")))
				return CBXTYPE_RAW;
			// Leica
			if (StrEqual(szExt, _T(".rwl")))
				return CBXTYPE_RAW;
			// Hasselblad
			if (StrEqual(szExt, _T(".3fr")))
				return CBXTYPE_RAW;
			// Phase One
			if (StrEqual(szExt, _T(".iiq")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".cap")))
				return CBXTYPE_RAW;
			// Leaf
			if (StrEqual(szExt, _T(".mos")))
				return CBXTYPE_RAW;
			// Epson
			if (StrEqual(szExt, _T(".erf")))
				return CBXTYPE_RAW;
			// Kodak
			if (StrEqual(szExt, _T(".kdc")))
				return CBXTYPE_RAW;
			if (StrEqual(szExt, _T(".dcr")))
				return CBXTYPE_RAW;
			// Mamiya
			if (StrEqual(szExt, _T(".mef")))
				return CBXTYPE_RAW;
			// Casio
			if (StrEqual(szExt, _T(".bay")))
				return CBXTYPE_RAW;
			// GoPro
			if (StrEqual(szExt, _T(".gpr")))
				return CBXTYPE_RAW;
#endif

			// Photoshop/Design formats (v6.1+)
#ifdef ENABLE_PSD_SUPPORT
			if (StrEqual(szExt, _T(".psd")))
				return CBXTYPE_PSD;
			if (StrEqual(szExt, _T(".psb")))
				return CBXTYPE_PSD;
#endif

			// DirectX/Game textures (v6.1+)
#ifdef ENABLE_DDS_SUPPORT
			if (StrEqual(szExt, _T(".dds")))
				return CBXTYPE_DDS;
#endif

			// HDR image formats (v6.1+)
#ifdef ENABLE_HDR_SUPPORT
			if (StrEqual(szExt, _T(".hdr")))
				return CBXTYPE_HDR;
#endif
#ifdef ENABLE_EXR_SUPPORT
			if (StrEqual(szExt, _T(".exr")))
				return CBXTYPE_EXR;
#endif

			// Netpbm portable image formats (v6.1+)
#ifdef ENABLE_PPM_SUPPORT
			if (StrEqual(szExt, _T(".ppm")))
				return CBXTYPE_PPM;
			if (StrEqual(szExt, _T(".pgm")))
				return CBXTYPE_PPM;
			if (StrEqual(szExt, _T(".pbm")))
				return CBXTYPE_PPM;
			if (StrEqual(szExt, _T(".pnm")))
				return CBXTYPE_PPM;
			if (StrEqual(szExt, _T(".pam")))
				return CBXTYPE_PPM;
			if (StrEqual(szExt, _T(".pfm")))
				return CBXTYPE_PPM;
#endif

			// Additional video formats (v6.1+)
#ifdef ENABLE_VIDEO_SUPPORT
			if (StrEqual(szExt, _T(".vob")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".divx")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".h264")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".h265")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".hevc")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".av1")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".vp9")))
				return CBXTYPE_VIDEO;
			if (StrEqual(szExt, _T(".y4m")))
				return CBXTYPE_VIDEO;
#endif

			// Additional audio formats (v6.1+)
#ifdef ENABLE_AUDIO_SUPPORT
			if (StrEqual(szExt, _T(".aac")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".wma")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".aiff")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".aif")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".dsf")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".dff")))
				return CBXTYPE_AUDIO;
			if (StrEqual(szExt, _T(".alac")))
				return CBXTYPE_AUDIO;
#endif

			// Additional document formats (v6.1+)
#ifdef ENABLE_DOCUMENT_SUPPORT
			if (StrEqual(szExt, _T(".rtf")))
				return CBXTYPE_DOC;
			if (StrEqual(szExt, _T(".odt")))
				return CBXTYPE_DOCX;
			if (StrEqual(szExt, _T(".odp")))
				return CBXTYPE_PPTX;
			if (StrEqual(szExt, _T(".ods")))
				return CBXTYPE_XLSX;
			if (StrEqual(szExt, _T(".xps")))
				return CBXTYPE_DOC;
#endif

			// eBook additional formats (v6.1+)
			// Fixed in Sprint 175: djvu/djv now correctly route to CBXTYPE_DJVU
			if (StrEqual(szExt, _T(".djvu")))
				return CBXTYPE_DJVU;
			if (StrEqual(szExt, _T(".djv")))
				return CBXTYPE_DJVU;

			// 3D Model formats (Sprint 175)
			if (StrEqual(szExt, _T(".obj")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".stl")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".gltf")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".glb")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".fbx")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".3ds")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".dae")))
				return CBXTYPE_MODEL;
			if (StrEqual(szExt, _T(".ply")))
				return CBXTYPE_MODEL;

			// Additional image formats (Sprint 180)
			if (StrEqual(szExt, _T(".wmf")))
				return CBXTYPE_WMF;
			if (StrEqual(szExt, _T(".emf")))
				return CBXTYPE_EMF;
			if (StrEqual(szExt, _T(".pcx")))
				return CBXTYPE_PCX;
			if (StrEqual(szExt, _T(".ff")))
				return CBXTYPE_FARBFELD;

			// JPEG 2000 formats (Sprint 181)
			if (StrEqual(szExt, _T(".jp2")))
				return CBXTYPE_JP2;
			if (StrEqual(szExt, _T(".j2k")))
				return CBXTYPE_JP2;
			if (StrEqual(szExt, _T(".j2c")))
				return CBXTYPE_JP2;
			if (StrEqual(szExt, _T(".jpx")))
				return CBXTYPE_JP2;
			if (StrEqual(szExt, _T(".jpf")))
				return CBXTYPE_JP2;
			if (StrEqual(szExt, _T(".jph")))
				return CBXTYPE_JP2;

			return CBXTYPE_NONE;
		}

		BOOL GetImageCountZIP(LPCTSTR cbzFile, int &imagecount, int &filecount)
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
			for (_i = 0; _i < _z.GetItemCount(); _i++)
			{
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
		BOOL GetImageCountRAR(LPCTSTR cbrFile, int &imagecount, int &filecount)
		{
			imagecount = 0;
			filecount = 0;

			CUnRar cr;
			if (!cr.Open(cbrFile))
				return FALSE;

			// enum solid / skip volumes or encrypted file header archives
			if (cr.IsArchiveVolume() || cr.IsArchiveEncryptedHeaders())
				return FALSE;

			while (cr.ReadItemInfo())
			{
				// skip dirs
				if (!cr.IsItemDirectory())
				{
					if (IsImage(cr.GetItemName()))
						imagecount += 1;
					filecount += 1;
				}
				if (!cr.SkipItem())
					return FALSE;
			}
			return TRUE;
		}
#endif // DISABLE_RAR_SUPPORT

		static inline BOOL Draw(
			CImage ci,
			_In_ HDC hDestDC,
			_In_ int xDest,
			_In_ int yDest,
			_In_ int nDestWidth,
			_In_ int nDestHeight,
			_In_ int xSrc,
			_In_ int ySrc,
			_In_ int nSrcWidth,
			_In_ int nSrcHeight,
			_In_ Gdiplus::InterpolationMode interpolationMode) throw()
		{
			Gdiplus::Bitmap bm((HBITMAP)ci, NULL);
			if (bm.GetLastStatus() != Gdiplus::Ok)
			{
				return FALSE;
			}

			Gdiplus::Graphics dcDst(hDestDC);
			dcDst.SetInterpolationMode(interpolationMode);

			Gdiplus::Rect destRec(xDest, yDest, nDestWidth, nDestHeight);

			Gdiplus::Status status = dcDst.DrawImage(&bm, destRec, xSrc, ySrc, nSrcWidth, nSrcHeight, Gdiplus::Unit::UnitPixel);

			return status == Gdiplus::Ok;
		}

		HBITMAP ThumbnailFromIStream(IStream *pIs, const LPSIZE pThumbSize)
		{
			ATLASSERT(pIs);

			// Windows 11 Optimization: Try modern image formats first (delay-loaded DLLs)
			// This provides better performance and smaller memory footprint
			HBITMAP hModernBitmap = NULL;

// LEGACY: Modern formats tried inline (v5.0 era). Now handled by Engine pipeline.
// Only compiled when CBXSHELL_LEGACY_DECODERS is defined.
#if defined(CBXSHELL_LEGACY_DECODERS) && defined(ENABLE_WEBP_SUPPORT)
			{
				// Read stream data for format detection
				STATSTG stat = {};
				if (SUCCEEDED(pIs->Stat(&stat, STATFLAG_NONAME)))
				{
					size_t streamSize = static_cast<size_t>(stat.cbSize.QuadPart);

					if (streamSize > 0 && streamSize < CBXMEM_MAXBUFFER_SIZE)
					{
						std::vector<BYTE> buffer(static_cast<size_t>(streamSize));

						LARGE_INTEGER li = {};
						pIs->Seek(li, STREAM_SEEK_SET, NULL);

						ULONG bytesRead = 0;
						if (SUCCEEDED(pIs->Read(buffer.data(), static_cast<ULONG>(streamSize), &bytesRead)) && bytesRead == streamSize)
						{
							// Try modern formats in order of likelihood and efficiency

							// Try WebP decoder (delay-loaded)
							if (DarkThumbs::WebPDecoder::IsWebPFormat(buffer.data(), streamSize))
							{
								if (SUCCEEDED(DarkThumbs::WebPDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}

#ifdef ENABLE_HEIF_SUPPORT
							// Try HEIF/HEIC decoder (Windows native via WIC, Sprint 2)
							if (DarkThumbs::HEIFDecoderNative::IsHEIFFormat(buffer.data(), streamSize))
							{
								bool isDarkMode = DarkMode::IsSystemDarkMode();
								if (SUCCEEDED(DarkThumbs::HEIFDecoderNative::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap, isDarkMode)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_AVIF_SUPPORT
							// Try AVIF decoder (WIC-based, Windows 10+ with AV1 extension, Sprint 3)
							if (DarkThumbs::AVIFDecoder::IsAVIFFormat(buffer.data(), streamSize))
							{
								if (SUCCEEDED(DarkThumbs::AVIFDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_JXL_SUPPORT
							// Try JPEG XL decoder (delay-loaded, Sprint 4)
							if (DarkThumbs::JXLDecoder::IsJXLFormat(buffer.data(), streamSize))
							{
								if (SUCCEEDED(DarkThumbs::JXLDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_PDF_SUPPORT
							// Try PDF decoder (Windows.Data.Pdf, Sprint C4)
							if (DarkThumbs::PDFDecoder::IsPDFFormat(buffer.data(), streamSize))
							{
								// Render first page as thumbnail
								if (SUCCEEDED(DarkThumbs::PDFDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap,
																					  pThumbSize ? pThumbSize->cx : 256, pThumbSize ? pThumbSize->cy : 256)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_SVG_SUPPORT
							// Try SVG decoder (WIC placeholder, Sprint D2)
							if (DarkThumbs::SVGDecoder::IsSVGFormat(buffer.data(), streamSize))
							{
								if (SUCCEEDED(DarkThumbs::SVGDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap,
																					  pThumbSize ? pThumbSize->cx : 256, pThumbSize ? pThumbSize->cy : 256)))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_RAW_SUPPORT
							// Try RAW camera decoder (LibRaw-based, Sprint 13)
							{
								// RAW formats: Canon (CR2, CR3, CRW), Nikon (NEF, NRW), Sony (ARW, SRF, SR2),
								// Olympus (ORF), Panasonic (RW2), Pentax (PEF), Fujifilm (RAF), DNG, etc.
								HRESULT hrRaw = DarkThumbs::RAWDecoder::DecodeToHBITMAP(buffer.data(), streamSize, &hModernBitmap,
																						pThumbSize ? pThumbSize->cx : 512, pThumbSize ? pThumbSize->cy : 512);
								if (SUCCEEDED(hrRaw))
								{
									return ScaleBitmapToThumbnail(hModernBitmap, pThumbSize);
								}
							}
#endif

#ifdef ENABLE_RAW_SUPPORT
							// Try RAW decoder (WIC with Camera Codec Pack, Sprint D4)
							// Note: Requires Windows Camera Codec Pack or manufacturer codecs
							if (RawDecoder::IsRAWFormat(nullptr, buffer.data(), streamSize))
							{
								HBITMAP hRawBitmap = RawDecoder::DecodeToHBITMAP(pIs,
																				 pThumbSize ? pThumbSize->cx : 256);
								if (hRawBitmap)
								{
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
#endif				   // ENABLE_WEBP_SUPPORT		// Fallback to standard CImage (handles JPEG, PNG, BMP, GIF)
			CImage ci; // uses gdi+ internally
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
			if (!GetObject(hSourceBitmap, sizeof(bm), &bm))
			{
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
			if (rx >= 1 && ry >= 1)
			{
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
		HBITMAP ScaleCImageToThumbnail(CImage &ci, const LPSIZE pThumbSize)
		{
			// check size
			int tw = ci.GetWidth();
			int th = ci.GetHeight();
			float cx = (float)pThumbSize->cx;
			float cy = (float)pThumbSize->cy;
			float rx = cx / (float)tw;
			float ry = cy / (float)th;

			// if bigger size
			if ((rx < 1) || (ry < 1))
			{
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
				ci.ReleaseDC(); // don't forget!
				if (hbmpNew.IsNull())
					return NULL;

				HBITMAP hbmpOld = hdcNew.SelectBitmap(hbmpNew);
				// Use dark mode aware background color
				COLORREF bgColor = DarkMode::GetThumbnailBackgroundColor();
				hdcNew.FillSolidRect(0, 0, tw, th, bgColor);

				Draw(ci, hdcNew, 0, 0, tw, th, 0, 0, ci.GetWidth(), ci.GetHeight(), Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic); // too late for error checks
				if (m_showIcon)
					DrawIcon(hdcNew, 0, 0, zipIcon);

				hdcNew.SelectBitmap(hbmpOld);

				return hbmpNew.Detach();
			}

			return ci.Detach();
		}

		////unused
		// HBITMAP ThumbnailFromBuffer(LPCBYTE pBuf, const ULONG dwBufSize, const LPSIZE pThumbSize)
		//{
		//	HBITMAP hBmp = NULL;
		//	IStream* pIs = NULL;
		//	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwBufSize);
		//	if (hG)
		//	{
		//		if (S_OK==CreateStreamOnHGlobal(hG, TRUE, (LPSTREAM*)&pIs))
		//		{
		//			ULONG br;
		//			if (S_OK==pIs->Write(pBuf, dwBufSize, &br))//transfer buffer data
		//			{
		//				if (br==dwBufSize) hBmp=ThumbnailFromIStream(pIs, pThumbSize);
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

			UINT64 _ps, _us; // my speed optimization?
			CString prevname;
			__int64 thumbindex = -1;
			__int64 i = -1; // start at none (-1)

			while (_r.ReadItemInfo())
			{
				i += 1;
				_ps = _r.GetItemPackedSize64();
				_us = _r.GetItemUnpackedSize64();

				// skip directory/emtpy file/bigger than 32mb
				if (_r.IsItemDirectory() || (_us > CBXMEM_MAXBUFFER_SIZE) || (_ps == 0) || (_us == 0))
				{
					_r.SkipItem();
					continue;
				}

				// take only index of first alphabetical name
				if (IsImage(_r.GetItemName()))
				{
					// can't compare empty string
					if (prevname.IsEmpty())
						prevname = _r.GetItemName();
					if (thumbindex < 0)
						thumbindex = i; // assign thumbindex if already sorted
					// sort by name
					if (-1 == StrCmpLogicalW(_r.GetItemName(), prevname))
					{
						thumbindex = i;
						prevname = _r.GetItemName();
					}
				}
				_r.SkipItem(); // don't forget
			}

			return thumbindex;
		}
#endif // DISABLE_RAR_SUPPORT

	public:
		void LoadRegistrySettings()
		{
			DWORD _d;
			CRegKey _rk;
			if (ERROR_SUCCESS == _rk.Open(HKEY_CURRENT_USER, CBX_APP_KEY, KEY_READ))
			{
				if (ERROR_SUCCESS == _rk.QueryDWORDValue(_T("NoSort"), _d))
					m_bSort = (_d == FALSE);
				if (ERROR_SUCCESS == _rk.QueryDWORDValue(_T("ShowIcon"), _d))
					m_showIcon = (_d == TRUE);
			}
		}

		// Get current file path (for Engine integration v5.3.0)
		const wchar_t* GetFilePath() const
		{
			return m_cbxFile;
		}

#ifdef _DEBUG
	public:
		void debug_SetSort(BOOL bS = TRUE) { m_bSort = bS; }
#endif

	}; // class _CCBXArchive

} // namespace __cbx

#endif //_CBXARCHIVE_442B998D_B9C0_4AB0_BB2A_BC9C0AA10053_
