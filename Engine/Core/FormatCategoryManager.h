// FormatCategoryManager.h — Categorized Format Group Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Organizes 200+ supported file formats into logical categories for the
// LENSManager GUI. Supports collapsible groups, Select All/None per group,
// and search/filter. Replaces flat checkbox list with structured hierarchy.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Format category enumeration (GUI groups — distinct from
/// FormatRegistry::FormatCategoryGroup)
enum class FormatCategoryGroup : uint8_t {
 Archives = 0, ///< ZIP, 7Z, RAR, TAR, GZ, etc.
 Comics = 1, ///< CBZ, CBR, CB7, CBT
 EBooks = 2, ///< EPUB, MOBI, FB2
 Images = 3, ///< WebP, AVIF, JXL, HEIF, PSD, etc.
 RawPhotos = 4, ///< CR2, NEF, ARW, DNG, etc.
 Video = 5, ///< MKV, MP4, AVI, WebM, etc.
 Audio = 6, ///< MP3, FLAC, OGG, WAV
 Documents = 7, ///< PDF, DOCX, XLSX, PPTX, TXT, MD
 Fonts = 8, ///< TTF, OTF, WOFF, WOFF2
 Models3D = 9, ///< STL, OBJ, FBX, glTF, USDZ
 Scientific = 10, ///< FITS, DICOM, NIfTI, HDF5
 Geospatial = 11, ///< GeoTIFF, Shapefile, KML
 Specialized = 12, ///< DDS, VTF, KTX, EXR, HDR, DPX
 CategoryCount = 13
};

/// A format entry within a category
struct FormatCategoryEntry {
 const char *extension = nullptr; ///< e.g., ".webp"
 const char *displayName = nullptr; ///< e.g., "WebP Image"
 const char *decoderName = nullptr; ///< e.g., "WebPDecoder"
 int lensType = 0; ///< LENSTYPE_* constant
 int checkboxId = 0; ///< IDC_CB_* resource ID
 bool isEnabled = true; ///< Currently registered
 bool isAvailable = true; ///< Decoder functional
 bool isGPUAccelerated = false; ///< Uses GPU decode path
};

/// A format category with its entries
struct FormatGroup {
 FormatCategoryGroup category = FormatCategoryGroup::Archives;
 const char *name = nullptr;
 const char *icon = nullptr; ///< Icon resource name
 std::vector<FormatCategoryEntry> formats;
 bool isExpanded = true;
 bool isAllEnabled = false;
};

/// Format category manager for GUI organization
class FormatCategoryManager {
public:
 static FormatCategoryManager &Instance() {
 static FormatCategoryManager instance;
 return instance;
 }

 /// Get all format groups
 const std::vector<FormatGroup> &GetGroups() const { return m_groups; }

 /// Get a specific group
 const FormatGroup *GetGroup(FormatCategoryGroup cat) const {
 auto idx = static_cast<size_t>(cat);
 return idx < m_groups.size() ? &m_groups[idx] : nullptr;
 }

 /// Count total formats across all categories
 uint32_t GetTotalFormatCount() const {
 uint32_t count = 0;
 for (const auto &g : m_groups)
 count += static_cast<uint32_t>(g.formats.size());
 return count;
 }

 /// Count enabled formats
 uint32_t GetEnabledCount() const {
 uint32_t count = 0;
 for (const auto &g : m_groups)
 for (const auto &f : g.formats)
 if (f.isEnabled)
 ++count;
 return count;
 }

 /// Toggle all formats in a category
 void SetCategoryEnabled(FormatCategoryGroup cat, bool enabled) {
 auto idx = static_cast<size_t>(cat);
 if (idx >= m_groups.size())
 return;
 for (auto &f : m_groups[idx].formats)
 f.isEnabled = enabled;
 m_groups[idx].isAllEnabled = enabled;
 }

 /// Search formats by name/extension
 std::vector<const FormatCategoryEntry *> Search(const char *query) const {
 std::vector<const FormatCategoryEntry *> results;
 if (!query)
 return results;
 std::string q(query);
 for (auto &c : q)
 c = static_cast<char>(tolower(c));
 for (const auto &g : m_groups) {
 for (const auto &f : g.formats) {
 std::string ext(f.extension ? f.extension : "");
 std::string name(f.displayName ? f.displayName : "");
 for (auto &c : ext)
 c = static_cast<char>(tolower(c));
 for (auto &c : name)
 c = static_cast<char>(tolower(c));
 if (ext.find(q) != std::string::npos ||
 name.find(q) != std::string::npos) {
 results.push_back(&f);
 }
 }
 }
 return results;
 }

