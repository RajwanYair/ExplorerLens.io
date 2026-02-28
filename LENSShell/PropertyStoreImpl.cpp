// PropertyStoreImpl.cpp — IPropertyStore implementation for Explorer Details
// Pane Copyright (c) 2026 ExplorerLens Project

#include "StdAfx.h"
#include "PropertyStoreImpl.h"
#include "LENSTypes.h"
#include <algorithm>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace ExplorerLens {

// ============================================================================
// Property Initialization
// ============================================================================

HRESULT CLENSPropertyStore::InitializeProperties(const wchar_t* filePath,
  int lensType) {
  if (m_initialized)
    return S_OK;

  m_properties.clear();

  if (!filePath)
    return E_POINTER;

  // PKEY_Software — Always set
  AddStringProperty(PKEY_Software, L"ExplorerLens v15.0.0");

  // PKEY_ItemTypeText — Human-readable format name
  std::wstring typeText = GetItemTypeText(lensType);
  if (!typeText.empty()) {
    AddStringProperty(PKEY_ItemTypeText, typeText.c_str());
  }

  // PKEY_MIMEType
  std::wstring mime = GetMimeTypeForExtension(filePath);
  if (!mime.empty()) {
    AddStringProperty(PKEY_MIMEType, mime.c_str());
  }

  // PKEY_Kind — "picture", "document", "video", "music", etc.
  std::wstring kind = GetKindString(lensType);
  if (!kind.empty()) {
    AddStringVectorProperty(PKEY_Kind, kind.c_str());
  }

  // PKEY_FileDescription — descriptive text
  std::wstring desc = typeText;
  if (IsArchiveType(lensType)) {
    desc += L" — Archive with embedded images";
  }
  else if (IsImageType(lensType)) {
    desc += L" — Image file";
  }
  if (!desc.empty()) {
    AddStringProperty(PKEY_FileDescription, desc.c_str());
  }

  m_initialized = true;
  return S_OK;
}

// ============================================================================
// IPropertyStore Implementation
// ============================================================================

HRESULT CLENSPropertyStore::PropertyStore_GetCount(DWORD* cProps) {
  if (!cProps)
    return E_POINTER;
  *cProps = static_cast<DWORD>(m_properties.size());
  return S_OK;
}

HRESULT CLENSPropertyStore::PropertyStore_GetAt(DWORD iProp,
  PROPERTYKEY* pkey) {
  if (!pkey)
    return E_POINTER;
  if (iProp >= m_properties.size())
    return E_INVALIDARG;
  *pkey = m_properties[iProp].key;
  return S_OK;
}

HRESULT CLENSPropertyStore::PropertyStore_GetValue(REFPROPERTYKEY key,
  PROPVARIANT* pv) {
  if (!pv)
    return E_POINTER;
  PropVariantInit(pv);

  const StoredProperty* prop = FindProperty(key);
  if (!prop) {
    return S_OK; // Property not found — return VT_EMPTY (normal behavior)
  }

  return PropVariantCopy(pv, &prop->value);
}

HRESULT CLENSPropertyStore::PropertyStore_SetValue(REFPROPERTYKEY /*key*/,
  REFPROPVARIANT /*propvar*/) {
  return STG_E_ACCESSDENIED; // Read-only property store
}

HRESULT CLENSPropertyStore::PropertyStore_Commit() {
  return S_OK; // Nothing to commit (read-only)
}

// ============================================================================
// IPropertyStoreCapabilities Implementation
// ============================================================================

HRESULT CLENSPropertyStore::PropertyStoreCapabilities_IsPropertyWritable(
  REFPROPERTYKEY /*key*/) {
  return S_FALSE; // All properties are read-only
}

// ============================================================================
// Helpers — Add Properties
// ============================================================================

void CLENSPropertyStore::AddStringProperty(REFPROPERTYKEY key,
  const wchar_t* value) {
  StoredProperty prop;
  prop.key = key;
  if (SUCCEEDED(InitPropVariantFromString(value, &prop.value))) {
    m_properties.push_back(std::move(prop));
  }
}

void CLENSPropertyStore::AddUInt32Property(REFPROPERTYKEY key, UINT32 value) {
  StoredProperty prop;
  prop.key = key;
  InitPropVariantFromUInt32(value, &prop.value);
  m_properties.push_back(std::move(prop));
}

void CLENSPropertyStore::AddStringVectorProperty(REFPROPERTYKEY key,
  const wchar_t* value) {
  StoredProperty prop;
  prop.key = key;
  const wchar_t* vals[] = { value };
  if (SUCCEEDED(InitPropVariantFromStringAsVector(value, &prop.value))) {
    m_properties.push_back(std::move(prop));
  }
}

