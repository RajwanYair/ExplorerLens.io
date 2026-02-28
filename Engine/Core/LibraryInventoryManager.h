//==============================================================================
// LibraryInventoryManager.h — Library Inventory Manager
// Tracks all external library versions, build status, and dependencies.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Programmatic library inventory with build status tracking.
class LibraryInventoryManager {
public:
 enum class BuildStatus { Built, NotBuilt, Optional, Deprecated, COUNT };

 enum class LibCategory {
 Compression,
 Image,
 Camera,
 PDF,
 Video,
 Font,
 Archive,
 COUNT
 };

 struct LibraryEntry {
 std::wstring name;
 std::wstring version;
 LibCategory category;
 BuildStatus status;
 std::wstring license;
 std::wstring purpose;
 };

 static const wchar_t *StatusName(BuildStatus s) {
 switch (s) {
 case BuildStatus::Built:
 return L"Built";
 case BuildStatus::NotBuilt:
 return L"NotBuilt";
 case BuildStatus::Optional:
 return L"Optional";
 case BuildStatus::Deprecated:
 return L"Deprecated";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *CategoryName(LibCategory c) {
 switch (c) {
 case LibCategory::Compression:
 return L"Compression";
 case LibCategory::Image:
 return L"Image";
 case LibCategory::Camera:
 return L"Camera";
 case LibCategory::PDF:
 return L"PDF";
 case LibCategory::Video:
 return L"Video";
 case LibCategory::Font:
 return L"Font";
 case LibCategory::Archive:
 return L"Archive";
 default:
 return L"Unknown";
 }
 }

 static size_t StatusCount() {
 return static_cast<size_t>(BuildStatus::COUNT);
 }
 static size_t CategoryCount() {
 return static_cast<size_t>(LibCategory::COUNT);
 }

 static std::vector<LibraryEntry> GetInventory() {
 return {
 {L"zlib", L"1.3.1", LibCategory::Compression, BuildStatus::Built,
 L"zlib", L"ZIP/deflate compression"},
 {L"lz4", L"1.10.0", LibCategory::Compression, BuildStatus::Built,
 L"BSD-2", L"Fast compression"},
 {L"zstd", L"1.5.7", LibCategory::Compression, BuildStatus::Built,
 L"BSD-3", L"Zstandard compression"},
 {L"minizip-ng", L"4.0.10", LibCategory::Compression, BuildStatus::Built,
 L"zlib", L"ZIP archive handling"},
 {L"LZMA SDK", L"26.00", LibCategory::Compression, BuildStatus::Built,
 L"Public", L"7z archives"},
 {L"UnRAR", L"7.2.2", LibCategory::Archive, BuildStatus::Built, L"Unrar",
 L"RAR extraction"},
 {L"bzip2", L"1.0.8", LibCategory::Compression, BuildStatus::Built,
 L"BSD", L"BZ2 compression"},
 {L"xz", L"5.6.3", LibCategory::Compression, BuildStatus::Built,
 L"Public", L"XZ/LZMA compression"},
 {L"libarchive", L"3.7.6", LibCategory::Archive, BuildStatus::Built,
 L"BSD-2", L"Universal archive handling"},
 {L"libwebp", L"1.5.0", LibCategory::Image, BuildStatus::Built, L"BSD-3",
 L"WebP images"},
 {L"libjxl", L"0.11.1", LibCategory::Image, BuildStatus::Built, L"BSD-3",
 L"JPEG XL images"},
 {L"libavif", L"1.3.0", LibCategory::Image, BuildStatus::Built, L"BSD-2",
 L"AVIF images"},
 {L"libheif", L"1.19.5", LibCategory::Image, BuildStatus::Built,
 L"LGPL-3", L"HEIF/HEIC images"},
 {L"libde265", L"1.0.15", LibCategory::Image, BuildStatus::Built,
 L"LGPL-3", L"HEVC decoding"},
 {L"dav1d", L"1.5.1", LibCategory::Image, BuildStatus::Built, L"BSD-2",
 L"AV1 decoding"},
 {L"brotli", L"1.1.0", LibCategory::Compression, BuildStatus::Built,
 L"MIT", L"JXL dependency"},
 {L"highway", L"1.2.0", LibCategory::Image, BuildStatus::Built,
 L"Apache-2", L"SIMD for JXL"},
 {L"LibRaw", L"0.21.3", LibCategory::Camera, BuildStatus::Built,
 L"LGPL-2.1", L"RAW camera formats"},
 {L"MuPDF", L"1.24.11", LibCategory::PDF, BuildStatus::Optional,
 L"AGPL-3", L"PDF rendering"},
 {L"OpenJPEG", L"2.5.3", LibCategory::Image, BuildStatus::Optional,
 L"BSD-2", L"JPEG 2000"},
 {L"FreeType", L"2.13.3", LibCategory::Font, BuildStatus::Optional,
 L"FTL/GPL", L"Font rendering"},
 };
 }

 static size_t TotalLibraries() { return GetInventory().size(); }

 static size_t BuiltCount() {
 size_t count = 0;
 for (const auto &e : GetInventory())
 if (e.status == BuildStatus::Built)
 ++count;
 return count;
 }

 static bool Validate() { return TotalLibraries() >= 18; }
};

} // namespace Engine
} // namespace ExplorerLens
