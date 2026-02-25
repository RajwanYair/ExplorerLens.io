//==============================================================================
// FormatGroupManager.h — Format Group Manager
// Organizes file formats into collapsible UI categories.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Manages categorized format groups for the Manager UI.
class FormatGroupManager {
public:
  enum class FormatGroup {
    Archives,
    Comics,
    EBooks,
    Images,
    RawPhotos,
    Media,
    Documents,
    Scientific,
    ThreeD,
    Fonts,
    Specialized,
    COUNT
  };

  enum class GroupAction {
    SelectAll,
    SelectNone,
    Toggle,
    Expand,
    Collapse,
    COUNT
  };

  struct GroupInfo {
    FormatGroup group;
    std::wstring displayName;
    uint32_t formatCount;
    bool expanded;
  };

  static const wchar_t *GroupName(FormatGroup g) {
    switch (g) {
    case FormatGroup::Archives:
      return L"Archives";
    case FormatGroup::Comics:
      return L"Comics";
    case FormatGroup::EBooks:
      return L"eBooks";
    case FormatGroup::Images:
      return L"Images";
    case FormatGroup::RawPhotos:
      return L"Raw Photos";
    case FormatGroup::Media:
      return L"Media";
    case FormatGroup::Documents:
      return L"Documents";
    case FormatGroup::Scientific:
      return L"Scientific";
    case FormatGroup::ThreeD:
      return L"3D Models";
    case FormatGroup::Fonts:
      return L"Fonts";
    case FormatGroup::Specialized:
      return L"Specialized";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *ActionName(GroupAction a) {
    switch (a) {
    case GroupAction::SelectAll:
      return L"SelectAll";
    case GroupAction::SelectNone:
      return L"SelectNone";
    case GroupAction::Toggle:
      return L"Toggle";
    case GroupAction::Expand:
      return L"Expand";
    case GroupAction::Collapse:
      return L"Collapse";
    default:
      return L"Unknown";
    }
  }

  static size_t GroupCount() { return static_cast<size_t>(FormatGroup::COUNT); }
  static size_t ActionCount() {
    return static_cast<size_t>(GroupAction::COUNT);
  }

  static std::vector<GroupInfo> GetGroups() {
    return {
        {FormatGroup::Archives, L"Archives (ZIP, RAR, 7Z, TAR)", 8, true},
        {FormatGroup::Comics, L"Comics (CBZ, CBR, CB7)", 3, true},
        {FormatGroup::EBooks, L"eBooks (EPUB, MOBI, FB2)", 5, true},
        {FormatGroup::Images, L"Images (WebP, JXL, AVIF, HEIF)", 15, true},
        {FormatGroup::RawPhotos, L"Raw Photos (CR3, ARW, NEF)", 12, true},
        {FormatGroup::Media, L"Media (MP4, MKV, FLAC, MP3)", 10, true},
        {FormatGroup::Documents, L"Documents (PDF, DOCX, XLSX)", 8, true},
        {FormatGroup::Scientific, L"Scientific (DICOM, FITS, HDF5)", 6, true},
        {FormatGroup::ThreeD, L"3D Models (OBJ, STL, FBX)", 8, true},
        {FormatGroup::Fonts, L"Fonts (TTF, OTF, WOFF)", 4, true},
        {FormatGroup::Specialized, L"Specialized (DDS, VTF, KTX)", 6, true},
    };
  }

  static uint32_t TotalFormats() {
    uint32_t total = 0;
    for (const auto &g : GetGroups())
      total += g.formatCount;
    return total;
  }
};

} // namespace Engine
} // namespace ExplorerLens
