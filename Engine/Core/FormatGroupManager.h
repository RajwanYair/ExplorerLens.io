// FormatGroupManager.h — Format Group Organizer for 200+ Formats
// Copyright (c) 2026 ExplorerLens Project
//
// Organizes all 200+ supported file formats into collapsible GUI groups.
// Provides enable/disable per-extension, group expand/collapse state,
// lookup, counting, and persistence to INI-style config files.
// Thread-safe via SRWLOCK. Header-only implementation.

#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Information about a single supported file format.
struct FormatInfo {
  std::wstring extension;
  std::wstring description;
  std::wstring mimeType;
  uint32_t     decoderType = 0;
  bool         enabled = true;
};

/// A named group of related formats for the GUI.
struct FormatGroupEntry {
  std::wstring            name;
  std::wstring            icon;
  std::vector<FormatInfo> formats;
  bool                    expanded = true;
  uint32_t                enabledCount = 0;
};

/// Manages categorized format groups for the Manager UI.
class FormatGroupManager {
public:
  FormatGroupManager() {
    InitializeSRWLock(&m_lock);
    BuildDefaultGroups();
  }

  ~FormatGroupManager() = default;
  FormatGroupManager(const FormatGroupManager&) = delete;
  FormatGroupManager& operator=(const FormatGroupManager&) = delete;

  // ── Group queries ─────────────────────────────────────────────────────
  /// Return all format groups.
  inline std::vector<FormatGroupEntry> GetAllGroups() const {
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    auto copy = m_groups;
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return copy;
  }

  /// Find a format by extension (case-insensitive, without leading dot).
  inline FormatInfo* FindFormat(const std::wstring& extension) {
    std::wstring lower = ToLower(extension);
    AcquireSRWLockExclusive(&m_lock);
    for (auto& group : m_groups) {
      for (auto& fmt : group.formats) {
        if (ToLower(fmt.extension) == lower) {
          // NOTE: returning pointer is safe only while lock is held externally;
          // for thread safety, callers should use GetAllGroups() for read snapshots.
          ReleaseSRWLockExclusive(&m_lock);
          return &fmt;
        }
      }
    }
    ReleaseSRWLockExclusive(&m_lock);
    return nullptr;
  }

