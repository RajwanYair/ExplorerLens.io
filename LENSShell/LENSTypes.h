///////////////////////////////////////////////////////////////////////////////
// LENSTypes.h — Format type constants and image type identifiers
//
// Extracted from LENSArchive.h to reduce monolithic header size.
// These constants define the file format identifiers used throughout the
// LENSShell extension for routing files to the appropriate decoder.
//
// LENSTYPE_* constants identify container/document formats.
// IMGTYPE_* constants identify embedded image formats within archives.
//
// Usage: #include "LENSTypes.h"
// Include this instead of LENSArchive.h when only type constants are needed.
///////////////////////////////////////////////////////////////////////////////

#ifndef _LENSTYPES_H_
#define _LENSTYPES_H_

#include <tchar.h>

// ============================================================================
// Memory and configuration constants
// ============================================================================

#define LENSMEM_MAXBUFFER_SIZE 33554432  // 32mb
#define LENSTYPE int
#define LENSTYPE_NONE 0

// ============================================================================
// Archive and eBook format types
// ============================================================================

#define LENSTYPE_ZIP 1
#define LENSTYPE_CBZ 2
#define LENSTYPE_RAR 3  // .rar RAR archives
#define LENSTYPE_CBR 4  // .cbr Comic book RAR archives
#define LENSTYPE_EPUB 5
#define LENSTYPE_7Z 6
#define LENSTYPE_CB7 7
#define LENSTYPE_TAR 8
#define LENSTYPE_CBT 9
#define LENSTYPE_MOBI 10
#define LENSTYPE_FB2 11
#define LENSTYPE_AZW 12
#define LENSTYPE_AZW3 13
#define LENSTYPE_PHZ 14

// ============================================================================
// Compression format types
// ============================================================================

#define LENSTYPE_BZIP2 15  // .bz2 BZIP2 compressed archives
#define LENSTYPE_ZSTD 16   // .zst Zstandard compressed archives
#define LENSTYPE_LZMA 17   // .xz/.lzma LZMA compressed archives

// ============================================================================
// Media and document format types
// ============================================================================

#define LENSTYPE_VIDEO 18  // Video files (.mp4, .avi, .mkv, etc.)
#define LENSTYPE_PDF 19    // PDF documents
#define LENSTYPE_AUDIO 20  // Audio files (.mp3, .flac, .wav, etc.)

// ============================================================================
// Extended format types
// ============================================================================

#define LENSTYPE_LZ4 21   // .lz4 LZ4 compressed archives
#define LENSTYPE_DJVU 22  // .djvu/.djv DjVu scanned documents
#define LENSTYPE_CHM 23   // .chm Compiled HTML Help
#define LENSTYPE_ODT 24   // .odt OpenDocument Text
#define LENSTYPE_ODP 25   // .odp OpenDocument Presentation

// ============================================================================
// LibArchive-backed format types
// ============================================================================

#define LENSTYPE_TAR_GZ 26   // .tar.gz / .tgz
#define LENSTYPE_TAR_BZ2 27  // .tar.bz2 / .tbz
#define LENSTYPE_TAR_XZ 28   // .tar.xz / .txz
#define LENSTYPE_TAR_ZST 29  // .tar.zst
#define LENSTYPE_CPIO 30     // .cpio

// ============================================================================
// Modern image format types
// ============================================================================

#define LENSTYPE_WEBP 40  // .webp Google WebP image
#define LENSTYPE_AVIF 41  // .avif AV1 Image Format
#define LENSTYPE_HEIC 42  // .heic High Efficiency Image Format
#define LENSTYPE_HEIF 43  // .heif HEIF container
#define LENSTYPE_JXL 44   // .jxl JPEG XL
#define LENSTYPE_TIFF 45  // .tif/.tiff Tagged Image File Format
#define LENSTYPE_SVG 46   // .svg Scalable Vector Graphics
#define LENSTYPE_RAW 47   // .dng/.cr2/.cr3/.nef/.arw Camera RAW formats

// ============================================================================
// Professional / Specialty image format types
// ============================================================================

#define LENSTYPE_PSD 48  // .psd/.psb Adobe Photoshop
#define LENSTYPE_DDS 49  // .dds DirectDraw Surface (game textures)
#define LENSTYPE_ISO 50  // .iso ISO 9660 CD/DVD images
#define LENSTYPE_XAR 51  // .xar macOS archives
#define LENSTYPE_AR 52   // .ar Unix archives
#define LENSTYPE_DEB 53  // .deb Debian packages
#define LENSTYPE_CAB 54  // .cab Microsoft Cabinet
#define LENSTYPE_HDR 55  // .hdr Radiance RGBE HDR
#define LENSTYPE_EXR 56  // .exr OpenEXR HDR image
#define LENSTYPE_PPM 57  // .ppm/.pgm/.pbm/.pnm Netpbm portable formats

// ============================================================================
// Standalone image format types
// ============================================================================

#define LENSTYPE_ICO 58  // .ico/.cur Windows icon/cursor
#define LENSTYPE_QOI 59  // .qoi Quite OK Image format

// ============================================================================
// Office document format types
// ============================================================================

#define LENSTYPE_DOCX 60  // .docx Microsoft Word
#define LENSTYPE_PPTX 61  // .pptx Microsoft PowerPoint
#define LENSTYPE_XLSX 62  // .xlsx Microsoft Excel
#define LENSTYPE_DOC 63   // .doc Legacy Microsoft Word
#define LENSTYPE_PPT 64   // .ppt Legacy Microsoft PowerPoint
#define LENSTYPE_XLS 65   // .xls Legacy Microsoft Excel

// ============================================================================
// Font format types
// ============================================================================

#define LENSTYPE_FONT 70  // .ttf/.otf/.woff/.woff2 Font files

// ============================================================================
// Additional image/media format types
// ============================================================================

#define LENSTYPE_TGA 75       // .tga Targa image format
#define LENSTYPE_BMP 76       // .bmp/.dib Windows bitmap
#define LENSTYPE_GIF 77       // .gif Graphics Interchange Format
#define LENSTYPE_MODEL 80     // .obj/.stl/.fbx/.gltf/.glb 3D model files
#define LENSTYPE_DOCUMENT 81  // Generic document type (fallback)
#define LENSTYPE_WMF 82       // .wmf Windows Metafile
#define LENSTYPE_EMF 83       // .emf Enhanced Metafile
#define LENSTYPE_PCX 84       // .pcx ZSoft PCX image
#define LENSTYPE_FARBFELD 85  // .ff Farbfeld image
#define LENSTYPE_JP2 86       // .jp2/.j2k/.j2c/.jpx JPEG 2000
#define LENSTYPE_EPS 87       // .eps/.epsf/.ps Encapsulated PostScript
#define LENSTYPE_KTX 88       // .ktx/.ktx2 Khronos KTX texture
#define LENSTYPE_VTF 89       // .vtf Valve Texture Format
#define LENSTYPE_ORA 90       // .ora OpenRaster image
#define LENSTYPE_XCF 91       // .xcf GIMP native image
#define LENSTYPE_SGI 92       // .sgi/.rgb/.rgba/.bw SGI image
#define LENSTYPE_XPM 93       // .xpm X PixMap image
#define LENSTYPE_JXR 94       // .jxr/.wdp/.hdp JPEG XR / HD Photo

// ============================================================================
// Image format identifiers (for detection within archives)
// ============================================================================

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

// ============================================================================
// Registry key for application settings
// ============================================================================

#define LENS_APP_KEY _T("Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}")

#endif  // _LENSTYPES_H_
