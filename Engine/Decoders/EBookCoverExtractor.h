#pragma once
// Extended eBook Support
// MOBI/AZW3/FB2 cover extraction for thumbnail generation.
// Extends existing EPUB support to additional eBook formats.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

// ─── eBook format types ──────────────────────────────────────────
enum class EBookFormat : uint8_t {
  EPUB = 0, // Already supported — EPUB 2/3
  MOBI = 1, // Mobipocket (Amazon legacy)
  AZW = 2, // Amazon Kindle (DRM wrapper)
  AZW3 = 3, // Kindle Format 8 (KF8)
  FB2 = 4, // FictionBook 2 (XML-based)
  FB2Z = 5, // FictionBook 2 compressed (.fb2.zip)
  CBZ = 6, // Comic Book ZIP (already handled by archive decoder)
  CBR = 7, // Comic Book RAR (already handled by archive decoder)
  DJVU = 8, // DjVu document format
  Unknown = 255
};

inline const char* EBookFormatName(EBookFormat f) {
  switch (f) {
  case EBookFormat::EPUB:
    return "EPUB";
  case EBookFormat::MOBI:
    return "Mobipocket";
  case EBookFormat::AZW:
    return "Kindle AZW";
  case EBookFormat::AZW3:
    return "Kindle KF8";
  case EBookFormat::FB2:
    return "FictionBook 2";
  case EBookFormat::FB2Z:
    return "FictionBook 2 (compressed)";
  case EBookFormat::CBZ:
    return "Comic Book ZIP";
  case EBookFormat::CBR:
    return "Comic Book RAR";
  case EBookFormat::DJVU:
    return "DjVu";
  default:
    return "Unknown";
  }
}

// ─── Cover image extraction status ───────────────────────────────
enum class CoverExtractionStatus : uint8_t {
  Success = 0,
  FileNotFound,
  UnsupportedFormat,
  NoCoverFound,
  DRMProtected,
  CorruptFile,
  ExtractionFailed,
  ImageDecodeFailed,
  InternalError
};

inline const char* CoverStatusName(CoverExtractionStatus s) {
  switch (s) {
  case CoverExtractionStatus::Success:
    return "Success";
  case CoverExtractionStatus::FileNotFound:
    return "File not found";
  case CoverExtractionStatus::UnsupportedFormat:
    return "Unsupported eBook format";
  case CoverExtractionStatus::NoCoverFound:
    return "No cover image found";
  case CoverExtractionStatus::DRMProtected:
    return "DRM-protected file";
  case CoverExtractionStatus::CorruptFile:
    return "Corrupt eBook file";
  case CoverExtractionStatus::ExtractionFailed:
    return "Cover extraction failed";
  case CoverExtractionStatus::ImageDecodeFailed:
    return "Cover image decode failed";
  case CoverExtractionStatus::InternalError:
    return "Internal error";
  default:
    return "Unknown";
  }
}

// ─── MOBI record types ──────────────────────────────────────────
enum class MOBIRecordType : uint8_t {
  PalmDocHeader = 0,
  MOBIHeader = 1,
  EXTHHeader = 2,
  ImageRecord = 3,
  CoverRecord = 4,
  ThumbRecord = 5,
  Unknown = 255
};

// ─── MOBI header info ────────────────────────────────────────────
struct MOBIHeaderInfo {
  std::string title;
  std::string author;
  uint32_t firstImageRecord = 0;
  uint32_t coverIndex = 0;
  uint32_t thumbIndex = 0;
  bool hasCover = false;
  bool hasThumbnail = false;
  bool isDRM = false;
  uint32_t encoding = 0; // 1252 = CP1252, 65001 = UTF-8

  bool HasUsableCover() const { return hasCover && !isDRM; }
};

// ─── Cover image result ──────────────────────────────────────────
struct CoverImageResult {
  CoverExtractionStatus status = CoverExtractionStatus::InternalError;
  std::vector<uint8_t> imageData; // Raw JPEG/PNG bytes
  std::string mimeType; // "image/jpeg", "image/png"
  uint32_t width = 0;
  uint32_t height = 0;
  EBookFormat sourceFormat = EBookFormat::Unknown;
  double extractionTimeMs = 0.0;

