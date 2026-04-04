//==============================================================================
// ExplorerLens Engine - Format Detector Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "FormatDetector.h"
#include <shlwapi.h>
#include <windows.h>
#include <algorithm>
#include <cwchar>

#pragma comment(lib, "shlwapi.lib")

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Helper Functions
//==============================================================================

namespace {

// Case-insensitive string comparison (optimized with early length check)
inline bool StrEqualI(const wchar_t* a, const wchar_t* b)
{
    if (!a || !b)
        return false;
    // Quick length check before full comparison
    size_t lenA = wcslen(a);
    size_t lenB = wcslen(b);
    if (lenA != lenB)
        return false;
    return _wcsicmp(a, b) == 0;
}

}  // anonymous namespace

//==============================================================================
// Constructor / Destructor
//==============================================================================

FormatDetector::FormatDetector() {}

FormatDetector::~FormatDetector() {}

//==============================================================================
// Public Methods - Format Detection
//==============================================================================

DetectedFormat FormatDetector::DetectFormat(const wchar_t* filePath)
{
    if (!filePath) {
        return DetectedFormat::Unknown;
    }

    // Try extension-based detection first (fast)
    const wchar_t* ext = GetExtension(filePath);
    if (ext) {
        DetectedFormat type = DetectFromExtension(ext);
        if (type != DetectedFormat::Unknown) {
            return type;
        }
    }

    // Fall back to signature detection (slower but more reliable)
    return DetectFromSignature(filePath);
}

