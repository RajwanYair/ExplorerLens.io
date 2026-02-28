// LibraryVersionAudit.h — External Library Version Tracker and Auditor
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a compile-time registry of all external library versions.
// Provides runtime version queries and upgrade-available detection.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Library build status
enum class LibraryStatus : uint8_t {
 Built = 0, ///< Successfully built and linked
 NotBuilt = 1, ///< Source present but not compiled
 Optional = 2, ///< Optional dependency, not required
 Disabled = 3, ///< Explicitly disabled via feature flag
 Missing = 4, ///< Source not present in tree
 UpToDate = Built, ///< Alias: library is current
 UpdateAvailable = NotBuilt, ///< Alias: update pending
 COUNT = 5
};

/// A single library version entry
struct LibraryEntry {
 const char *name = nullptr;
 const char *currentVersion = nullptr;
 const char *latestKnownVersion = nullptr;
 const char *license = nullptr;
 const char *purpose = nullptr;
 const char *buildScript = nullptr;
 LibraryStatus status = LibraryStatus::Built;
 bool isUpToDate = true;
 bool hasCVE = false;
};

/// Library version audit engine
class LibraryVersionAudit {
public:
 static LibraryVersionAudit &Instance() {
 static LibraryVersionAudit instance;
 return instance;
 }

 /// Get all registered libraries
 const std::vector<LibraryEntry> &GetLibraries() const { return m_libraries; }

 /// Count libraries by status
 uint32_t CountByStatus(LibraryStatus status) const {
 uint32_t count = 0;
 for (const auto &lib : m_libraries)
 if (lib.status == status)
 ++count;
 return count;
 }

 /// Count libraries needing updates
 uint32_t CountOutdated() const {
 uint32_t count = 0;
 for (const auto &lib : m_libraries)
 if (!lib.isUpToDate && lib.status == LibraryStatus::Built)
 ++count;
 return count;
 }

 /// Get total library count
 uint32_t GetTotalCount() const {
 return static_cast<uint32_t>(m_libraries.size());
 }

 /// Check if all libraries are up to date
 bool AllUpToDate() const { return CountOutdated() == 0; }

 /// Status name lookup (wide string)
 static const wchar_t *StatusName(LibraryStatus s) {
 switch (s) {
 case LibraryStatus::Built:
 return L"Up-to-date";
 case LibraryStatus::NotBuilt:
 return L"Update Available";
 case LibraryStatus::Optional:
 return L"Optional";
 case LibraryStatus::Disabled:
 return L"Disabled";
 case LibraryStatus::Missing:
 return L"Missing";
 default:
 return L"Unknown";
 }
 }

 /// Total number of tracked libraries
 static size_t LibraryCount() { return 18; }

 /// Get number of status values
 static constexpr uint32_t GetStatusCount() { return 5; }

 /// Get audit summary
 std::string GetSummary() const {
 std::string s = "Library Audit: ";
 s += std::to_string(GetTotalCount()) + " total, ";
 s += std::to_string(CountByStatus(LibraryStatus::Built)) + " built, ";
 s += std::to_string(CountOutdated()) + " outdated";
 return s;
 }

private:
 LibraryVersionAudit() {
 m_libraries = {
 {"zlib", "1.3.1", "1.3.1", "zlib", "ZIP/deflate compression",
 "Build-Zlib.ps1", LibraryStatus::Built, true, false},
 {"lz4", "1.10.0", "1.10.0", "BSD-2", "Fast compression",
 "Build-LZ4.ps1", LibraryStatus::Built, true, false},
 {"zstd", "1.5.7", "1.5.7", "BSD-3", "Zstandard compression",
 "Build-Zstd.ps1", LibraryStatus::Built, true, false},
 {"LZMA SDK", "26.00", "26.00", "Public", "7z archives",
 "Build-LZMA-SDK-26.00.ps1", LibraryStatus::Built, true, false},
 {"minizip-ng", "4.0.10", "4.0.10", "zlib", "ZIP archive handling",
 "Build-MinizipNG.ps1", LibraryStatus::Built, true, false},
 {"UnRAR", "7.2.2", "7.2.2", "Custom", "RAR extraction", nullptr,
 LibraryStatus::Built, true, false},
 {"libwebp", "1.5.0", "1.5.0", "BSD-3", "WebP images",
 "Build-LibWebP-NMake.ps1", LibraryStatus::Built, true, false},
 {"libjxl", "0.11.1", "0.11.1", "BSD-3", "JPEG XL images", nullptr,
 LibraryStatus::Built, true, false},
 {"libavif", "1.3.0", "1.3.0", "BSD-2", "AVIF images", nullptr,
 LibraryStatus::Built, true, false},
 {"libheif", "1.19.5", "1.19.5", "LGPL-3", "HEIF/HEIC images",
 "Build-LibHEIF.ps1", LibraryStatus::Built, true, false},
 {"libde265", "1.0.15", "1.0.15", "LGPL-3", "HEVC decoding", nullptr,
 LibraryStatus::Built, true, false},
 {"LibRaw", "0.21.3", "0.21.3", "LGPL-2.1", "RAW camera formats",
 "Build-LibRaw.ps1", LibraryStatus::Built, true, false},
 {"dav1d", "1.5.1", "1.5.1", "BSD-2", "AV1 decoding", "Build-Dav1d.ps1",
 LibraryStatus::Built, true, false},
 {"brotli", "1.1.0", "1.1.0", "MIT", "Brotli compression (libjxl dep)",
 nullptr, LibraryStatus::Built, true, false},
 {"highway", "1.2.0", "1.2.0", "Apache-2", "SIMD library (libjxl dep)",
 nullptr, LibraryStatus::Built, true, false},
 {"MuPDF", "1.24.11", "1.25.4", "AGPL-3", "PDF rendering",
 "Build-MuPDF.ps1", LibraryStatus::NotBuilt, false, false},
 {"OpenJPEG", "2.5.3", "2.5.3", "BSD-2", "JPEG 2000 decoding",
 "Build-OpenJPEG.ps1", LibraryStatus::NotBuilt, true, false},
 {"FreeType", "2.13.3", "2.13.3", "FTL/GPL", "Font rendering",
 "Build-FreeType.ps1", LibraryStatus::NotBuilt, true, false},
 };
 }

 std::vector<LibraryEntry> m_libraries;
};

} // namespace Engine
} // namespace ExplorerLens
