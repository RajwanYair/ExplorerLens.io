//==============================================================================
// ExplorerLens Engine — lensArchive.h Split — FormatTypes.h
// Centralized format type constants. Bridges legacy #define LENSTYPE and
// new enum class FormatType.
//==============================================================================
#pragma once
#include "FormatRegistry.h"
#include <string>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

/// Lookup table replacing the monolithic GetLENSType() if-else chain.
/// Maps lowercase extension (with dot) → FormatType.
class FormatTypeLookup {
public:
 static FormatTypeLookup& Instance() {
 static FormatTypeLookup s_instance;
 return s_instance;
 }

 FormatType Lookup(const std::wstring& ext) const {
 auto it = m_table.find(ext);
 return (it != m_table.end()) ? it->second : FormatType::Unknown;
 }

 size_t Count() const { return m_table.size(); }

 /// Initialize with all known extension → FormatType mappings
 void Initialize() {
 if (m_initialized) return;
 // Archives
 Add(L".zip", FormatType::ZIP); Add(L".cbz", FormatType::CBZ);
 Add(L".cbr", FormatType::CBR); Add(L".cb7", FormatType::CB7);
 Add(L".epub", FormatType::EPUB); Add(L".7z", FormatType::SevenZip);
 Add(L".rar", FormatType::RAR); Add(L".tar", FormatType::TAR);
 Add(L".gz", FormatType::GZIP); Add(L".tgz", FormatType::GZIP);
 Add(L".bz2", FormatType::BZIP2); Add(L".tbz", FormatType::BZIP2);
 Add(L".xz", FormatType::XZ); Add(L".txz", FormatType::XZ);
 Add(L".zst", FormatType::ZSTD); Add(L".tzst", FormatType::ZSTD);
 Add(L".cbt", FormatType::CBT); Add(L".lzma", FormatType::LZMA);
 Add(L".lz4", FormatType::LZ4); Add(L".iso", FormatType::ISO);
 Add(L".cab", FormatType::CAB); Add(L".cpio", FormatType::CPIO);
 Add(L".deb", FormatType::DEB); Add(L".xar", FormatType::XAR);
 Add(L".ar", FormatType::AR); Add(L".chm", FormatType::CHM);
 // Modern images
 Add(L".webp", FormatType::WebP); Add(L".avif", FormatType::AVIF);
 Add(L".avifs", FormatType::AVIF);
 Add(L".heic", FormatType::HEIC); Add(L".heif", FormatType::HEIF);
 Add(L".hif", FormatType::HEIF); Add(L".jxl", FormatType::JXL);
 // Professional images
 Add(L".psd", FormatType::PSD); Add(L".psb", FormatType::PSD);
 Add(L".dds", FormatType::DDS); Add(L".hdr", FormatType::HDR);
 Add(L".exr", FormatType::EXR); Add(L".tga", FormatType::TGA);
 Add(L".ico", FormatType::ICO); Add(L".cur", FormatType::ICO);
 Add(L".qoi", FormatType::QOI); Add(L".svg", FormatType::SVG);
 Add(L".svgz", FormatType::SVG);
 // Netpbm
 Add(L".ppm", FormatType::PPM); Add(L".pgm", FormatType::PGM);
 Add(L".pbm", FormatType::PBM); Add(L".pnm", FormatType::PNM);
 Add(L".pam", FormatType::PAM); Add(L".pfm", FormatType::PFM);
 // v8.4+ formats
 Add(L".wmf", FormatType::WMF); Add(L".emf", FormatType::EMF);
 Add(L".pcx", FormatType::PCX); Add(L".ff", FormatType::Farbfeld);
 Add(L".jp2", FormatType::JP2); Add(L".j2k", FormatType::JP2);
 Add(L".j2c", FormatType::JP2); Add(L".jpx", FormatType::JP2);
 Add(L".jph", FormatType::JP2); Add(L".eps", FormatType::EPS);
 Add(L".epsf", FormatType::EPS); Add(L".ps", FormatType::EPS);
 Add(L".ai", FormatType::EPS);
 Add(L".ktx", FormatType::KTX); Add(L".ktx2", FormatType::KTX);
 Add(L".vtf", FormatType::VTF);
 Add(L".ora", FormatType::ORA); Add(L".xcf", FormatType::XCF);
 Add(L".sgi", FormatType::SGI); Add(L".bw", FormatType::SGI);
 Add(L".rgb", FormatType::SGI); Add(L".rgba", FormatType::SGI);
 Add(L".xpm", FormatType::XPM);
 // Scientific
 Add(L".dcm", FormatType::DICOM); Add(L".dicom", FormatType::DICOM);
 Add(L".fits", FormatType::FITS); Add(L".fit", FormatType::FITS);
 Add(L".fts", FormatType::FITS);
 // Film
 Add(L".dpx", FormatType::DPX); Add(L".cin", FormatType::Cineon);
 // Text/data preview
 Add(L".md", FormatType::Markdown);
 Add(L".csv", FormatType::CSV); Add(L".tsv", FormatType::CSV);
 Add(L".json", FormatType::JSON); Add(L".yaml", FormatType::JSON);
 Add(L".yml", FormatType::JSON);
 // Documents, RAW, etc. remain in legacy GetLENSType() for backward compat
 m_initialized = true;
 }

 /// Validate: returns number of mapped extensions
 struct LookupStats {
 size_t totalMappings = 0;
 size_t archiveTypes = 0;
 size_t imageTypes = 0;
 };

 LookupStats GetStats() const {
 LookupStats s;
 s.totalMappings = m_table.size();
 for (auto& [ext, type] : m_table) {
 uint16_t v = static_cast<uint16_t>(type);
 if (v >= 1 && v <= 28) s.archiveTypes++;
 else if (v >= 40 && v <= 98) s.imageTypes++;
 }
 return s;
 }

private:
 FormatTypeLookup() { Initialize(); }
 void Add(const std::wstring& ext, FormatType type) { m_table[ext] = type; }
 std::unordered_map<std::wstring, FormatType> m_table;
 bool m_initialized = false;
};

}} // namespace ExplorerLens::Engine

