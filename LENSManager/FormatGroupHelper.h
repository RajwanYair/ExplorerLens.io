// FormatGroupHelper.h — Format categorization and group management
// Copyright (c) 2026 ExplorerLens Project
//
// Provides logical grouping of 35+ format types into categories for
// organized display in the Settings UI. Used by the dialog to implement
// Select All/None per category and visual section headers.
#pragma once

#include <string>
#include <vector>

namespace ExplorerLens {

// ============================================================================
// Format group categories
// ============================================================================

enum class FormatCategory {
  ARCHIVES,      // ZIP, RAR, 7Z, TAR, compressed
  COMIC_BOOKS,   // CBZ, CBR, CB7, CBT
  EBOOKS,        // EPUB, MOBI, FB2, AZW, AZW3, PHZ
  MODERN_IMAGES, // WebP, AVIF, HEIC, JXL, TIFF, SVG
  RAW_IMAGES,    // Camera RAW (DNG, CR2, NEF, ARW, etc.)
  LEGACY_IMAGES, // BMP, GIF, TGA, PCX, ICO, WMF, EMF
  HDR_IMAGES,    // HDR, EXR
  PROFESSIONAL,  // PSD, DDS, JP2, KTX, VTF, ORA, XCF
  DOCUMENTS,     // PDF, DOCX, PPTX, XLSX, DJVU, CHM
  MEDIA,         // Video, Audio
  FONTS,         // TTF, OTF, WOFF
  MODELS_3D,     // OBJ, STL, FBX, glTF
  COUNT
};

struct FormatEntry {
  int ctrlId;          // Dialog control ID (IDC_CB_*)
  int lensType;        // LENS_* type from RegManager.h
  const wchar_t *name; // Human-readable name
  const wchar_t *ext;  // Primary extension (e.g., L".webp")
  FormatCategory group;
};

struct FormatGroup {
  FormatCategory category;
  const wchar_t *name;
  const wchar_t *description;
};

// ============================================================================
// Static group definitions
// ============================================================================

inline const FormatGroup FORMAT_GROUPS[] = {
    {FormatCategory::ARCHIVES, L"Archives",
     L"ZIP, RAR, 7z, TAR, compressed archives"},
    {FormatCategory::COMIC_BOOKS, L"Comic Books",
     L"CBZ, CBR, CB7, CBT comic archives"},
    {FormatCategory::EBOOKS, L"eBooks", L"EPUB, MOBI, FB2, AZW Kindle books"},
    {FormatCategory::MODERN_IMAGES, L"Modern Images",
     L"WebP, AVIF, HEIC, JPEG XL, SVG"},
    {FormatCategory::RAW_IMAGES, L"Camera RAW",
     L"DNG, CR2, NEF, ARW raw files"},
    {FormatCategory::LEGACY_IMAGES, L"Classic Images",
     L"BMP, GIF, TGA, PCX, ICO, WMF"},
    {FormatCategory::HDR_IMAGES, L"HDR Images", L"EXR, HDR high dynamic range"},
    {FormatCategory::PROFESSIONAL, L"Professional",
     L"PSD, JPEG 2000, DDS textures"},
    {FormatCategory::DOCUMENTS, L"Documents", L"PDF, Office, DjVu, CHM"},
    {FormatCategory::MEDIA, L"Media", L"Video and audio files"},
    {FormatCategory::FONTS, L"Fonts", L"TrueType, OpenType, WOFF"},
    {FormatCategory::MODELS_3D, L"3D Models", L"OBJ, STL, FBX, glTF"},
};

constexpr size_t FORMAT_GROUP_COUNT =
    static_cast<size_t>(FormatCategory::COUNT);

// ============================================================================
// Utility: Get group info
// ============================================================================

inline const FormatGroup *GetGroupInfo(FormatCategory cat) {
  for (const auto &g : FORMAT_GROUPS) {
    if (g.category == cat)
      return &g;
  }
  return nullptr;
}

// ============================================================================
// Utility: Filter format entries by category
// ============================================================================

inline std::vector<const FormatEntry *>
GetFormatsInGroup(const FormatEntry *entries, size_t count,
                  FormatCategory cat) {
  std::vector<const FormatEntry *> result;
  for (size_t i = 0; i < count; ++i) {
    if (entries[i].group == cat) {
      result.push_back(&entries[i]);
    }
  }
  return result;
}

} // namespace ExplorerLens
