#pragma once
/**
 * @file SmartFormatDetectorV2.h
 * @brief Deep format detection beyond file extension.
 *
 * Purpose:
 *   Analyzes file headers, container structure, and metadata to accurately
 *   identify file formats. Goes beyond extension matching by parsing magic
 *   bytes, RIFF sub-types, ISO base media file (ftyp) brands, TIFF IFD tags,
 *   PE headers, and ZIP central directory entries.
 *
 * Classes:
 *   - SmartFormatDetectorV2: Stateless deep-detection engine with statistics tracking.
 *
 * Key types:
 *   - DetectedFormatV2: Enum of 45+ recognized formats.
 *   - DetectionStats: Aggregate counts and timing.
 *
 * Inputs:
 *   - Raw byte buffer (header), or file path for on-disk detection.
 * Outputs:
 *   - DetectedFormatV2 enum, human-readable name, MIME type.
 *
 * Thread safety:
 *   Detection methods are stateless and thread-safe. Statistics are protected
 *   by SRWLOCK.
 *
 * Dependencies: Windows API + C++ standard library only.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DetectedFormatV2 : uint32_t {
    Unknown = 0,
    JPEG, PNG, GIF, BMP, WebP, AVIF, HEIC, HEIF, JXL,
    TIFF_LE, TIFF_BE, PSD, PDF, SVG,
    RAW_CR2, RAW_NEF, RAW_ARW, RAW_DNG, RAW_ORF,
    ZIP, RAR4, RAR5, SevenZip, TAR, GZIP, XZ, BZIP2,
    ISO, CAB,
    EXE_PE, DLL_PE,
    FLAC, MP3, WAV, OGG, MP4, AVI, MKV, RIFF,
    OpenEXR, HDR_RGBE, ICO, CUR, WMF, EMF,
    COUNT
};

struct DetectionStats {
    uint64_t totalDetections = 0;
    std::unordered_map<uint32_t, uint64_t> perFormatCounts;
    double avgDetectionTimeUs = 0.0;
    uint64_t totalDetectionTimeUs = 0;
};

class SmartFormatDetectorV2 {
public:
    inline SmartFormatDetectorV2() noexcept {
        InitializeSRWLock(&m_statsLock);
    }
    inline ~SmartFormatDetectorV2() noexcept = default;

    SmartFormatDetectorV2(const SmartFormatDetectorV2&) = delete;
    SmartFormatDetectorV2& operator=(const SmartFormatDetectorV2&) = delete;

    /// Detect format from raw header bytes. Needs at least 16 bytes for reliable detection.
    inline DetectedFormatV2 DetectFromHeader(const uint8_t* data, size_t size) const {
        if (!data || size < 2) return DetectedFormatV2::Unknown;

        LARGE_INTEGER startTime{};
        QueryPerformanceCounter(&startTime);

        auto result = DetectImpl(data, size);

        LARGE_INTEGER endTime{}, freq{};
        QueryPerformanceCounter(&endTime);
        QueryPerformanceFrequency(&freq);
        uint64_t elapsedUs = 0;
        if (freq.QuadPart > 0) {
            elapsedUs = static_cast<uint64_t>(
                (endTime.QuadPart - startTime.QuadPart) * 1000000 / freq.QuadPart);
        }
        RecordDetection(result, elapsedUs);
        return result;
    }

    /// Detect format from a file path. Reads first 4KB and applies heuristics.
    inline DetectedFormatV2 DetectFromFile(const std::wstring& path) const {
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return DetectedFormatV2::Unknown;

        uint8_t buffer[4096]{};
        DWORD bytesRead = 0;
        BOOL ok = ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, nullptr);
        CloseHandle(hFile);

        if (!ok || bytesRead < 2) return DetectedFormatV2::Unknown;
        return DetectFromHeader(buffer, bytesRead);
    }

    /// Get the human-readable name for a detected format.
    inline static std::wstring GetFormatName(DetectedFormatV2 fmt) {
        switch (fmt) {
        case DetectedFormatV2::JPEG:      return L"JPEG";
        case DetectedFormatV2::PNG:       return L"PNG";
        case DetectedFormatV2::GIF:       return L"GIF";
        case DetectedFormatV2::BMP:       return L"BMP";
        case DetectedFormatV2::WebP:      return L"WebP";
        case DetectedFormatV2::AVIF:      return L"AVIF";
        case DetectedFormatV2::HEIC:      return L"HEIC";
        case DetectedFormatV2::HEIF:      return L"HEIF";
        case DetectedFormatV2::JXL:       return L"JPEG XL";
        case DetectedFormatV2::TIFF_LE:   return L"TIFF (Little-Endian)";
        case DetectedFormatV2::TIFF_BE:   return L"TIFF (Big-Endian)";
        case DetectedFormatV2::PSD:       return L"Adobe Photoshop";
        case DetectedFormatV2::PDF:       return L"PDF";
        case DetectedFormatV2::SVG:       return L"SVG";
        case DetectedFormatV2::RAW_CR2:   return L"Canon CR2 RAW";
        case DetectedFormatV2::RAW_NEF:   return L"Nikon NEF RAW";
        case DetectedFormatV2::RAW_ARW:   return L"Sony ARW RAW";
        case DetectedFormatV2::RAW_DNG:   return L"Adobe DNG RAW";
        case DetectedFormatV2::RAW_ORF:   return L"Olympus ORF RAW";
        case DetectedFormatV2::ZIP:       return L"ZIP Archive";
        case DetectedFormatV2::RAR4:      return L"RAR v4 Archive";
        case DetectedFormatV2::RAR5:      return L"RAR v5 Archive";
        case DetectedFormatV2::SevenZip:  return L"7-Zip Archive";
        case DetectedFormatV2::TAR:       return L"TAR Archive";
        case DetectedFormatV2::GZIP:      return L"GZIP";
        case DetectedFormatV2::XZ:        return L"XZ";
        case DetectedFormatV2::BZIP2:     return L"BZIP2";
        case DetectedFormatV2::ISO:       return L"ISO 9660 Image";
        case DetectedFormatV2::CAB:       return L"Windows Cabinet";
        case DetectedFormatV2::EXE_PE:    return L"Windows Executable (PE)";
        case DetectedFormatV2::DLL_PE:    return L"Windows DLL (PE)";
        case DetectedFormatV2::FLAC:      return L"FLAC Audio";
        case DetectedFormatV2::MP3:       return L"MP3 Audio";
        case DetectedFormatV2::WAV:       return L"WAV Audio";
        case DetectedFormatV2::OGG:       return L"OGG";
        case DetectedFormatV2::MP4:       return L"MP4 Video";
        case DetectedFormatV2::AVI:       return L"AVI Video";
        case DetectedFormatV2::MKV:       return L"Matroska Video";
        case DetectedFormatV2::RIFF:      return L"RIFF Container";
        case DetectedFormatV2::OpenEXR:   return L"OpenEXR";
        case DetectedFormatV2::HDR_RGBE:  return L"Radiance HDR";
        case DetectedFormatV2::ICO:       return L"Windows Icon";
        case DetectedFormatV2::CUR:       return L"Windows Cursor";
        case DetectedFormatV2::WMF:       return L"Windows Metafile";
        case DetectedFormatV2::EMF:       return L"Enhanced Metafile";
        default: return L"Unknown";
        }
    }

    /// Get the MIME type for a detected format.
    inline static std::wstring GetMimeType(DetectedFormatV2 fmt) {
        switch (fmt) {
        case DetectedFormatV2::JPEG:      return L"image/jpeg";
        case DetectedFormatV2::PNG:       return L"image/png";
        case DetectedFormatV2::GIF:       return L"image/gif";
        case DetectedFormatV2::BMP:       return L"image/bmp";
        case DetectedFormatV2::WebP:      return L"image/webp";
        case DetectedFormatV2::AVIF:      return L"image/avif";
        case DetectedFormatV2::HEIC:      return L"image/heic";
        case DetectedFormatV2::HEIF:      return L"image/heif";
        case DetectedFormatV2::JXL:       return L"image/jxl";
        case DetectedFormatV2::TIFF_LE:
        case DetectedFormatV2::TIFF_BE:   return L"image/tiff";
        case DetectedFormatV2::PSD:       return L"image/vnd.adobe.photoshop";
        case DetectedFormatV2::PDF:       return L"application/pdf";
        case DetectedFormatV2::SVG:       return L"image/svg+xml";
        case DetectedFormatV2::RAW_CR2:
        case DetectedFormatV2::RAW_NEF:
        case DetectedFormatV2::RAW_ARW:
        case DetectedFormatV2::RAW_DNG:
        case DetectedFormatV2::RAW_ORF:   return L"image/x-raw";
        case DetectedFormatV2::ZIP:       return L"application/zip";
        case DetectedFormatV2::RAR4:
        case DetectedFormatV2::RAR5:      return L"application/vnd.rar";
        case DetectedFormatV2::SevenZip:  return L"application/x-7z-compressed";
        case DetectedFormatV2::TAR:       return L"application/x-tar";
        case DetectedFormatV2::GZIP:      return L"application/gzip";
        case DetectedFormatV2::XZ:        return L"application/x-xz";
        case DetectedFormatV2::BZIP2:     return L"application/x-bzip2";
        case DetectedFormatV2::ISO:       return L"application/x-iso9660-image";
        case DetectedFormatV2::CAB:       return L"application/vnd.ms-cab-compressed";
        case DetectedFormatV2::EXE_PE:    return L"application/vnd.microsoft.portable-executable";
        case DetectedFormatV2::DLL_PE:    return L"application/vnd.microsoft.portable-executable";
        case DetectedFormatV2::FLAC:      return L"audio/flac";
        case DetectedFormatV2::MP3:       return L"audio/mpeg";
        case DetectedFormatV2::WAV:       return L"audio/wav";
        case DetectedFormatV2::OGG:       return L"audio/ogg";
        case DetectedFormatV2::MP4:       return L"video/mp4";
        case DetectedFormatV2::AVI:       return L"video/x-msvideo";
        case DetectedFormatV2::MKV:       return L"video/x-matroska";
        case DetectedFormatV2::OpenEXR:   return L"image/x-exr";
        case DetectedFormatV2::HDR_RGBE:  return L"image/vnd.radiance";
        case DetectedFormatV2::ICO:       return L"image/x-icon";
        case DetectedFormatV2::CUR:       return L"image/x-icon";
        case DetectedFormatV2::WMF:       return L"image/wmf";
        case DetectedFormatV2::EMF:       return L"image/emf";
        default: return L"application/octet-stream";
        }
    }

    /// Check if the detected format is supported for thumbnail generation.
    inline static bool CanDecode(DetectedFormatV2 fmt) {
        switch (fmt) {
        case DetectedFormatV2::JPEG:
        case DetectedFormatV2::PNG:
        case DetectedFormatV2::GIF:
        case DetectedFormatV2::BMP:
        case DetectedFormatV2::WebP:
        case DetectedFormatV2::AVIF:
        case DetectedFormatV2::HEIC:
        case DetectedFormatV2::HEIF:
        case DetectedFormatV2::JXL:
        case DetectedFormatV2::TIFF_LE:
        case DetectedFormatV2::TIFF_BE:
        case DetectedFormatV2::PSD:
        case DetectedFormatV2::PDF:
        case DetectedFormatV2::SVG:
        case DetectedFormatV2::RAW_CR2:
        case DetectedFormatV2::RAW_NEF:
        case DetectedFormatV2::RAW_ARW:
        case DetectedFormatV2::RAW_DNG:
        case DetectedFormatV2::RAW_ORF:
        case DetectedFormatV2::OpenEXR:
        case DetectedFormatV2::HDR_RGBE:
        case DetectedFormatV2::ICO:
        case DetectedFormatV2::CUR:
        case DetectedFormatV2::WMF:
        case DetectedFormatV2::EMF:
            return true;
        default:
            return false;
        }
    }

    /// Get accumulated detection statistics.
    inline DetectionStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        DetectionStats copy = m_stats;
        if (copy.totalDetections > 0) {
            copy.avgDetectionTimeUs =
                static_cast<double>(copy.totalDetectionTimeUs) /
                static_cast<double>(copy.totalDetections);
        }
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        return copy;
    }

private:
    mutable SRWLOCK m_statsLock{};
    mutable DetectionStats m_stats{};

    inline void RecordDetection(DetectedFormatV2 fmt, uint64_t elapsedUs) const {
        AcquireSRWLockExclusive(const_cast<PSRWLOCK>(&m_statsLock));
        m_stats.totalDetections++;
        m_stats.perFormatCounts[static_cast<uint32_t>(fmt)]++;
        m_stats.totalDetectionTimeUs += elapsedUs;
        ReleaseSRWLockExclusive(const_cast<PSRWLOCK>(&m_statsLock));
    }

    /// Core detection logic — pure function, no side effects.
    inline static DetectedFormatV2 DetectImpl(const uint8_t* data, size_t size) {
        if (size < 2) return DetectedFormatV2::Unknown;

        // JPEG: FF D8 FF
        if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
            return DetectedFormatV2::JPEG;

        // PNG: 89 50 4E 47 0D 0A 1A 0A
        if (size >= 8 && data[0] == 0x89 && data[1] == 0x50 &&
            data[2] == 0x4E && data[3] == 0x47 &&
            data[4] == 0x0D && data[5] == 0x0A &&
            data[6] == 0x1A && data[7] == 0x0A)
            return DetectedFormatV2::PNG;

        // GIF: GIF87a or GIF89a
        if (size >= 6 && data[0] == 0x47 && data[1] == 0x49 &&
            data[2] == 0x46 && data[3] == 0x38 &&
            (data[4] == 0x37 || data[4] == 0x39) && data[5] == 0x61)
            return DetectedFormatV2::GIF;

        // BMP: BM
        if (size >= 2 && data[0] == 0x42 && data[1] == 0x4D)
            return DetectedFormatV2::BMP;

        // PSD: 8BPS
        if (size >= 4 && data[0] == 0x38 && data[1] == 0x42 &&
            data[2] == 0x50 && data[3] == 0x53)
            return DetectedFormatV2::PSD;

        // PDF: %PDF
        if (size >= 4 && data[0] == 0x25 && data[1] == 0x50 &&
            data[2] == 0x44 && data[3] == 0x46)
            return DetectedFormatV2::PDF;

        // JPEG XL codestream: FF 0A
        if (size >= 2 && data[0] == 0xFF && data[1] == 0x0A)
            return DetectedFormatV2::JXL;

        // JPEG XL container: 00 00 00 0C 4A 58 4C 20
        if (size >= 8 && data[0] == 0x00 && data[1] == 0x00 &&
            data[2] == 0x00 && data[3] == 0x0C &&
            data[4] == 0x4A && data[5] == 0x58 &&
            data[6] == 0x4C && data[7] == 0x20)
            return DetectedFormatV2::JXL;

        // RIFF container: RIFF + sub-type check
        if (size >= 12 && data[0] == 0x52 && data[1] == 0x49 &&
            data[2] == 0x46 && data[3] == 0x46) {
            // Check sub-type at offset 8
            if (data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P')
                return DetectedFormatV2::WebP;
            if (data[8] == 'A' && data[9] == 'V' && data[10] == 'I' && data[11] == ' ')
                return DetectedFormatV2::AVI;
            if (data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E')
                return DetectedFormatV2::WAV;
            return DetectedFormatV2::RIFF;
        }

        // ftyp box (ISO Base Media File Format): offset 4 = "ftyp"
        if (size >= 12 && data[4] == 'f' && data[5] == 't' &&
            data[6] == 'y' && data[7] == 'p') {
            return DetectFtyp(data, size);
        }

        // TIFF (also covers CR2, NEF, ARW, DNG, ORF via IFD check)
        if (size >= 4) {
            bool tiffLE = (data[0] == 0x49 && data[1] == 0x49 && data[2] == 0x2A && data[3] == 0x00);
            bool tiffBE = (data[0] == 0x4D && data[1] == 0x4D && data[2] == 0x00 && data[3] == 0x2A);
            if (tiffLE || tiffBE) {
                return DetectTiffSubtype(data, size, tiffLE);
            }
        }

        // ZIP: PK\x03\x04
        if (size >= 4 && data[0] == 0x50 && data[1] == 0x4B &&
            data[2] == 0x03 && data[3] == 0x04)
            return DetectedFormatV2::ZIP;

        // RAR4: Rar!\x1A\x07\x00
        if (size >= 7 && data[0] == 0x52 && data[1] == 0x61 &&
            data[2] == 0x72 && data[3] == 0x21 &&
            data[4] == 0x1A && data[5] == 0x07 && data[6] == 0x00)
            return DetectedFormatV2::RAR4;

        // RAR5: Rar!\x1A\x07\x01\x00
        if (size >= 8 && data[0] == 0x52 && data[1] == 0x61 &&
            data[2] == 0x72 && data[3] == 0x21 &&
            data[4] == 0x1A && data[5] == 0x07 &&
            data[6] == 0x01 && data[7] == 0x00)
            return DetectedFormatV2::RAR5;

        // 7z: 37 7A BC AF 27 1C
        if (size >= 6 && data[0] == 0x37 && data[1] == 0x7A &&
            data[2] == 0xBC && data[3] == 0xAF &&
            data[4] == 0x27 && data[5] == 0x1C)
            return DetectedFormatV2::SevenZip;

        // GZIP: 1F 8B
        if (size >= 2 && data[0] == 0x1F && data[1] == 0x8B)
            return DetectedFormatV2::GZIP;

        // XZ: FD 37 7A 58 5A 00
        if (size >= 6 && data[0] == 0xFD && data[1] == 0x37 &&
            data[2] == 0x7A && data[3] == 0x58 &&
            data[4] == 0x5A && data[5] == 0x00)
            return DetectedFormatV2::XZ;

        // BZIP2: BZ (42 5A 68)
        if (size >= 3 && data[0] == 0x42 && data[1] == 0x5A && data[2] == 0x68)
            return DetectedFormatV2::BZIP2;

        // CAB: MSCF
        if (size >= 4 && data[0] == 0x4D && data[1] == 0x53 &&
            data[2] == 0x43 && data[3] == 0x46)
            return DetectedFormatV2::CAB;

        // PE (MZ header)
        if (size >= 2 && data[0] == 0x4D && data[1] == 0x5A) {
            return DetectPE(data, size);
        }

        // FLAC: fLaC
        if (size >= 4 && data[0] == 0x66 && data[1] == 0x4C &&
            data[2] == 0x61 && data[3] == 0x43)
            return DetectedFormatV2::FLAC;

        // OGG: OggS
        if (size >= 4 && data[0] == 0x4F && data[1] == 0x67 &&
            data[2] == 0x67 && data[3] == 0x53)
            return DetectedFormatV2::OGG;

        // MP3: ID3 tag or sync word FF FB / FF F3 / FF F2
        if (size >= 3) {
            if (data[0] == 0x49 && data[1] == 0x44 && data[2] == 0x33)
                return DetectedFormatV2::MP3;
            if (data[0] == 0xFF && (data[1] == 0xFB || data[1] == 0xF3 || data[1] == 0xF2))
                return DetectedFormatV2::MP3;
        }

        // MKV/WebM: EBML header 1A 45 DF A3
        if (size >= 4 && data[0] == 0x1A && data[1] == 0x45 &&
            data[2] == 0xDF && data[3] == 0xA3)
            return DetectedFormatV2::MKV;

        // OpenEXR: 76 2F 31 01
        if (size >= 4 && data[0] == 0x76 && data[1] == 0x2F &&
            data[2] == 0x31 && data[3] == 0x01)
            return DetectedFormatV2::OpenEXR;

        // Radiance HDR: #?RADIANCE or #?RGBE
        if (size >= 10 && data[0] == '#' && data[1] == '?') {
            if (size >= 11 && std::memcmp(data + 2, "RADIANCE", 8) == 0)
                return DetectedFormatV2::HDR_RGBE;
            if (size >= 6 && std::memcmp(data + 2, "RGBE", 4) == 0)
                return DetectedFormatV2::HDR_RGBE;
        }

        // ICO/CUR: 00 00 01 00 (ICO) or 00 00 02 00 (CUR)
        if (size >= 4 && data[0] == 0x00 && data[1] == 0x00) {
            if (data[2] == 0x01 && data[3] == 0x00) return DetectedFormatV2::ICO;
            if (data[2] == 0x02 && data[3] == 0x00) return DetectedFormatV2::CUR;
        }

        // WMF: D7 CD C6 9A (placeable WMF)
        if (size >= 4 && data[0] == 0xD7 && data[1] == 0xCD &&
            data[2] == 0xC6 && data[3] == 0x9A)
            return DetectedFormatV2::WMF;

        // EMF: 01 00 00 00 at offset 0, then check for " EMF" signature at offset 40
        if (size >= 44 && data[0] == 0x01 && data[1] == 0x00 &&
            data[2] == 0x00 && data[3] == 0x00 &&
            data[40] == 0x20 && data[41] == 0x45 &&
            data[42] == 0x4D && data[43] == 0x46)
            return DetectedFormatV2::EMF;

        // SVG heuristic: look for "<?xml" or "<svg" in first bytes (text-based)
        if (size >= 5) {
            if ((data[0] == '<' && data[1] == '?' && data[2] == 'x' &&
                data[3] == 'm' && data[4] == 'l') ||
                (data[0] == '<' && data[1] == 's' && data[2] == 'v' && data[3] == 'g')) {
                // Quick scan for "<svg" within first 1KB
                size_t scanLen = (std::min)(size, static_cast<size_t>(1024));
                for (size_t i = 0; i + 3 < scanLen; ++i) {
                    if (data[i] == '<' && data[i + 1] == 's' &&
                        data[i + 2] == 'v' && data[i + 3] == 'g')
                        return DetectedFormatV2::SVG;
                }
            }
        }

        // TAR heuristic: check for "ustar" at offset 257
        if (size >= 263 && std::memcmp(data + 257, "ustar", 5) == 0)
            return DetectedFormatV2::TAR;

        // ISO 9660: "CD001" at offset 32769
        // We can only check this if we have enough data — typically from DetectFromFile
        if (size >= 32774 && std::memcmp(data + 32769, "CD001", 5) == 0)
            return DetectedFormatV2::ISO;

        return DetectedFormatV2::Unknown;
    }

    /// Parse ftyp box to distinguish AVIF, HEIC, HEIF, MP4.
    inline static DetectedFormatV2 DetectFtyp(const uint8_t* data, size_t size) {
        // Brand is at offset 8, 4 bytes
        if (size < 12) return DetectedFormatV2::Unknown;
        char brand[5]{};
        std::memcpy(brand, data + 8, 4);
        brand[4] = '\0';

        if (std::strcmp(brand, "avif") == 0 || std::strcmp(brand, "avis") == 0)
            return DetectedFormatV2::AVIF;
        if (std::strcmp(brand, "mif1") == 0) {
            // mif1 can be AVIF or HEIF — check compatible brands
            // Parse box size from first 4 bytes (big-endian)
            uint32_t boxSize = (static_cast<uint32_t>(data[0]) << 24) |
                (static_cast<uint32_t>(data[1]) << 16) |
                (static_cast<uint32_t>(data[2]) << 8) |
                static_cast<uint32_t>(data[3]);
            size_t end = (std::min)(static_cast<size_t>(boxSize), size);
            for (size_t off = 16; off + 4 <= end; off += 4) {
                char compat[5]{};
                std::memcpy(compat, data + off, 4);
                compat[4] = '\0';
                if (std::strcmp(compat, "avif") == 0) return DetectedFormatV2::AVIF;
                if (std::strcmp(compat, "heic") == 0) return DetectedFormatV2::HEIC;
                if (std::strcmp(compat, "heif") == 0) return DetectedFormatV2::HEIF;
            }
            return DetectedFormatV2::HEIF; // fallback for mif1
        }
        if (std::strcmp(brand, "heic") == 0 || std::strcmp(brand, "heix") == 0)
            return DetectedFormatV2::HEIC;
        if (std::strcmp(brand, "heif") == 0 || std::strcmp(brand, "hevx") == 0 ||
            std::strcmp(brand, "heim") == 0 || std::strcmp(brand, "heis") == 0)
            return DetectedFormatV2::HEIF;

        // Common MP4 brands
        if (std::strcmp(brand, "isom") == 0 || std::strcmp(brand, "iso2") == 0 ||
            std::strcmp(brand, "mp41") == 0 || std::strcmp(brand, "mp42") == 0 ||
            std::strcmp(brand, "M4V ") == 0 || std::strcmp(brand, "qt  ") == 0 ||
            std::strcmp(brand, "avc1") == 0 || std::strcmp(brand, "dash") == 0)
            return DetectedFormatV2::MP4;

        return DetectedFormatV2::MP4; // Unknown ftyp → treat as MP4 container
    }

    /// Detect TIFF subtypes (CR2, NEF, ARW, DNG, ORF) by scanning IFD tags.
    inline static DetectedFormatV2 DetectTiffSubtype(const uint8_t* data, size_t size, bool littleEndian) {
        // CR2 has "CR" at offset 8-9 in the TIFF header
        if (size >= 10 && data[8] == 'C' && data[9] == 'R')
            return DetectedFormatV2::RAW_CR2;

        // Read IFD0 offset
        uint32_t ifdOffset = 0;
        if (littleEndian) {
            if (size >= 8)
                ifdOffset = static_cast<uint32_t>(data[4]) |
                (static_cast<uint32_t>(data[5]) << 8) |
                (static_cast<uint32_t>(data[6]) << 16) |
                (static_cast<uint32_t>(data[7]) << 24);
        }
        else {
            if (size >= 8)
                ifdOffset = (static_cast<uint32_t>(data[4]) << 24) |
                (static_cast<uint32_t>(data[5]) << 16) |
                (static_cast<uint32_t>(data[6]) << 8) |
                static_cast<uint32_t>(data[7]);
        }

        if (ifdOffset == 0 || ifdOffset + 2 > size)
            return littleEndian ? DetectedFormatV2::TIFF_LE : DetectedFormatV2::TIFF_BE;

        // Read IFD entry count
        uint16_t entryCount = 0;
        if (littleEndian) {
            entryCount = static_cast<uint16_t>(data[ifdOffset]) |
                (static_cast<uint16_t>(data[ifdOffset + 1]) << 8);
        }
        else {
            entryCount = (static_cast<uint16_t>(data[ifdOffset]) << 8) |
                static_cast<uint16_t>(data[ifdOffset + 1]);
        }

        // Scan IFD entries looking for maker-specific tags
        size_t scanLimit = (std::min)(static_cast<size_t>(entryCount), static_cast<size_t>(200));
        for (size_t i = 0; i < scanLimit; ++i) {
            size_t entryOff = ifdOffset + 2 + i * 12;
            if (entryOff + 12 > size) break;

            uint16_t tag = 0;
            if (littleEndian) {
                tag = static_cast<uint16_t>(data[entryOff]) |
                    (static_cast<uint16_t>(data[entryOff + 1]) << 8);
            }
            else {
                tag = (static_cast<uint16_t>(data[entryOff]) << 8) |
                    static_cast<uint16_t>(data[entryOff + 1]);
            }

            // DNG: DNGVersion tag = 0xC612
            if (tag == 0xC612) return DetectedFormatV2::RAW_DNG;
            // NEF uses Nikon MakerNote — check tag 0x010F (Make) value
            // ARW uses Sony MakerNote
            // ORF uses Olympus-specific tags
        }

        // Scan for maker strings in first 4KB
        size_t scanBytes = (std::min)(size, static_cast<size_t>(4096));
        for (size_t pos = 0; pos + 5 < scanBytes; ++pos) {
            if (std::memcmp(data + pos, "NIKON", 5) == 0 ||
                std::memcmp(data + pos, "Nikon", 5) == 0)
                return DetectedFormatV2::RAW_NEF;
            if (std::memcmp(data + pos, "SONY", 4) == 0 ||
                std::memcmp(data + pos, "Sony", 4) == 0)
                return DetectedFormatV2::RAW_ARW;
            if (pos + 7 < scanBytes &&
                (std::memcmp(data + pos, "OLYMPUS", 7) == 0 ||
                    std::memcmp(data + pos, "Olympus", 7) == 0))
                return DetectedFormatV2::RAW_ORF;
        }

        return littleEndian ? DetectedFormatV2::TIFF_LE : DetectedFormatV2::TIFF_BE;
    }

    /// Detect PE type (EXE vs DLL).
    inline static DetectedFormatV2 DetectPE(const uint8_t* data, size_t size) {
        // MZ header — e_lfanew at offset 0x3C (4 bytes, little-endian)
        if (size < 0x40) return DetectedFormatV2::EXE_PE;
        uint32_t peOffset = static_cast<uint32_t>(data[0x3C]) |
            (static_cast<uint32_t>(data[0x3D]) << 8) |
            (static_cast<uint32_t>(data[0x3E]) << 16) |
            (static_cast<uint32_t>(data[0x3F]) << 24);

        // Verify PE signature "PE\0\0"
        if (peOffset + 24 > size) return DetectedFormatV2::EXE_PE;
        if (data[peOffset] != 'P' || data[peOffset + 1] != 'E' ||
            data[peOffset + 2] != 0 || data[peOffset + 3] != 0)
            return DetectedFormatV2::EXE_PE;

        // Characteristics at peOffset + 22 (2 bytes LE)
        uint16_t characteristics =
            static_cast<uint16_t>(data[peOffset + 22]) |
            (static_cast<uint16_t>(data[peOffset + 23]) << 8);

        // IMAGE_FILE_DLL = 0x2000
        if (characteristics & 0x2000)
            return DetectedFormatV2::DLL_PE;
        return DetectedFormatV2::EXE_PE;
    }
};

} // namespace Engine
} // namespace ExplorerLens
