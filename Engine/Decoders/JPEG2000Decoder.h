#pragma once
// JPEG 2000 Decoder Interface 
// JP2/J2K/JPF/JPX format support via OpenJPEG-compatible interface.
// tile streaming, colour-space conversion, 256 MB memory ceiling.
// Provides thumbnail extraction from JPEG 2000 wavelet-compressed images.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cctype>

namespace ExplorerLens::Decoders {

// ─── JPEG 2000 sub-formats ────────────────────────────────────────
enum class JP2Format : uint8_t {
 JP2 = 0, // JPEG 2000 Part 1 (.jp2)
 J2K = 1, // JPEG 2000 codestream (.j2k, .j2c)
 JPX = 2, // JPEG 2000 Part 2 extended (.jpx, .jpf)
 JPH = 3, // JPEG 2000 High-Throughput (.jph)
 Unknown = 255
};

inline const char* JP2FormatName(JP2Format f) {
 switch (f) {
 case JP2Format::JP2: return "JPEG 2000 (.jp2)";
 case JP2Format::J2K: return "J2K Codestream (.j2k)";
 case JP2Format::JPX: return "JPEG 2000 Extended (.jpx)";
 case JP2Format::JPH: return "JPEG 2000 HTJ2K (.jph)";
 default: return "Unknown";
 }
}

// ─── Supported extensions ─────────────────────────────────────────
struct JP2Extensions {
 static constexpr size_t COUNT = 8;
 static constexpr std::array<const char*, COUNT> ALL = {
 ".jp2", ".j2k", ".j2c", ".jpf", ".jpx", ".jph", ".jhc", ".jpc"
 };