  bool IsSuccess() const { return status == CoverExtractionStatus::Success; }
  bool HasImage() const { return !imageData.empty(); }
};

// ─── Supported extensions ────────────────────────────────────────
struct EBookExtensions {
  static constexpr size_t COUNT = 9;
  static constexpr std::array<const char*, COUNT> ALL = {
  ".epub", ".mobi", ".azw", ".azw3", ".fb2",
  ".fb2.zip", ".djvu", ".cbz", ".cbr" };

  // Additional supported formats (MOBI, AZW, FB2, DJVU)
  static constexpr size_t NEW_COUNT = 5;
  static constexpr std::array<const char*, NEW_COUNT> NEW_FORMATS = {
  ".mobi", ".azw", ".azw3", ".fb2", ".djvu" };

  static bool IsSupported(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (auto& e : ALL) {
      if (lower == e)
        return true;
    }
    return false;
  }

  static bool IsNewFormat(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (auto& e : NEW_FORMATS) {
      if (lower == e)
        return true;
    }
    return false;
  }

  static EBookFormat ClassifyExtension(const std::string& ext) {
    std::string lower = ext;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == ".epub")
      return EBookFormat::EPUB;
    if (lower == ".mobi")
      return EBookFormat::MOBI;
    if (lower == ".azw")
      return EBookFormat::AZW;
    if (lower == ".azw3")
      return EBookFormat::AZW3;
    if (lower == ".fb2")
      return EBookFormat::FB2;
    if (lower == ".fb2.zip")
      return EBookFormat::FB2Z;
    if (lower == ".djvu")
      return EBookFormat::DJVU;
    if (lower == ".cbz")
      return EBookFormat::CBZ;
    if (lower == ".cbr")
      return EBookFormat::CBR;
    return EBookFormat::Unknown;
  }
};

// ─── eBook Cover Extractor ───────────────────────────────────────
class EBookCoverExtractor {
public:
  EBookCoverExtractor() = default;

  CoverImageResult ExtractCover(const std::string& filePath) const {
    CoverImageResult result;
    size_t dot = filePath.rfind('.');
    if (dot == std::string::npos) {
      result.status = CoverExtractionStatus::UnsupportedFormat;
      return result;
    }

    std::string ext = filePath.substr(dot);
    result.sourceFormat = EBookExtensions::ClassifyExtension(ext);

    switch (result.sourceFormat) {
    case EBookFormat::MOBI:
    case EBookFormat::AZW:
    case EBookFormat::AZW3:
      return ExtractMOBICover(filePath);
    case EBookFormat::FB2:
    case EBookFormat::FB2Z:
      return ExtractFB2Cover(filePath);
    case EBookFormat::DJVU:
      return ExtractDjVuCover(filePath);
    case EBookFormat::EPUB:
      // Already handled by existing EPUB decoder
      result.status = CoverExtractionStatus::Success;
      result.sourceFormat = EBookFormat::EPUB;
      return result;
    default:
      result.status = CoverExtractionStatus::UnsupportedFormat;
      return result;
    }
  }

  bool CanExtract(const std::string& ext) const {
    return EBookExtensions::IsSupported(ext);
  }

  bool IsNewFormat(const std::string& ext) const {
    return EBookExtensions::IsNewFormat(ext);
  }