DetectedFormat FormatDetector::DetectFromExtension(const wchar_t* extension)
{
    if (!extension || extension[0] == L'\0') {
        return DetectedFormat::Unknown;
    }

    // Ensure extension starts with dot
    const wchar_t* ext = extension;
    if (ext[0] != L'.') {
        // If no leading dot, we need to handle this
        // For now, return Unknown
        return DetectedFormat::Unknown;
    }

    // Image formats
    if (StrEqualI(ext, L".jpg") || StrEqualI(ext, L".jpeg"))
        return DetectedFormat::ImageJPEG;
    if (StrEqualI(ext, L".png"))
        return DetectedFormat::ImagePNG;
    if (StrEqualI(ext, L".bmp") || StrEqualI(ext, L".dib"))
        return DetectedFormat::ImageBMP;
    if (StrEqualI(ext, L".gif"))
        return DetectedFormat::ImageGIF;
    if (StrEqualI(ext, L".tif") || StrEqualI(ext, L".tiff"))
        return DetectedFormat::ImageTIFF;
    if (StrEqualI(ext, L".webp"))
        return DetectedFormat::ImageWEBP;
    if (StrEqualI(ext, L".avif"))
        return DetectedFormat::ImageAVIF;
    if (StrEqualI(ext, L".heif") || StrEqualI(ext, L".heic"))
        return DetectedFormat::ImageHEIF;
    if (StrEqualI(ext, L".jxl"))
        return DetectedFormat::ImageJXL;
    if (StrEqualI(ext, L".ico"))
        return DetectedFormat::ImageICO;
    if (StrEqualI(ext, L".raw") || StrEqualI(ext, L".cr2") || StrEqualI(ext, L".nef") || StrEqualI(ext, L".arw")
        || StrEqualI(ext, L".dng"))
        return DetectedFormat::ImageRAW;

    // Professional image formats
    if (StrEqualI(ext, L".psd") || StrEqualI(ext, L".psb"))
        return DetectedFormat::ImagePSD;
    if (StrEqualI(ext, L".dds"))
        return DetectedFormat::ImageDDS;
    if (StrEqualI(ext, L".hdr"))
        return DetectedFormat::ImageHDR;
    if (StrEqualI(ext, L".exr"))
        return DetectedFormat::ImageEXR;
    if (StrEqualI(ext, L".ppm") || StrEqualI(ext, L".pgm") || StrEqualI(ext, L".pbm") || StrEqualI(ext, L".pnm")
        || StrEqualI(ext, L".pam") || StrEqualI(ext, L".pfm"))
        return DetectedFormat::ImagePPM;
    if (StrEqualI(ext, L".tga"))
        return DetectedFormat::ImageTGA;
    if (StrEqualI(ext, L".qoi"))
        return DetectedFormat::ImageQOI;
    if (StrEqualI(ext, L".svg") || StrEqualI(ext, L".svgz"))
        return DetectedFormat::ImageSVG;

    // Archive formats
    if (StrEqualI(ext, L".zip") || StrEqualI(ext, L".cbz"))
        return DetectedFormat::ArchiveZIP;
    if (StrEqualI(ext, L".rar") || StrEqualI(ext, L".cbr"))
        return DetectedFormat::ArchiveRAR;
    if (StrEqualI(ext, L".7z") || StrEqualI(ext, L".cb7"))
        return DetectedFormat::Archive7Z;
    if (StrEqualI(ext, L".tar") || StrEqualI(ext, L".cbt"))
        return DetectedFormat::ArchiveTAR;

    // Document formats
    if (StrEqualI(ext, L".pdf"))
        return DetectedFormat::DocumentPDF;
    if (StrEqualI(ext, L".epub"))
        return DetectedFormat::DocumentEPUB;

    // Video formats
    if (StrEqualI(ext, L".mp4") || StrEqualI(ext, L".m4v"))
        return DetectedFormat::VideoMP4;
    if (StrEqualI(ext, L".mkv"))
        return DetectedFormat::VideoMKV;
    if (StrEqualI(ext, L".avi"))
        return DetectedFormat::VideoAVI;
    if (StrEqualI(ext, L".wmv") || StrEqualI(ext, L".mov") || StrEqualI(ext, L".flv") || StrEqualI(ext, L".webm")
        || StrEqualI(ext, L".mpg") || StrEqualI(ext, L".mpeg") || StrEqualI(ext, L".ts") || StrEqualI(ext, L".mts")
        || StrEqualI(ext, L".m2ts") || StrEqualI(ext, L".3gp") || StrEqualI(ext, L".vob") || StrEqualI(ext, L".ogv")
        || StrEqualI(ext, L".asf"))
        return DetectedFormat::VideoMP4;  // Generic video

    // Audio formats
    if (StrEqualI(ext, L".mp3"))
        return DetectedFormat::AudioMP3;
    if (StrEqualI(ext, L".flac"))
        return DetectedFormat::AudioFLAC;
    if (StrEqualI(ext, L".wma") || StrEqualI(ext, L".aac") || StrEqualI(ext, L".m4a") || StrEqualI(ext, L".ogg")
        || StrEqualI(ext, L".opus") || StrEqualI(ext, L".wav") || StrEqualI(ext, L".aiff") || StrEqualI(ext, L".aif")
        || StrEqualI(ext, L".ape") || StrEqualI(ext, L".wv"))
        return DetectedFormat::AudioMP3;  // Generic audio

    // Document formats (expanded)
    if (StrEqualI(ext, L".epub"))
        return DetectedFormat::DocumentEPUB;
    if (StrEqualI(ext, L".mobi") || StrEqualI(ext, L".azw") || StrEqualI(ext, L".azw3") || StrEqualI(ext, L".fb2"))
        return DetectedFormat::DocumentEPUB;  // eBook category
    if (StrEqualI(ext, L".docx") || StrEqualI(ext, L".doc") || StrEqualI(ext, L".rtf") || StrEqualI(ext, L".odt")
        || StrEqualI(ext, L".xps") || StrEqualI(ext, L".oxps") || StrEqualI(ext, L".djvu") || StrEqualI(ext, L".djv"))
        return DetectedFormat::DocumentPDF;  // Doc category
    if (StrEqualI(ext, L".xlsx") || StrEqualI(ext, L".xls") || StrEqualI(ext, L".ods") || StrEqualI(ext, L".pptx")
        || StrEqualI(ext, L".ppt") || StrEqualI(ext, L".odp"))
        return DetectedFormat::DocumentPDF;

    // Font formats
    if (StrEqualI(ext, L".ttf") || StrEqualI(ext, L".otf") || StrEqualI(ext, L".woff") || StrEqualI(ext, L".woff2")
        || StrEqualI(ext, L".ttc") || StrEqualI(ext, L".fon") || StrEqualI(ext, L".fnt"))
        return DetectedFormat::DocumentPDF;  // Font category

    return DetectedFormat::Unknown;
}