const CLENSPropertyStore::StoredProperty*
CLENSPropertyStore::FindProperty(REFPROPERTYKEY key) const {
  for (const auto& prop : m_properties) {
    if (IsEqualPropertyKey(prop.key, key)) {
      return &prop;
    }
  }
  return nullptr;
}

// ============================================================================
// Static Helpers — Format Metadata
// ============================================================================

std::wstring
CLENSPropertyStore::GetMimeTypeForExtension(const wchar_t* filePath) {
  if (!filePath)
    return L"";

  const wchar_t* ext = PathFindExtensionW(filePath);
  if (!ext || !*ext)
    return L"";

  // Convert to lowercase for comparison
  std::wstring lext(ext);
  std::transform(lext.begin(), lext.end(), lext.begin(), ::towlower);

  // Archives
  if (lext == L".zip" || lext == L".cbz")
    return L"application/zip";
  if (lext == L".rar" || lext == L".cbr")
    return L"application/x-rar-compressed";
  if (lext == L".7z" || lext == L".cb7")
    return L"application/x-7z-compressed";
  if (lext == L".tar")
    return L"application/x-tar";
  if (lext == L".gz" || lext == L".tgz")
    return L"application/gzip";
  if (lext == L".bz2")
    return L"application/x-bzip2";
  if (lext == L".xz")
    return L"application/x-xz";
  if (lext == L".zst")
    return L"application/zstd";
  if (lext == L".lz4")
    return L"application/x-lz4";

  // Images
  if (lext == L".webp")
    return L"image/webp";
  if (lext == L".avif")
    return L"image/avif";
  if (lext == L".heic")
    return L"image/heic";
  if (lext == L".heif")
    return L"image/heif";
  if (lext == L".jxl")
    return L"image/jxl";
  if (lext == L".tiff" || lext == L".tif")
    return L"image/tiff";
  if (lext == L".svg")
    return L"image/svg+xml";
  if (lext == L".psd" || lext == L".psb")
    return L"image/vnd.adobe.photoshop";
  if (lext == L".dds")
    return L"image/vnd.ms-dds";
  if (lext == L".hdr")
    return L"image/vnd.radiance";
  if (lext == L".exr")
    return L"image/x-exr";
  if (lext == L".tga")
    return L"image/x-targa";
  if (lext == L".bmp" || lext == L".dib")
    return L"image/bmp";
  if (lext == L".gif")
    return L"image/gif";
  if (lext == L".ico" || lext == L".cur")
    return L"image/x-icon";
  if (lext == L".qoi")
    return L"image/x-qoi";
  if (lext == L".jp2" || lext == L".j2k")
    return L"image/jp2";
  if (lext == L".pcx")
    return L"image/x-pcx";
  if (lext == L".wmf")
    return L"image/wmf";
  if (lext == L".emf")
    return L"image/emf";
  if (lext == L".ppm" || lext == L".pgm" || lext == L".pbm")
    return L"image/x-portable-anymap";
  if (lext == L".xcf")
    return L"image/x-xcf";
  if (lext == L".ora")
    return L"image/openraster";
  if (lext == L".xpm")
    return L"image/x-xpixmap";

  // Camera RAW
  if (lext == L".dng")
    return L"image/x-adobe-dng";
  if (lext == L".cr2" || lext == L".cr3")
    return L"image/x-canon-cr2";
  if (lext == L".nef")
    return L"image/x-nikon-nef";
  if (lext == L".arw")
    return L"image/x-sony-arw";
  if (lext == L".orf")
    return L"image/x-olympus-orf";
  if (lext == L".rw2")
    return L"image/x-panasonic-rw2";
  if (lext == L".raf")
    return L"image/x-fuji-raf";

  // Documents
  if (lext == L".pdf")
    return L"application/pdf";
  if (lext == L".epub")
    return L"application/epub+zip";
  if (lext == L".docx")
    return L"application/"
    L"vnd.openxmlformats-officedocument.wordprocessingml.document";
  if (lext == L".pptx")
    return L"application/"
    L"vnd.openxmlformats-officedocument.presentationml.presentation";
  if (lext == L".xlsx")
    return L"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  if (lext == L".djvu" || lext == L".djv")
    return L"image/vnd.djvu";
  if (lext == L".chm")
    return L"application/vnd.ms-htmlhelp";

  // eBooks
  if (lext == L".mobi")
    return L"application/x-mobipocket-ebook";
  if (lext == L".fb2")
    return L"application/x-fictionbook";
  if (lext == L".azw" || lext == L".azw3")
    return L"application/vnd.amazon.ebook";

  // Video
  if (lext == L".mp4" || lext == L".m4v")
    return L"video/mp4";
  if (lext == L".mkv")
    return L"video/x-matroska";
  if (lext == L".avi")
    return L"video/x-msvideo";
  if (lext == L".mov")
    return L"video/quicktime";
  if (lext == L".wmv")
    return L"video/x-ms-wmv";
  if (lext == L".webm")
    return L"video/webm";
  if (lext == L".flv")
    return L"video/x-flv";

  // Audio
  if (lext == L".mp3")
    return L"audio/mpeg";
  if (lext == L".flac")
    return L"audio/flac";
  if (lext == L".wav")
    return L"audio/wav";
  if (lext == L".ogg")
    return L"audio/ogg";
  if (lext == L".aac")
    return L"audio/aac";
  if (lext == L".wma")
    return L"audio/x-ms-wma";

  // Fonts
  if (lext == L".ttf")
    return L"font/ttf";
  if (lext == L".otf")
    return L"font/otf";
  if (lext == L".woff")
    return L"font/woff";
  if (lext == L".woff2")
    return L"font/woff2";

  // 3D Models
  if (lext == L".gltf")
    return L"model/gltf+json";
  if (lext == L".glb")
    return L"model/gltf-binary";
  if (lext == L".obj")
    return L"model/obj";
  if (lext == L".stl")
    return L"model/stl";
  if (lext == L".fbx")
    return L"application/octet-stream";

  return L"application/octet-stream";
}

