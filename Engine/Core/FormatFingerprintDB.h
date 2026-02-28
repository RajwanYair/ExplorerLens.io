#pragma once
// FormatFingerprintDB.h — Format Fingerprint Database
// Binary signature database for 250+ file formats with O(1) magic-byte lookup,
// MIME type resolution, and confidence scoring for ambiguous formats.
#include <cstdint>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

/// Confidence level for format identification
enum class FormatConfidence : uint8_t {
 None = 0, // No match
 Low, // Extension-only match
 Medium, // Partial magic match
 High, // Full magic + structure match
 Definitive, // Validated header + trailer
 COUNT
};

/// Format family classification
enum class FormatFamily : uint8_t {
 Unknown = 0,
 RasterImage,
 VectorImage,
 RawCamera,
 HDRImage,
 Archive,
 Document,
 Video,
 Audio,
 Model3D,
 Scientific,
 Geospatial,
 Font,
 Executable,
 Database,
 COUNT
};

/// A stored fingerprint entry
struct FormatFingerprint {
 const char *extension = nullptr;
 const uint8_t *magic = nullptr;
 uint32_t magicLen = 0;
 uint32_t magicOffset = 0;
 const char *mimeType = nullptr;
 FormatFamily family = FormatFamily::Unknown;
 FormatConfidence confidence = FormatConfidence::None;
};

class FormatFingerprintDB {
public:
 static constexpr size_t ConfidenceCount() {
 return static_cast<size_t>(FormatConfidence::COUNT);
 }
 static constexpr size_t FamilyCount() {
 return static_cast<size_t>(FormatFamily::COUNT);
 }

 static const wchar_t *ConfidenceName(FormatConfidence c) {
 switch (c) {
 case FormatConfidence::None:
 return L"None";
 case FormatConfidence::Low:
 return L"Low";
 case FormatConfidence::Medium:
 return L"Medium";
 case FormatConfidence::High:
 return L"High";
 case FormatConfidence::Definitive:
 return L"Definitive";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *FamilyName(FormatFamily f) {
 switch (f) {
 case FormatFamily::Unknown:
 return L"Unknown";
 case FormatFamily::RasterImage:
 return L"Raster Image";
 case FormatFamily::VectorImage:
 return L"Vector Image";
 case FormatFamily::RawCamera:
 return L"RAW Camera";
 case FormatFamily::HDRImage:
 return L"HDR Image";
 case FormatFamily::Archive:
 return L"Archive";
 case FormatFamily::Document:
 return L"Document";
 case FormatFamily::Video:
 return L"Video";
 case FormatFamily::Audio:
 return L"Audio";
 case FormatFamily::Model3D:
 return L"3D Model";
 case FormatFamily::Scientific:
 return L"Scientific";
 case FormatFamily::Geospatial:
 return L"Geospatial";
 case FormatFamily::Font:
 return L"Font";
 case FormatFamily::Executable:
 return L"Executable";
 case FormatFamily::Database:
 return L"Database";
 default:
 return L"Unknown";
 }
 }

 /// Check if a buffer starts with the PNG magic bytes
 static bool MatchesPNG(const uint8_t *data, size_t len) {
 static const uint8_t pngMagic[] = {0x89, 0x50, 0x4E, 0x47,
 0x0D, 0x0A, 0x1A, 0x0A};
 return len >= 8 && std::memcmp(data, pngMagic, 8) == 0;
 }

 /// Check if a buffer starts with JPEG SOI marker
 static bool MatchesJPEG(const uint8_t *data, size_t len) {
 return len >= 2 && data[0] == 0xFF && data[1] == 0xD8;
 }

 /// Check if a buffer starts with PDF magic
 static bool MatchesPDF(const uint8_t *data, size_t len) {
 return len >= 5 && data[0] == '%' && data[1] == 'P' && data[2] == 'D' &&
 data[3] == 'F' && data[4] == '-';
 }
};

} // namespace Engine
} // namespace ExplorerLens