DetectedFormat FormatDetector::DetectFromSignature(const wchar_t* filePath)
{
    if (!filePath) {
        return DetectedFormat::Unknown;
    }

    // Open file for reading
    HANDLE hFile =
        CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return DetectedFormat::Unknown;
    }

    // Read first 16 bytes (enough for most signatures)
    BYTE signature[16] = {0};
    DWORD bytesRead = 0;
    BOOL success = ReadFile(hFile, signature, sizeof(signature), &bytesRead, nullptr);
    CloseHandle(hFile);

    if (!success || bytesRead < 4) {
        return DetectedFormat::Unknown;
    }

    // Check signatures

    // JPEG: FF D8 FF
    if (signature[0] == 0xFF && signature[1] == 0xD8 && signature[2] == 0xFF) {
        return DetectedFormat::ImageJPEG;
    }

    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (signature[0] == 0x89 && signature[1] == 0x50 && signature[2] == 0x4E && signature[3] == 0x47) {
        return DetectedFormat::ImagePNG;
    }

    // BMP: 42 4D
    if (signature[0] == 0x42 && signature[1] == 0x4D) {
        return DetectedFormat::ImageBMP;
    }

    // GIF: 47 49 46 38 (GIF8)
    if (signature[0] == 0x47 && signature[1] == 0x49 && signature[2] == 0x46 && signature[3] == 0x38) {
        return DetectedFormat::ImageGIF;
    }

    // TIFF: 49 49 2A 00 (little-endian) or 4D 4D 00 2A (big-endian)
    if ((signature[0] == 0x49 && signature[1] == 0x49 && signature[2] == 0x2A && signature[3] == 0x00)
        || (signature[0] == 0x4D && signature[1] == 0x4D && signature[2] == 0x00 && signature[3] == 0x2A)) {
        return DetectedFormat::ImageTIFF;
    }

    // WebP: RIFF .... WEBP
    if (signature[0] == 0x52 && signature[1] == 0x49 && signature[2] == 0x46 && signature[3] == 0x46 && bytesRead >= 12
        && signature[8] == 0x57 && signature[9] == 0x45 && signature[10] == 0x42 && signature[11] == 0x50) {
        return DetectedFormat::ImageWEBP;
    }

    // ZIP (and CBZ, EPUB): 50 4B 03 04 or 50 4B 05 06 or 50 4B 07 08
    if (signature[0] == 0x50 && signature[1] == 0x4B
        && (signature[2] == 0x03 || signature[2] == 0x05 || signature[2] == 0x07)) {
        return DetectedFormat::ArchiveZIP;
    }

    // RAR: 52 61 72 21 1A 07 (Rar!)
    if (signature[0] == 0x52 && signature[1] == 0x61 && signature[2] == 0x72 && signature[3] == 0x21) {
        return DetectedFormat::ArchiveRAR;
    }

    // 7z: 37 7A BC AF 27 1C
    if (signature[0] == 0x37 && signature[1] == 0x7A && signature[2] == 0xBC && signature[3] == 0xAF
        && signature[4] == 0x27 && signature[5] == 0x1C) {
        return DetectedFormat::Archive7Z;
    }

    // PDF: 25 50 44 46 (%PDF)
    if (signature[0] == 0x25 && signature[1] == 0x50 && signature[2] == 0x44 && signature[3] == 0x46) {
        return DetectedFormat::DocumentPDF;
    }

    return DetectedFormat::Unknown;
}

//==============================================================================
// Public Methods - Format Checking
//==============================================================================

bool FormatDetector::IsImageFormat(const wchar_t* extension) const
{
    DetectedFormat type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);

    return (type >= DetectedFormat::ImageJPEG && type <= DetectedFormat::ImageSVG);
}

bool FormatDetector::IsArchiveFormat(const wchar_t* extension) const
{
    DetectedFormat type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);

    return (type >= DetectedFormat::ArchiveZIP && type <= DetectedFormat::ArchiveTAR);
}

bool FormatDetector::IsDocumentFormat(const wchar_t* extension) const
{
    DetectedFormat type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);

    return (type >= DetectedFormat::DocumentPDF && type <= DetectedFormat::DocumentEPUB);
}

bool FormatDetector::IsVideoFormat(const wchar_t* extension) const
{
    DetectedFormat type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);

    return (type >= DetectedFormat::VideoMP4 && type <= DetectedFormat::VideoAVI);
}

const wchar_t* FormatDetector::GetExtension(const wchar_t* filePath) const
{
    if (!filePath) {
        return nullptr;
    }

    // Use PathFindExtension from shlwapi
    return PathFindExtensionW(filePath);
}

}  // namespace Engine
}  // namespace ExplorerLens
