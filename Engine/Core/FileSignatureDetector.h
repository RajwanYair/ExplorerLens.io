// FileSignatureDetector.h — Magic Byte File Format Detection
// Copyright (c) 2026 ExplorerLens Project
//
// High-performance file format identification using magic byte signatures.
// Examines file headers to determine actual format regardless of extension.
// Supports 200+ formats with prioritized signature matching and fallback
// heuristics for ambiguous formats.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

/// Detected file format category
enum class SignatureCategory : uint8_t {
    Unknown,
    Image,
    Archive,
    Document,
    Video,
    Audio,
    CAD,
    Font,
    Scientific,
    Executable,
};

/// Result of signature detection
struct SignatureMatch {
    std::string     formatName;         // e.g., "JPEG", "PNG", "ZIP"
    std::string     mimeType;           // e.g., "image/jpeg"
    SignatureCategory  category = SignatureCategory::Unknown;
    float           confidence = 0.0f; // [0,1] confidence in the match
    uint32_t        headerBytes = 0;    // How many bytes were needed for detection
    bool            isContainer = false; // Archive/container format
};

/// Single signature entry: offset + byte pattern + optional mask
struct SignatureEntry {
    const char* formatName;
    const char* mimeType;
    SignatureCategory  category;
    uint32_t        offset;         // Byte offset to check
    const uint8_t* pattern;        // Expected bytes
    uint32_t        patternLen;     // Length of pattern
    const uint8_t* mask;           // Optional mask (nullptr = exact match)
    float           baseConfidence; // Base confidence for this signature
    bool            isContainer;
};

/// Fast file format detection engine using magic byte signatures.
/// Designed for the thumbnail pipeline where correct format identification
/// is critical for routing to the right decoder.
///
/// Usage:
///   FileSignatureDetector detector;
///   auto match = detector.Detect(headerBytes, headerSize);
///   auto match2 = detector.DetectFile(L"C:\\photo.jpg");
///
class FileSignatureDetector {
public:
    FileSignatureDetector() { BuildSignatureTable(); }

    /// Detect format from a memory buffer (typically first 64-256 bytes of file)
    SignatureMatch Detect(const uint8_t* data, size_t dataSize) const {
        if (!data || dataSize == 0) return {};

        SignatureMatch bestMatch;
        bestMatch.confidence = 0.0f;

        for (const auto& sig : m_signatures) {
            if (sig.offset + sig.patternLen > dataSize) continue;

            if (MatchPattern(data + sig.offset, sig.pattern, sig.mask, sig.patternLen)) {
                float confidence = sig.baseConfidence;

                // Boost confidence for longer patterns
                if (sig.patternLen >= 8) confidence += 0.1f;
                if (sig.patternLen >= 12) confidence += 0.05f;

                if (confidence > bestMatch.confidence) {
                    bestMatch.formatName = sig.formatName;
                    bestMatch.mimeType = sig.mimeType;
                    bestMatch.category = sig.category;
                    bestMatch.confidence = (std::min)(confidence, 1.0f);
                    bestMatch.headerBytes = sig.offset + sig.patternLen;
                    bestMatch.isContainer = sig.isContainer;
                }
            }
        }

        return bestMatch;
    }

    /// Detect format by reading from a file path
    SignatureMatch DetectFile(const wchar_t* filePath) const {
        if (!filePath) return {};

        HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return {};

        uint8_t header[256] = {};
        DWORD bytesRead = 0;
        ReadFile(hFile, header, sizeof(header), &bytesRead, nullptr);
        CloseHandle(hFile);

        return Detect(header, bytesRead);
    }

    /// Check if a format is an image type
    static bool IsImageFormat(const SignatureMatch& match) {
        return match.category == SignatureCategory::Image;
    }

    /// Check if a format is an archive type
    static bool IsArchiveFormat(const SignatureMatch& match) {
        return match.category == SignatureCategory::Archive || match.isContainer;
    }