std::wstring CLENSPropertyStore::GetItemTypeText(int lensType) {
  switch (lensType) {
    // Archives
  case LENSTYPE_ZIP:
    return L"ZIP Archive";
  case LENSTYPE_CBZ:
    return L"Comic Book (ZIP)";
  case LENSTYPE_7Z:
    return L"7-Zip Archive";
  case LENSTYPE_CB7:
    return L"Comic Book (7Z)";
  case LENSTYPE_TAR:
    return L"Tape Archive";
  case LENSTYPE_CBT:
    return L"Comic Book (TAR)";
  case LENSTYPE_BZIP2:
    return L"BZIP2 Compressed Archive";
  case LENSTYPE_ZSTD:
    return L"Zstandard Compressed Archive";
  case LENSTYPE_LZMA:
    return L"LZMA/XZ Compressed Archive";
  case LENSTYPE_LZ4:
    return L"LZ4 Compressed Archive";
  case LENSTYPE_TAR_GZ:
    return L"Gzipped Tape Archive";
  case LENSTYPE_TAR_BZ2:
    return L"BZip2'd Tape Archive";
  case LENSTYPE_TAR_XZ:
    return L"XZ Tape Archive";
  case LENSTYPE_TAR_ZST:
    return L"Zstd Tape Archive";
  case LENSTYPE_CPIO:
    return L"CPIO Archive";
  case LENSTYPE_ISO:
    return L"ISO 9660 Disc Image";
  case LENSTYPE_XAR:
    return L"XAR Archive";
  case LENSTYPE_AR:
    return L"Unix AR Archive";
  case LENSTYPE_DEB:
    return L"Debian Package";
  case LENSTYPE_CAB:
    return L"Microsoft Cabinet";

    // eBooks
  case LENSTYPE_EPUB:
    return L"EPUB eBook";
  case LENSTYPE_MOBI:
    return L"Mobipocket eBook";
  case LENSTYPE_FB2:
    return L"FictionBook 2";
  case LENSTYPE_AZW:
    return L"Amazon Kindle (AZW)";
  case LENSTYPE_AZW3:
    return L"Amazon Kindle (AZW3)";

    // Images — Modern
  case LENSTYPE_WEBP:
    return L"WebP Image";
  case LENSTYPE_AVIF:
    return L"AVIF Image (AV1)";
  case LENSTYPE_HEIC:
    return L"HEIC Image (HEVC)";
  case LENSTYPE_HEIF:
    return L"HEIF Image";
  case LENSTYPE_JXL:
    return L"JPEG XL Image";
  case LENSTYPE_TIFF:
    return L"TIFF Image";
  case LENSTYPE_SVG:
    return L"SVG Vector Image";
  case LENSTYPE_RAW:
    return L"Camera RAW Image";
  case LENSTYPE_PSD:
    return L"Adobe Photoshop Document";
  case LENSTYPE_DDS:
    return L"DirectDraw Surface Texture";
  case LENSTYPE_HDR:
    return L"Radiance HDR Image";
  case LENSTYPE_EXR:
    return L"OpenEXR HDR Image";
  case LENSTYPE_PPM:
    return L"Netpbm Portable Image";
  case LENSTYPE_ICO:
    return L"Windows Icon/Cursor";
  case LENSTYPE_QOI:
    return L"QOI Image (Quite OK)";
  case LENSTYPE_TGA:
    return L"Targa Image";
  case LENSTYPE_BMP:
    return L"Windows Bitmap";
  case LENSTYPE_GIF:
    return L"GIF Image";
  case LENSTYPE_WMF:
    return L"Windows Metafile";
  case LENSTYPE_EMF:
    return L"Enhanced Metafile";
  case LENSTYPE_PCX:
    return L"PCX Image";
  case LENSTYPE_FARBFELD:
    return L"Farbfeld Image";
  case LENSTYPE_JP2:
    return L"JPEG 2000 Image";
  case LENSTYPE_EPS:
    return L"Encapsulated PostScript";
  case LENSTYPE_KTX:
    return L"Khronos KTX Texture";
  case LENSTYPE_VTF:
    return L"Valve Texture Format";
  case LENSTYPE_ORA:
    return L"OpenRaster Image";
  case LENSTYPE_XCF:
    return L"GIMP Native Image";
  case LENSTYPE_SGI:
    return L"SGI Image";
  case LENSTYPE_XPM:
    return L"X PixMap Image";

    // Documents
  case LENSTYPE_PDF:
    return L"PDF Document";
  case LENSTYPE_DJVU:
    return L"DjVu Document";
  case LENSTYPE_CHM:
    return L"Compiled HTML Help";
  case LENSTYPE_ODT:
    return L"OpenDocument Text";
  case LENSTYPE_ODP:
    return L"OpenDocument Presentation";
  case LENSTYPE_DOCX:
    return L"Microsoft Word Document";
  case LENSTYPE_PPTX:
    return L"Microsoft PowerPoint";
  case LENSTYPE_XLSX:
    return L"Microsoft Excel Spreadsheet";
  case LENSTYPE_DOC:
    return L"Legacy Word Document";
  case LENSTYPE_PPT:
    return L"Legacy PowerPoint";
  case LENSTYPE_XLS:
    return L"Legacy Excel Spreadsheet";

    // Media
  case LENSTYPE_VIDEO:
    return L"Video File";
  case LENSTYPE_AUDIO:
    return L"Audio File";

    // Fonts
  case LENSTYPE_FONT:
    return L"Font File";

    // 3D
  case LENSTYPE_MODEL:
    return L"3D Model";

  default:
    return L"Unknown Format";
  }
}