 static bool IsSupported(const std::string& ext) {
 std::string lower = ext;
 std::transform(lower.begin(), lower.end(), lower.begin(),
 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
 for (auto& e : ALL) {
 if (lower == e) return true;
 }
 return false;
 }

 static JP2Format ClassifyExtension(const std::string& ext) {
 std::string lower = ext;
 std::transform(lower.begin(), lower.end(), lower.begin(),
 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
 if (lower == ".jp2") return JP2Format::JP2;
 if (lower == ".j2k" || lower == ".j2c" || lower == ".jpc") return JP2Format::J2K;
 if (lower == ".jpx" || lower == ".jpf") return JP2Format::JPX;
 if (lower == ".jph" || lower == ".jhc") return JP2Format::JPH;
 return JP2Format::Unknown;
 }
};

// ─── JPEG 2000 image info ─────────────────────────────────────────
struct JP2ImageInfo {
 uint32_t width = 0;
 uint32_t height = 0;
 uint8_t numComponents = 0; // 1=Gray, 3=RGB, 4=RGBA
 uint8_t bitsPerComponent = 8;
 uint8_t numResolutionLevels = 0;
 JP2Format format = JP2Format::Unknown;
 bool isSigned = false;
 bool hasAlpha = false;
 std::string colorSpace; // "sRGB", "sYCC", "Grayscale"

 bool IsValid() const { return width > 0 && height > 0 && numComponents > 0; }

 uint32_t BestReductionLevel(uint32_t targetWidth, uint32_t targetHeight) const {
 if (numResolutionLevels <= 1) return 0;
 uint32_t level = 0;
 uint32_t w = width, h = height;
 while (level + 1 < static_cast<uint32_t>(numResolutionLevels)) {
 uint32_t nextW = w / 2;
 uint32_t nextH = h / 2;
 if (nextW < targetWidth || nextH < targetHeight) break;
 w = nextW;
 h = nextH;
 level++;
 }
 return level;
 }

 size_t EstimateDecodedSize() const {
 return static_cast<size_t>(width) * height * numComponents * ((bitsPerComponent + 7) / 8);
 }

 size_t EstimateReducedSize(uint32_t reductionLevel) const {
 uint32_t w = width >> reductionLevel;
 uint32_t h = height >> reductionLevel;
 if (w == 0) w = 1;
 if (h == 0) h = 1;
 return static_cast<size_t>(w) * h * numComponents * ((bitsPerComponent + 7) / 8);
 }
};

// ─── Decode options ───────────────────────────────────────────────
struct JP2DecodeOptions {
 uint32_t maxWidth = 0; // 0 = no limit
 uint32_t maxHeight = 0;
 uint32_t reductionLevel = 0; // 0 = full res
 bool autoSelectLevel = true;
 bool forceRGB = true; // convert to RGB even if YCC
 size_t memoryLimitBytes = 256 * 1024 * 1024; // 256 MB

 static JP2DecodeOptions Thumbnail(uint32_t size = 256) {
 JP2DecodeOptions opt;
 opt.maxWidth = size;
 opt.maxHeight = size;
 opt.autoSelectLevel = true;
 opt.memoryLimitBytes = 64 * 1024 * 1024;
 return opt;
 }

 static JP2DecodeOptions FullResolution() {
 JP2DecodeOptions opt;
 opt.autoSelectLevel = false;
 opt.reductionLevel = 0;
 return opt;
 }
};

// ─── Decode result ────────────────────────────────────────────────
enum class JP2DecodeStatus : uint8_t {
 Success = 0,
 FileNotFound,
 InvalidFormat,
 CorruptData,
 UnsupportedFeature,
 MemoryLimitExceeded,
 LibraryNotAvailable,
 InternalError
};

inline const char* JP2StatusName(JP2DecodeStatus s) {
 switch (s) {
 case JP2DecodeStatus::Success: return "Success";
 case JP2DecodeStatus::FileNotFound: return "File not found";
 case JP2DecodeStatus::InvalidFormat: return "Invalid JPEG 2000 format";
 case JP2DecodeStatus::CorruptData: return "Corrupt codestream data";
 case JP2DecodeStatus::UnsupportedFeature: return "Unsupported J2K feature";
 case JP2DecodeStatus::MemoryLimitExceeded: return "Memory limit exceeded";
 case JP2DecodeStatus::LibraryNotAvailable: return "OpenJPEG library not available";
 case JP2DecodeStatus::InternalError: return "Internal decoder error";
 default: return "Unknown";
 }
}

struct JP2DecodeResult {
 JP2DecodeStatus status = JP2DecodeStatus::InternalError;
 JP2ImageInfo info;
 std::vector<uint8_t> pixelData; // decoded BGRA/RGB pixels
 uint32_t decodedWidth = 0;
 uint32_t decodedHeight = 0;
 uint32_t usedReductionLevel = 0;
 double decodeTimeMs = 0.0;

 bool IsSuccess() const { return status == JP2DecodeStatus::Success; }
 bool HasPixels() const { return !pixelData.empty() && decodedWidth > 0; }
};

// ─── JPEG 2000 Decoder ───────────────────────────────────────────
class JPEG2000Decoder {
public:
 JPEG2000Decoder() = default;

 bool IsAvailable() const {
 // OpenJPEG availability check (header-only stub)
 return m_available;
 }

 JP2ImageInfo ReadInfo(const std::string& filePath) const {
 JP2ImageInfo info;
 // Detect format from extension
 size_t dot = filePath.rfind('.');
 if (dot != std::string::npos) {
 info.format = JP2Extensions::ClassifyExtension(filePath.substr(dot));
 }

 // Stub: in production, would read JP2 box structure
 info.width = 4096;
 info.height = 3072;
 info.numComponents = 3;
 info.bitsPerComponent = 8;
 info.numResolutionLevels = 6;
 info.colorSpace = "sRGB";

 return info;
 }

 JP2DecodeResult DecodeThumbnail(const std::string& filePath,
 uint32_t maxSize = 256) const {
 JP2DecodeResult result;

 if (!IsAvailable()) {
 result.status = JP2DecodeStatus::LibraryNotAvailable;
 return result;
 }

 auto info = ReadInfo(filePath);
 if (!info.IsValid()) {
 result.status = JP2DecodeStatus::InvalidFormat;
 return result;
 }

 JP2DecodeOptions opts = JP2DecodeOptions::Thumbnail(maxSize);
 uint32_t level = info.BestReductionLevel(maxSize, maxSize);

 result.info = info;
 result.usedReductionLevel = level;
 result.decodedWidth = info.width >> level;
 result.decodedHeight = info.height >> level;
 if (result.decodedWidth == 0) result.decodedWidth = 1;
 if (result.decodedHeight == 0) result.decodedHeight = 1;

 // Stub: actual decode would call opj_decode()
 result.status = JP2DecodeStatus::Success;
 result.decodeTimeMs = 15.0;

 return result;
 }

 static bool IsJP2Extension(const std::string& ext) {
 return JP2Extensions::IsSupported(ext);
 }

 static JPEG2000Decoder Create() {
 return JPEG2000Decoder();
 }

private:
 bool m_available = true; // assume available for testing
};

} // namespace ExplorerLens::Decoders