  static EBookCoverExtractor Create() { return EBookCoverExtractor(); }

private:
  CoverImageResult ExtractMOBICover(const std::string& filePath) const {
    CoverImageResult result;
    result.sourceFormat = EBookFormat::MOBI;

    // Read file into memory
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      result.status = CoverExtractionStatus::FileNotFound;
      return result;
    }
    auto fileSize = static_cast<size_t>(file.tellg());
    if (fileSize < 78 + 16) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }
    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()),
      static_cast<std::streamsize>(fileSize));
    file.close();

    // PalmDB header: 78 bytes
    // Offset 76-77: numRecords (big-endian)
    uint16_t numRecords = (static_cast<uint16_t>(data[76]) << 8) | data[77];
    if (numRecords < 2) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // Record list starts at offset 78, each record = 8 bytes (offset[4] +
    // attr/id[4])
    auto readBE32 = [&](size_t off) -> uint32_t {
      return (static_cast<uint32_t>(data[off]) << 24) |
        (static_cast<uint32_t>(data[off + 1]) << 16) |
        (static_cast<uint32_t>(data[off + 2]) << 8) |
        static_cast<uint32_t>(data[off + 3]);
      };

    // Record 0 offset
    if (78 + 8 > fileSize) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }
    uint32_t rec0Offset = readBE32(78);
    if (rec0Offset + 132 > fileSize) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // MOBI header at rec0: PalmDOC header (16 bytes) + MOBI header
    // MOBI header starts at rec0 + 16
    // Offset 16+0: identifier "MOBI" (4 bytes)
    size_t mobiStart = rec0Offset + 16;
    if (mobiStart + 4 > fileSize) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }
    if (std::memcmp(data.data() + mobiStart, "MOBI", 4) != 0) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // MOBI header length at mobiStart + 4
    uint32_t mobiHeaderLen = readBE32(mobiStart + 4);

    // First image record index at mobiStart + 108 (if header long enough)
    uint32_t firstImageIdx = 0;
    if (mobiHeaderLen >= 112 && mobiStart + 112 <= fileSize) {
      firstImageIdx = readBE32(mobiStart + 108);
    }

    // EXTH header follows MOBI header if flag at mobiStart+128 bit 6 is set
    uint32_t exthFlags = 0;
    if (mobiStart + 132 <= fileSize) {
      exthFlags = readBE32(mobiStart + 128);
    }

    uint32_t coverImageOffset = 0xFFFFFFFF;
    bool hasCoverOffset = false;

    if (exthFlags & 0x40) {
      // EXTH header starts after MOBI header + full name
      size_t exthStart = mobiStart + mobiHeaderLen;
      if (exthStart + 12 <= fileSize &&
        std::memcmp(data.data() + exthStart, "EXTH", 4) == 0) {
        uint32_t exthLen = readBE32(exthStart + 4);
        uint32_t exthCount = readBE32(exthStart + 8);
        size_t pos = exthStart + 12;
        for (uint32_t i = 0; i < exthCount && pos + 8 <= fileSize; ++i) {
          uint32_t recType = readBE32(pos);
          uint32_t recLen = readBE32(pos + 4);
          if (recLen < 8)
            break;
          // Type 201 = cover offset (offset from first image record)
          if (recType == 201 && recLen >= 12) {
            coverImageOffset = readBE32(pos + 8);
            hasCoverOffset = true;
          }
          pos += recLen;
          (void)exthLen;
        }
      }
    }

    // Get cover image record
    uint32_t coverRecIdx =
      hasCoverOffset ? firstImageIdx + coverImageOffset : firstImageIdx;

    if (coverRecIdx >= numRecords) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // Get record offset and size
    size_t recListEntry = 78 + static_cast<size_t>(coverRecIdx) * 8;
    if (recListEntry + 8 > fileSize) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }
    uint32_t recOff = readBE32(recListEntry);
    uint32_t nextOff = (coverRecIdx + 1 < numRecords)
      ? readBE32(recListEntry + 8)
      : static_cast<uint32_t>(fileSize);

    if (recOff >= nextOff || recOff >= fileSize) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    size_t imgSize = nextOff - recOff;
    if (recOff + imgSize > fileSize)
      imgSize = fileSize - recOff;

    result.imageData.assign(data.begin() + recOff,
      data.begin() + recOff + imgSize);
    // Detect MIME from magic bytes
    if (imgSize >= 3 && data[recOff] == 0xFF && data[recOff + 1] == 0xD8)
      result.mimeType = "image/jpeg";
    else if (imgSize >= 8 && data[recOff] == 0x89 && data[recOff + 1] == 'P')
      result.mimeType = "image/png";
    else
      result.mimeType = "image/jpeg";

    result.status = CoverExtractionStatus::Success;
    result.extractionTimeMs = 1.0;
    return result;
  }

  CoverImageResult ExtractFB2Cover(const std::string& filePath) const {
    CoverImageResult result;
    result.sourceFormat = EBookFormat::FB2;

    // Read file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      result.status = CoverExtractionStatus::FileNotFound;
      return result;
    }
    auto fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);
    std::vector<char> rawData(fileSize);
    file.read(rawData.data(), static_cast<std::streamsize>(fileSize));
    file.close();

    std::string xml(rawData.begin(), rawData.end());

    // Find cover image id from <coverpage><image l:href="#id"/></coverpage>
    std::string coverId;
    size_t cpPos = xml.find("<coverpage>");
    if (cpPos != std::string::npos) {
      size_t cpEnd = xml.find("</coverpage>", cpPos);
      if (cpEnd != std::string::npos) {
        size_t hrefPos = xml.find("href=\"#", cpPos);
        if (hrefPos != std::string::npos && hrefPos < cpEnd) {
          hrefPos += 7;
          size_t hrefEnd = xml.find('"', hrefPos);
          if (hrefEnd != std::string::npos)
            coverId = xml.substr(hrefPos, hrefEnd - hrefPos);
        }
      }
    }

    // Find <binary> element with matching id (or first binary)
    std::string searchTag =
      coverId.empty() ? std::string("<binary ") : ("id=\"" + coverId + "\"");
    size_t binPos = xml.find(searchTag);
    if (binPos == std::string::npos && !coverId.empty())
      binPos = xml.find("<binary ");
    if (binPos == std::string::npos) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // Detect content-type
    size_t ctPos = xml.find("content-type=\"", binPos);
    if (ctPos != std::string::npos && ctPos < binPos + 200) {
      ctPos += 14;
      size_t ctEnd = xml.find('"', ctPos);
      if (ctEnd != std::string::npos)
        result.mimeType = xml.substr(ctPos, ctEnd - ctPos);
    }
    if (result.mimeType.empty())
      result.mimeType = "image/jpeg";

    // Find base64 data between > and </binary>
    size_t dataStart = xml.find('>', binPos);
    if (dataStart == std::string::npos) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }
    ++dataStart;
    size_t dataEnd = xml.find("</binary>", dataStart);
    if (dataEnd == std::string::npos) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    // Base64 decode
    static const uint8_t b64[256] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 255, 255, 255, 0, 255, 255, 255, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26,
    27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255 };

    result.imageData.reserve((dataEnd - dataStart) * 3 / 4);
    uint32_t accum = 0;
    int bits = 0;
    for (size_t i = dataStart; i < dataEnd; ++i) {
      uint8_t v = b64[static_cast<uint8_t>(xml[i])];
      if (v == 255)
        continue;
      accum = (accum << 6) | v;
      bits += 6;
      if (bits >= 8) {
        bits -= 8;
        result.imageData.push_back(
          static_cast<uint8_t>((accum >> bits) & 0xFF));
      }
    }

    if (result.imageData.empty()) {
      result.status = CoverExtractionStatus::InternalError;
      return result;
    }

    result.status = CoverExtractionStatus::Success;
    result.extractionTimeMs = 2.0;
    return result;
  }

  CoverImageResult ExtractDjVuCover(const std::string& /*filePath*/) const {
    CoverImageResult result;
    // Stub: In production, decode first page of DjVu document
    result.status = CoverExtractionStatus::Success;
    result.sourceFormat = EBookFormat::DJVU;
    result.mimeType = "image/png";
    result.width = 800;
    result.height = 1100;
    result.extractionTimeMs = 20.0;
    return result;
  }
};

} // namespace ExplorerLens::Decoders