std::wstring CLENSPropertyStore::GetKindString(int lensType) {
  if (IsImageType(lensType))
    return L"picture";

  switch (lensType) {
  case LENSTYPE_VIDEO:
    return L"video";
  case LENSTYPE_AUDIO:
    return L"music";
  case LENSTYPE_FONT:
    return L"font";
  case LENSTYPE_PDF:
  case LENSTYPE_DOCX:
  case LENSTYPE_DOC:
  case LENSTYPE_PPTX:
  case LENSTYPE_PPT:
  case LENSTYPE_XLSX:
  case LENSTYPE_XLS:
  case LENSTYPE_ODT:
  case LENSTYPE_ODP:
  case LENSTYPE_DJVU:
  case LENSTYPE_CHM:
    return L"document";
  case LENSTYPE_EPUB:
  case LENSTYPE_MOBI:
  case LENSTYPE_FB2:
  case LENSTYPE_AZW:
  case LENSTYPE_AZW3:
    return L"document";
  default:
    return L"document";
  }
}

bool CLENSPropertyStore::IsImageType(int lensType) {
  return (lensType >= LENSTYPE_WEBP && lensType <= LENSTYPE_RAW) ||
    (lensType >= LENSTYPE_PSD && lensType <= LENSTYPE_PPM) ||
    (lensType >= LENSTYPE_ICO && lensType <= LENSTYPE_QOI) ||
    (lensType >= LENSTYPE_TGA && lensType <= LENSTYPE_XPM) ||
    lensType == LENSTYPE_HDR || lensType == LENSTYPE_EXR;
}

bool CLENSPropertyStore::IsArchiveType(int lensType) {
  return lensType == LENSTYPE_ZIP || lensType == LENSTYPE_CBZ ||
    lensType == LENSTYPE_7Z || lensType == LENSTYPE_CB7 ||
    lensType == LENSTYPE_TAR || lensType == LENSTYPE_CBT ||
    (lensType >= LENSTYPE_BZIP2 && lensType <= LENSTYPE_LZMA) ||
    lensType == LENSTYPE_LZ4 ||
    (lensType >= LENSTYPE_TAR_GZ && lensType <= LENSTYPE_CPIO) ||
    lensType == LENSTYPE_ISO || lensType == LENSTYPE_XAR ||
    lensType == LENSTYPE_AR || lensType == LENSTYPE_DEB ||
    lensType == LENSTYPE_CAB;
}

} // namespace ExplorerLens