 /// Category name lookup
 static const wchar_t *CategoryName(FormatCategoryGroup c) {
 switch (c) {
 case FormatCategoryGroup::Archives:
 return L"Archives";
 case FormatCategoryGroup::Comics:
 return L"Comics";
 case FormatCategoryGroup::EBooks:
 return L"eBooks";
 case FormatCategoryGroup::Images:
 return L"Images";
 case FormatCategoryGroup::RawPhotos:
 return L"RAW Photos";
 case FormatCategoryGroup::Video:
 return L"Video";
 case FormatCategoryGroup::Audio:
 return L"Audio";
 case FormatCategoryGroup::Documents:
 return L"Documents";
 case FormatCategoryGroup::Fonts:
 return L"Fonts";
 case FormatCategoryGroup::Models3D:
 return L"3D Models";
 case FormatCategoryGroup::Scientific:
 return L"Scientific";
 case FormatCategoryGroup::Geospatial:
 return L"Geospatial";
 case FormatCategoryGroup::Specialized:
 return L"Specialized";
 default:
 return L"Unknown";
 }
 }

 static constexpr uint32_t CategoryCount() {
 return static_cast<uint32_t>(FormatCategoryGroup::CategoryCount);
 }

 static constexpr uint32_t GetCategoryCount() { return CategoryCount(); }

private:
 FormatCategoryManager() { BuildGroups(); }

 void BuildGroups() {
 m_groups.resize(static_cast<size_t>(FormatCategoryGroup::CategoryCount));

 auto &archives = m_groups[0];
 archives.category = FormatCategoryGroup::Archives;
 archives.name = "Archives";
 archives.formats = {
 {".zip", "ZIP Archive", "ArchiveDecoder", 1, 0, true, true, false},
 {".7z", "7-Zip Archive", "ArchiveDecoder", 6, 0, true, true, false},
 {".rar", "RAR Archive", "ArchiveDecoder", 3, 0, true, true, false},
 {".tar", "TAR Archive", "ArchiveDecoder", 8, 0, true, true, false},
 {".gz", "GZip Archive", "ArchiveDecoder", 0, 0, true, true, false},
 };

 auto &comics = m_groups[1];
 comics.category = FormatCategoryGroup::Comics;
 comics.name = "Comics";
 comics.formats = {
 {".cbz", "Comic Book ZIP", "ArchiveDecoder", 2, 0, true, true, false},
 {".cbr", "Comic Book RAR", "ArchiveDecoder", 4, 0, true, true, false},
 {".cb7", "Comic Book 7Z", "ArchiveDecoder", 7, 0, true, true, false},
 {".cbt", "Comic Book TAR", "ArchiveDecoder", 9, 0, true, true, false},
 };

 auto &ebooks = m_groups[2];
 ebooks.category = FormatCategoryGroup::EBooks;
 ebooks.name = "eBooks";
 ebooks.formats = {
 {".epub", "EPUB eBook", "EBookDecoder", 5, 0, true, true, false},
 {".mobi", "Kindle MOBI", "EBookDecoder", 10, 0, true, true, false},
 {".fb2", "FictionBook 2", "EBookDecoder", 11, 0, true, true, false},
 };

 auto &images = m_groups[3];
 images.category = FormatCategoryGroup::Images;
 images.name = "Images";
 images.formats = {
 {".webp", "WebP Image", "WebPDecoder", 0, 0, true, true, true},
 {".avif", "AVIF Image", "AVIFDecoder", 0, 0, true, true, true},
 {".jxl", "JPEG XL", "JXLDecoder", 0, 0, true, true, false},
 {".heif", "HEIF Image", "HEIFDecoder", 0, 0, true, true, false},
 {".heic", "HEIC Image", "HEIFDecoder", 0, 0, true, true, false},
 {".psd", "Photoshop", "PSDDecoder", 0, 0, true, true, false},
 {".svg", "SVG Vector", "SVGDecoder", 0, 0, true, true, false},
 {".qoi", "QOI Image", "QOIDecoder", 0, 0, true, true, false},
 };

 // Initialize remaining groups with names
 m_groups[4] = {
 FormatCategoryGroup::RawPhotos, "RAW Photos", nullptr, {}, true, false};
 m_groups[5] = {
 FormatCategoryGroup::Video, "Video", nullptr, {}, true, false};
 m_groups[6] = {
 FormatCategoryGroup::Audio, "Audio", nullptr, {}, true, false};
 m_groups[7] = {
 FormatCategoryGroup::Documents, "Documents", nullptr, {}, true, false};
 m_groups[8] = {
 FormatCategoryGroup::Fonts, "Fonts", nullptr, {}, true, false};
 m_groups[9] = {
 FormatCategoryGroup::Models3D, "3D Models", nullptr, {}, true, false};
 m_groups[10] = {FormatCategoryGroup::Scientific,
 "Scientific",
 nullptr,
 {},
 true,
 false};
 m_groups[11] = {FormatCategoryGroup::Geospatial,
 "Geospatial",
 nullptr,
 {},
 true,
 false};
 m_groups[12] = {FormatCategoryGroup::Specialized,
 "Specialized",
 nullptr,
 {},
 true,
 false};
 }

 std::vector<FormatGroup> m_groups;
};

} // namespace Engine
} // namespace ExplorerLens