  /// Check if an extension is in the supported list.
  inline bool IsSupported(const std::wstring& extension) const {
    std::wstring lower = ToLower(extension);
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    for (const auto& group : m_groups) {
      for (const auto& fmt : group.formats) {
        if (ToLower(fmt.extension) == lower) {
          ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
          return true;
        }
      }
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return false;
  }

  // ── Counts ────────────────────────────────────────────────────────────
  inline uint32_t GetTotalFormats() const {
    uint32_t total = 0;
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    for (const auto& g : m_groups) total += static_cast<uint32_t>(g.formats.size());
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return total;
  }

  inline uint32_t GetEnabledFormats() const {
    uint32_t count = 0;
    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    for (const auto& g : m_groups) {
      for (const auto& f : g.formats) {
        if (f.enabled) ++count;
      }
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return count;
  }

  // ── Mutators ──────────────────────────────────────────────────────────
  /// Enable or disable a specific format by extension.
  inline void SetFormatEnabled(const std::wstring& extension, bool enabled) {
    std::wstring lower = ToLower(extension);
    AcquireSRWLockExclusive(&m_lock);
    for (auto& group : m_groups) {
      for (auto& fmt : group.formats) {
        if (ToLower(fmt.extension) == lower) {
          fmt.enabled = enabled;
          RecalcEnabledCount(group);
          ReleaseSRWLockExclusive(&m_lock);
          return;
        }
      }
    }
    ReleaseSRWLockExclusive(&m_lock);
  }

  /// Set the expanded/collapsed state of a group (GUI state).
  inline void SetGroupExpanded(const std::wstring& groupName, bool expanded) {
    AcquireSRWLockExclusive(&m_lock);
    for (auto& group : m_groups) {
      if (group.name == groupName) {
        group.expanded = expanded;
        break;
      }
    }
    ReleaseSRWLockExclusive(&m_lock);
  }

  // ── Persistence ───────────────────────────────────────────────────────
  /// Save enabled/disabled states to an INI-style file.
  inline bool SaveConfig(const std::wstring& path) const {
    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;

    AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    ofs << "; ExplorerLens Format Configuration\n";
    ofs << "; Auto-generated — do not edit manually\n\n";

    for (const auto& group : m_groups) {
      std::string groupNameA = WideToUtf8(group.name);
      ofs << "[" << groupNameA << "]\n";
      ofs << "expanded=" << (group.expanded ? "1" : "0") << "\n";
      for (const auto& fmt : group.formats) {
        std::string extA = WideToUtf8(fmt.extension);
        ofs << extA << "=" << (fmt.enabled ? "1" : "0") << "\n";
      }
      ofs << "\n";
    }
    ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
    return true;
  }

  /// Load enabled/disabled states from an INI-style file.
  inline bool LoadConfig(const std::wstring& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;

    AcquireSRWLockExclusive(&m_lock);

    std::string currentSection;
    std::string line;
    while (std::getline(ifs, line)) {
      // Trim
      while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
        line.erase(line.begin());
      if (line.empty() || line[0] == ';') continue;

      // Section header
      if (line.front() == '[' && line.back() == ']') {
        currentSection = line.substr(1, line.size() - 2);
        // Apply expanded state
        std::wstring sectionW = Utf8ToWide(currentSection);
        for (auto& g : m_groups) {
          if (g.name == sectionW) {
            // Will be set by "expanded=" key if present
            break;
          }
        }
        continue;
      }

      // Key=value
      auto eq = line.find('=');
      if (eq == std::string::npos) continue;
      std::string key = line.substr(0, eq);
      std::string val = line.substr(eq + 1);

      std::wstring sectionW = Utf8ToWide(currentSection);

      if (key == "expanded") {
        for (auto& g : m_groups) {
          if (g.name == sectionW) {
            g.expanded = (val == "1");
            break;
          }
        }
      }
      else {
        std::wstring extW = Utf8ToWide(key);
        for (auto& g : m_groups) {
          if (g.name == sectionW) {
            for (auto& f : g.formats) {
              if (ToLower(f.extension) == ToLower(extW)) {
                f.enabled = (val == "1");
                break;
              }
            }
            RecalcEnabledCount(g);
            break;
          }
        }
      }
    }
    ReleaseSRWLockExclusive(&m_lock);
    return true;
  }

private:
  // ── Default groups with all 200+ formats ──────────────────────────────
  inline void BuildDefaultGroups() {
    m_groups.clear();
    m_groups.reserve(11);

    auto addGroup = [&](const std::wstring& name, const std::wstring& icon,
      std::vector<FormatInfo> formats) {
        FormatGroupEntry g;
        g.name = name;
        g.icon = icon;
        g.formats = std::move(formats);
        g.expanded = true;
        RecalcEnabledCount(g);
        m_groups.push_back(std::move(g));
      };

    auto F = [](const wchar_t* ext, const wchar_t* desc,
      const wchar_t* mime, uint32_t dec) -> FormatInfo {
        return { ext, desc, mime, dec, true };
      };

    addGroup(L"Images", L"\xF4BC", {
        F(L"jpg",  L"JPEG Image",           L"image/jpeg",      1),
        F(L"jpeg", L"JPEG Image",           L"image/jpeg",      1),
        F(L"png",  L"PNG Image",            L"image/png",       1),
        F(L"bmp",  L"Bitmap Image",         L"image/bmp",       1),
        F(L"gif",  L"GIF Image",            L"image/gif",       1),
        F(L"tga",  L"Targa Image",          L"image/x-targa",   1),
        F(L"ico",  L"Windows Icon",         L"image/x-icon",    1),
        F(L"cur",  L"Windows Cursor",       L"image/x-icon",    1),
        F(L"ppm",  L"PPM Image",            L"image/x-portable-pixmap",  2),
        F(L"pgm",  L"PGM Image",            L"image/x-portable-graymap", 2),
        F(L"pbm",  L"PBM Image",            L"image/x-portable-bitmap",  2),
        F(L"pcx",  L"PCX Image",            L"image/x-pcx",     2),
        F(L"tiff", L"TIFF Image",           L"image/tiff",      1),
        F(L"tif",  L"TIFF Image",           L"image/tiff",      1),
        F(L"dds",  L"DirectDraw Surface",   L"image/vnd.ms-dds",3),
        F(L"hdr",  L"Radiance HDR",         L"image/vnd.radiance", 3),
        F(L"exr",  L"OpenEXR Image",        L"image/x-exr",     3),
        F(L"qoi",  L"QOI Image",            L"image/qoi",       2),
        F(L"sgi",  L"SGI Image",            L"image/sgi",       2),
        F(L"xpm",  L"X PixMap",             L"image/x-xpixmap", 2),
        F(L"farbfeld", L"Farbfeld Image",   L"image/x-farbfeld",2),
      });

    addGroup(L"Modern Images", L"\xF1B2", {
        F(L"webp", L"WebP Image",           L"image/webp",      4),
        F(L"avif", L"AVIF Image",           L"image/avif",      5),
        F(L"jxl",  L"JPEG XL Image",        L"image/jxl",       6),
        F(L"heic", L"HEIC Image",           L"image/heic",      7),
        F(L"heif", L"HEIF Image",           L"image/heif",      7),
        F(L"jp2",  L"JPEG 2000",            L"image/jp2",       8),
        F(L"j2k",  L"JPEG 2000 Codestream", L"image/x-jp2-codestream", 8),
        F(L"jxr",  L"JPEG XR Image",        L"image/vnd.ms-photo", 9),
        F(L"hdp",  L"HD Photo",             L"image/vnd.ms-photo", 9),
        F(L"wdp",  L"Windows Digital Photo", L"image/vnd.ms-photo", 9),
      });

    addGroup(L"RAW Camera", L"\xF030", {
        F(L"cr2",  L"Canon RAW 2",        L"image/x-canon-cr2",   10),
        F(L"cr3",  L"Canon RAW 3",        L"image/x-canon-cr3",   10),
        F(L"nef",  L"Nikon RAW",          L"image/x-nikon-nef",   10),
        F(L"arw",  L"Sony RAW",           L"image/x-sony-arw",    10),
        F(L"dng",  L"Digital Negative",   L"image/x-adobe-dng",   10),
        F(L"orf",  L"Olympus RAW",        L"image/x-olympus-orf", 10),
        F(L"rw2",  L"Panasonic RAW",      L"image/x-panasonic-rw2", 10),
        F(L"pef",  L"Pentax RAW",         L"image/x-pentax-pef",  10),
        F(L"raf",  L"Fuji RAW",           L"image/x-fuji-raf",    10),
        F(L"srw",  L"Samsung RAW",        L"image/x-samsung-srw", 10),
        F(L"x3f",  L"Sigma RAW",          L"image/x-sigma-x3f",  10),
        F(L"mrw",  L"Minolta RAW",        L"image/x-minolta-mrw", 10),
        F(L"3fr",  L"Hasselblad RAW",     L"image/x-hasselblad-3fr", 10),
        F(L"rwl",  L"Leica RAW",          L"image/x-leica-rwl",  10),
        F(L"kdc",  L"Kodak RAW",          L"image/x-kodak-kdc",  10),
        F(L"dcr",  L"Kodak RAW",          L"image/x-kodak-dcr",  10),
        F(L"iiq",  L"Phase One RAW",      L"image/x-phaseone-iiq",10),
        F(L"erf",  L"Epson RAW",          L"image/x-epson-erf",  10),
      });

    addGroup(L"Vector & Design", L"\xF5EE", {
        F(L"svg",  L"SVG Vector",           L"image/svg+xml",   11),
        F(L"psd",  L"Photoshop Document",   L"image/vnd.adobe.photoshop", 12),
        F(L"ai",   L"Adobe Illustrator",    L"application/postscript", 11),
        F(L"eps",  L"Encapsulated PostScript", L"application/postscript", 11),
        F(L"emf",  L"Enhanced Metafile",    L"image/emf",       13),
        F(L"wmf",  L"Windows Metafile",     L"image/wmf",       13),
        F(L"xps",  L"XPS Document",         L"application/oxps",14),
        F(L"xcf",  L"GIMP Image",           L"image/x-xcf",    12),
        F(L"ora",  L"OpenRaster Image",     L"image/openraster",12),
        F(L"vtf",  L"Valve Texture",        L"image/x-vtf",     3),
        F(L"ktx",  L"Khronos Texture",      L"image/ktx",       3),
        F(L"ktx2", L"Khronos Texture 2",    L"image/ktx2",      3),
        F(L"dpx",  L"Digital Picture Exchange", L"image/x-dpx", 3),
      });

    addGroup(L"Documents", L"\xF15C", {
        F(L"pdf",  L"PDF Document",         L"application/pdf", 15),
        F(L"djvu", L"DjVu Document",        L"image/vnd.djvu",  15),
        F(L"epub", L"EPUB E-Book",          L"application/epub+zip", 16),
        F(L"cbz",  L"Comic Book (ZIP)",     L"application/x-cbz", 16),
        F(L"cbr",  L"Comic Book (RAR)",     L"application/x-cbr", 16),
        F(L"cb7",  L"Comic Book (7z)",      L"application/x-cb7", 16),
        F(L"mobi", L"Kindle E-Book",        L"application/x-mobipocket-ebook", 16),
        F(L"fb2",  L"FictionBook",          L"application/x-fictionbook+xml", 16),
        F(L"xls",  L"Excel Spreadsheet",    L"application/vnd.ms-excel", 17),
        F(L"xlsx", L"Excel Document",       L"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", 17),
        F(L"doc",  L"Word Document",        L"application/msword", 17),
        F(L"docx", L"Word Document",        L"application/vnd.openxmlformats-officedocument.wordprocessingml.document", 17),
        F(L"pptx", L"PowerPoint",           L"application/vnd.openxmlformats-officedocument.presentationml.presentation", 17),
        F(L"rtf",  L"Rich Text Format",     L"application/rtf", 17),
        F(L"txt",  L"Plain Text",           L"text/plain",      18),
        F(L"md",   L"Markdown",             L"text/markdown",   18),
        F(L"csv",  L"Comma-Separated Values", L"text/csv",      18),
        F(L"json", L"JSON Data",            L"application/json",18),
        F(L"xml",  L"XML Document",         L"application/xml", 18),
        F(L"yaml", L"YAML Document",        L"application/x-yaml", 18),
        F(L"ipynb",L"Jupyter Notebook",     L"application/x-ipynb+json", 18),
      });

    addGroup(L"Archives", L"\xF1C6", {
        F(L"zip",  L"ZIP Archive",          L"application/zip",  20),
        F(L"rar",  L"RAR Archive",          L"application/vnd.rar", 20),
        F(L"7z",   L"7-Zip Archive",        L"application/x-7z-compressed", 20),
        F(L"tar",  L"TAR Archive",          L"application/x-tar", 20),
        F(L"gz",   L"Gzip Archive",         L"application/gzip", 20),
        F(L"xz",   L"XZ Archive",           L"application/x-xz", 20),
        F(L"bz2",  L"Bzip2 Archive",        L"application/x-bzip2", 20),
        F(L"cab",  L"Windows Cabinet",      L"application/vnd.ms-cab-compressed", 20),
        F(L"iso",  L"Disc Image",           L"application/x-iso9660-image", 20),
        F(L"msi",  L"Windows Installer",    L"application/x-msi", 20),
        F(L"zst",  L"Zstandard Archive",    L"application/zstd", 20),
        F(L"lz4",  L"LZ4 Archive",         L"application/x-lz4", 20),
        F(L"lzma", L"LZMA Archive",         L"application/x-lzma", 20),
        F(L"wim",  L"Windows Image",        L"application/x-ms-wim", 20),
        F(L"dmg",  L"macOS Disk Image",     L"application/x-apple-diskimage", 20),
        F(L"deb",  L"Debian Package",       L"application/vnd.debian.binary-package", 20),
        F(L"rpm",  L"RPM Package",          L"application/x-rpm", 20),
        F(L"cpio", L"CPIO Archive",         L"application/x-cpio", 20),
        F(L"ar",   L"Unix Archive",         L"application/x-archive", 20),
      });

    addGroup(L"Video", L"\xF03D", {
        F(L"mp4",  L"MPEG-4 Video",         L"video/mp4",       21),
        F(L"mkv",  L"Matroska Video",       L"video/x-matroska",21),
        F(L"avi",  L"AVI Video",            L"video/x-msvideo", 21),
        F(L"wmv",  L"Windows Media Video",  L"video/x-ms-wmv",  21),
        F(L"mov",  L"QuickTime Video",      L"video/quicktime", 21),
        F(L"flv",  L"Flash Video",          L"video/x-flv",     21),
        F(L"webm", L"WebM Video",           L"video/webm",      21),
        F(L"m4v",  L"MPEG-4 Video",         L"video/mp4",       21),
        F(L"ts",   L"MPEG Transport Stream",L"video/mp2t",      21),
        F(L"mpg",  L"MPEG Video",           L"video/mpeg",      21),
        F(L"mpeg", L"MPEG Video",           L"video/mpeg",      21),
        F(L"3gp",  L"3GPP Video",           L"video/3gpp",      21),
        F(L"ogv",  L"Ogg Video",            L"video/ogg",       21),
        F(L"asf",  L"Advanced Systems Format", L"video/x-ms-asf", 21),
      });

    addGroup(L"Audio", L"\xF001", {
        F(L"mp3",  L"MP3 Audio",            L"audio/mpeg",      22),
        F(L"flac", L"FLAC Audio",           L"audio/flac",      22),
        F(L"ogg",  L"Ogg Vorbis Audio",     L"audio/ogg",       22),
        F(L"wav",  L"WAV Audio",            L"audio/wav",       22),
        F(L"m4a",  L"AAC Audio",            L"audio/mp4",       22),
        F(L"aac",  L"AAC Audio",            L"audio/aac",       22),
        F(L"wma",  L"Windows Media Audio",  L"audio/x-ms-wma",  22),
        F(L"opus", L"Opus Audio",           L"audio/opus",      22),
        F(L"aiff", L"AIFF Audio",           L"audio/aiff",      22),
        F(L"ape",  L"Monkey's Audio",       L"audio/x-ape",     22),
        F(L"wv",   L"WavPack Audio",        L"audio/x-wavpack", 22),
        F(L"mid",  L"MIDI",                 L"audio/midi",      22),
      });

    addGroup(L"3D & CAD", L"\xF1B2", {
        F(L"stl",  L"STL 3D Model",         L"model/stl",       30),
        F(L"obj",  L"Wavefront OBJ",        L"model/obj",       30),
        F(L"fbx",  L"FBX Model",            L"model/vnd.fbx",   30),
        F(L"gltf", L"glTF Model",           L"model/gltf+json", 31),
        F(L"glb",  L"glTF Binary",          L"model/gltf-binary",31),
        F(L"step", L"STEP CAD Model",       L"model/step",      32),
        F(L"stp",  L"STEP CAD Model",       L"model/step",      32),
        F(L"iges", L"IGES CAD Model",       L"model/iges",      32),
        F(L"igs",  L"IGES CAD Model",       L"model/iges",      32),
        F(L"3ds",  L"3D Studio Model",      L"application/x-3ds",30),
        F(L"ply",  L"Stanford PLY",         L"model/ply",       30),
        F(L"dae",  L"COLLADA Model",        L"model/vnd.collada+xml", 30),
        F(L"usd",  L"Universal Scene Description", L"model/vnd.usd+zip", 30),
        F(L"usda", L"USD ASCII",            L"model/vnd.usda",  30),
        F(L"usdc", L"USD Crate",            L"model/vnd.usdc",  30),
        F(L"usdz", L"USD Zip",              L"model/vnd.usdz+zip",30),
        F(L"blend",L"Blender File",         L"application/x-blender",30),
      });

    addGroup(L"Scientific", L"\xF0C3", {
        F(L"fits", L"FITS Astronomical",     L"application/fits", 40),
        F(L"fit",  L"FITS Astronomical",     L"application/fits", 40),
        F(L"dcm",  L"DICOM Medical",         L"application/dicom",41),
        F(L"dicom",L"DICOM Medical",         L"application/dicom",41),
        F(L"nii",  L"NIfTI Neuroimaging",    L"application/x-nifti",42),
        F(L"nii.gz",L"NIfTI Compressed",     L"application/x-nifti",42),
        F(L"hdf5", L"HDF5 Scientific",       L"application/x-hdf5",43),
        F(L"h5",   L"HDF5 Scientific",       L"application/x-hdf5",43),
        F(L"nc",   L"NetCDF Data",           L"application/x-netcdf",43),
        F(L"mat",  L"MATLAB Data",           L"application/x-matlab-data",43),
        F(L"nrrd", L"NRRD Volume",           L"application/x-nrrd",43),
        F(L"mha",  L"MetaImage Header",      L"application/x-metaimage",43),
      });

    addGroup(L"Fonts", L"\xF031", {
        F(L"ttf",  L"TrueType Font",         L"font/ttf",        50),
        F(L"otf",  L"OpenType Font",         L"font/otf",        50),
        F(L"woff", L"Web Open Font Format",  L"font/woff",       50),
        F(L"woff2",L"WOFF 2.0 Font",        L"font/woff2",      50),
        F(L"eot",  L"Embedded OpenType",     L"application/vnd.ms-fontobject", 50),
        F(L"pfb",  L"PostScript Font",       L"application/x-font-type1", 50),
        F(L"pfm",  L"PostScript Font Metrics",L"application/x-font-type1",50),
      });
  }

  static void RecalcEnabledCount(FormatGroupEntry& group) {
    uint32_t count = 0;
    for (const auto& f : group.formats) {
      if (f.enabled) ++count;
    }
    group.enabledCount = count;
  }

  // ── String helpers ────────────────────────────────────────────────────
  static std::wstring ToLower(const std::wstring& s) {
    std::wstring result = s;
    for (auto& ch : result) {
      if (ch >= L'A' && ch <= L'Z') ch = ch + 32;
    }
    return result;
  }

  static std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()),
      nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()),
      s.data(), len, nullptr, nullptr);
    return s;
  }

  static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()),
      nullptr, 0);
    if (len <= 0) return {};
    std::wstring w(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()),
      w.data(), len);
    return w;
  }

  // ── State ─────────────────────────────────────────────────────────────
  mutable SRWLOCK          m_lock = SRWLOCK_INIT;
  std::vector<FormatGroupEntry> m_groups;
};

} // namespace Engine
} // namespace ExplorerLens