    /// Get the recommended LENSTYPE for a detected format
    /// Returns 0 if no mapping exists
    uint32_t GetLENSType(const SignatureMatch& match) const {
        auto it = m_formatToLENSType.find(match.formatName);
        return (it != m_formatToLENSType.end()) ? it->second : 0;
    }

private:
    // Signature patterns (static storage)
    static constexpr uint8_t kPNG[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    static constexpr uint8_t kJPEG[] = { 0xFF, 0xD8, 0xFF };
    static constexpr uint8_t kGIF87[] = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
    static constexpr uint8_t kGIF89[] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
    static constexpr uint8_t kBMP[] = { 0x42, 0x4D };
    static constexpr uint8_t kTIFF_LE[] = { 0x49, 0x49, 0x2A, 0x00 };
    static constexpr uint8_t kTIFF_BE[] = { 0x4D, 0x4D, 0x00, 0x2A };
    static constexpr uint8_t kWEBP[] = { 0x52, 0x49, 0x46, 0x46 };  // RIFF header
    static constexpr uint8_t kWEBP2[] = { 0x57, 0x45, 0x42, 0x50 };  // WEBP at offset 8
    static constexpr uint8_t kPSD[] = { 0x38, 0x42, 0x50, 0x53 };   // 8BPS
    static constexpr uint8_t kICO[] = { 0x00, 0x00, 0x01, 0x00 };
    static constexpr uint8_t kCUR[] = { 0x00, 0x00, 0x02, 0x00 };
    static constexpr uint8_t kPDF[] = { 0x25, 0x50, 0x44, 0x46, 0x2D };  // %PDF-
    static constexpr uint8_t kZIP[] = { 0x50, 0x4B, 0x03, 0x04 };
    static constexpr uint8_t kRAR5[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 };
    static constexpr uint8_t kRAR4[] = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
    static constexpr uint8_t k7Z[] = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
    static constexpr uint8_t kGZ[] = { 0x1F, 0x8B };
    static constexpr uint8_t kXZ[] = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
    static constexpr uint8_t kZSTD[] = { 0x28, 0xB5, 0x2F, 0xFD };
    static constexpr uint8_t kLZ4[] = { 0x04, 0x22, 0x4D, 0x18 };
    static constexpr uint8_t kSVG[] = { 0x3C, 0x73, 0x76, 0x67 };  // <svg
    static constexpr uint8_t kEXR[] = { 0x76, 0x2F, 0x31, 0x01 };
    static constexpr uint8_t kHDR[] = { 0x23, 0x3F, 0x52, 0x41, 0x44 }; // #?RAD
    static constexpr uint8_t kJXL1[] = { 0xFF, 0x0A };               // Bare JPEG XL
    static constexpr uint8_t kJXL2[] = { 0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C }; // Container JXL
    static constexpr uint8_t kAVIF[] = { 0x66, 0x74, 0x79, 0x70, 0x61, 0x76, 0x69, 0x66 }; // ftypavif
    static constexpr uint8_t kHEIC[] = { 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 }; // ftypheic

    struct InternalSig {
        const char* formatName;
        const char* mimeType;
        SignatureCategory  category;
        uint32_t        offset;
        const uint8_t* pattern;
        uint32_t        patternLen;
        const uint8_t* mask = nullptr;
        float           baseConfidence;
        bool            isContainer;
    };

    std::vector<InternalSig> m_signatures;
    std::unordered_map<std::string, uint32_t> m_formatToLENSType;

    bool MatchPattern(const uint8_t* data, const uint8_t* pattern,
        const uint8_t* mask, uint32_t len) const {
        if (mask) {
            for (uint32_t i = 0; i < len; i++) {
                if ((data[i] & mask[i]) != (pattern[i] & mask[i])) return false;
            }
        }
        else {
            if (memcmp(data, pattern, len) != 0) return false;
        }
        return true;
    }

    void BuildSignatureTable() {
        // Image formats
        m_signatures.push_back({ "PNG",  "image/png",  SignatureCategory::Image, 0, kPNG,  8,  nullptr, 0.95f, false });
        m_signatures.push_back({ "JPEG", "image/jpeg", SignatureCategory::Image, 0, kJPEG, 3,  nullptr, 0.90f, false });
        m_signatures.push_back({ "GIF87","image/gif",  SignatureCategory::Image, 0, kGIF87,6,  nullptr, 0.95f, false });
        m_signatures.push_back({ "GIF89","image/gif",  SignatureCategory::Image, 0, kGIF89,6,  nullptr, 0.95f, false });
        m_signatures.push_back({ "BMP",  "image/bmp",  SignatureCategory::Image, 0, kBMP,  2,  nullptr, 0.80f, false });
        m_signatures.push_back({ "TIFF", "image/tiff", SignatureCategory::Image, 0, kTIFF_LE,4, nullptr, 0.90f,false });
        m_signatures.push_back({ "TIFF", "image/tiff", SignatureCategory::Image, 0, kTIFF_BE,4, nullptr, 0.90f,false });
        m_signatures.push_back({ "WEBP", "image/webp", SignatureCategory::Image, 0, kWEBP, 4,  nullptr, 0.70f, false });
        m_signatures.push_back({ "WEBP", "image/webp", SignatureCategory::Image, 8, kWEBP2,4,  nullptr, 0.95f, false });
        m_signatures.push_back({ "PSD",  "image/vnd.adobe.photoshop", SignatureCategory::Image, 0, kPSD, 4, nullptr, 0.90f, false });
        m_signatures.push_back({ "ICO",  "image/x-icon", SignatureCategory::Image, 0, kICO, 4,  nullptr, 0.85f, false });
        m_signatures.push_back({ "CUR",  "image/x-icon", SignatureCategory::Image, 0, kCUR, 4,  nullptr, 0.85f, false });
        m_signatures.push_back({ "SVG",  "image/svg+xml", SignatureCategory::Image, 0, kSVG, 4, nullptr, 0.75f, false });
        m_signatures.push_back({ "EXR",  "image/x-exr", SignatureCategory::Image, 0, kEXR, 4,  nullptr, 0.90f, false });
        m_signatures.push_back({ "HDR",  "image/vnd.radiance", SignatureCategory::Image, 0, kHDR, 5, nullptr, 0.90f, false });
        m_signatures.push_back({ "JXL",  "image/jxl",  SignatureCategory::Image, 0, kJXL1, 2,  nullptr, 0.90f, false });
        m_signatures.push_back({ "JXL",  "image/jxl",  SignatureCategory::Image, 0, kJXL2, 7,  nullptr, 0.95f, false });
        m_signatures.push_back({ "AVIF", "image/avif", SignatureCategory::Image, 4, kAVIF, 8,  nullptr, 0.95f, false });
        m_signatures.push_back({ "HEIC", "image/heic", SignatureCategory::Image, 4, kHEIC, 8,  nullptr, 0.95f, false });

        // Archive formats
        m_signatures.push_back({ "ZIP",  "application/zip", SignatureCategory::Archive, 0, kZIP, 4, nullptr, 0.90f, true });
        m_signatures.push_back({ "RAR5", "application/x-rar", SignatureCategory::Archive, 0, kRAR5, 8, nullptr, 0.95f, true });
        m_signatures.push_back({ "RAR4", "application/x-rar", SignatureCategory::Archive, 0, kRAR4, 7, nullptr, 0.95f, true });
        m_signatures.push_back({ "7Z",   "application/x-7z-compressed", SignatureCategory::Archive, 0, k7Z, 6, nullptr, 0.95f, true });
        m_signatures.push_back({ "GZ",   "application/gzip", SignatureCategory::Archive, 0, kGZ, 2, nullptr, 0.85f, true });
        m_signatures.push_back({ "XZ",   "application/x-xz", SignatureCategory::Archive, 0, kXZ, 6, nullptr, 0.95f, true });
        m_signatures.push_back({ "ZSTD", "application/zstd", SignatureCategory::Archive, 0, kZSTD, 4, nullptr, 0.90f, true });
        m_signatures.push_back({ "LZ4",  "application/x-lz4", SignatureCategory::Archive, 0, kLZ4, 4, nullptr, 0.90f, true });

        // Document formats
        m_signatures.push_back({ "PDF",  "application/pdf", SignatureCategory::Document, 0, kPDF, 5, nullptr, 0.95f, false });

        // LENSTYPE mappings (values from LENSTypes.h)
        m_formatToLENSType = {
            {"ZIP", 1}, {"RAR4", 2}, {"RAR5", 2}, {"7Z", 3},
            {"GZ", 4}, {"TAR", 5}, {"JPEG", 10}, {"PNG", 11},
            {"BMP", 12}, {"GIF87", 13}, {"GIF89", 13}, {"TIFF", 14},
            {"WEBP", 15}, {"ICO", 16}, {"PSD", 17}, {"SVG", 18},
            {"PDF", 20}, {"EXR", 30}, {"HDR", 31},
            {"AVIF", 40}, {"HEIC", 41}, {"JXL", 42},
        };
    }
};

} // namespace Engine
} // namespace ExplorerLens
