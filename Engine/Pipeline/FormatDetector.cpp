//==============================================================================
// DarkThumbs Engine - Format Detector Implementation
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "FormatDetector.h"
#include <windows.h>
#include <shlwapi.h>
#include <cwchar>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")

namespace DarkThumbs {
namespace Engine {

//==============================================================================
// Helper Functions
//==============================================================================

namespace {

// Case-insensitive string comparison
bool StrEqualI(const wchar_t* a, const wchar_t* b)
{
    if (!a || !b) return false;
    return _wcsicmp(a, b) == 0;
}

// Check if string ends with suffix (case-insensitive)
bool EndsWith(const wchar_t* str, const wchar_t* suffix)
{
    if (!str || !suffix) return false;
    
    size_t strLen = wcslen(str);
    size_t suffixLen = wcslen(suffix);
    
    if (suffixLen > strLen) return false;
    
    return StrEqualI(str + strLen - suffixLen, suffix);
}

} // anonymous namespace

//==============================================================================
// Constructor / Destructor
//==============================================================================

FormatDetector::FormatDetector()
{
}

FormatDetector::~FormatDetector()
{
}

//==============================================================================
// Public Methods - Format Detection
//==============================================================================

FormatType FormatDetector::DetectFormat(const wchar_t* filePath)
{
    if (!filePath) {
        return FormatType::Unknown;
    }
    
    // Try extension-based detection first (fast)
    const wchar_t* ext = GetExtension(filePath);
    if (ext) {
        FormatType type = DetectFromExtension(ext);
        if (type != FormatType::Unknown) {
            return type;
        }
    }
    
    // Fall back to signature detection (slower but more reliable)
    return DetectFromSignature(filePath);
}

FormatType FormatDetector::DetectFromExtension(const wchar_t* extension)
{
    if (!extension || extension[0] == L'\0') {
        return FormatType::Unknown;
    }
    
    // Ensure extension starts with dot
    const wchar_t* ext = extension;
    if (ext[0] != L'.') {
        // If no leading dot, we need to handle this
        // For now, return Unknown
        return FormatType::Unknown;
    }
    
    // Image formats
    if (StrEqualI(ext, L".jpg") || StrEqualI(ext, L".jpeg")) return FormatType::ImageJPEG;
    if (StrEqualI(ext, L".png")) return FormatType::ImagePNG;
    if (StrEqualI(ext, L".bmp") || StrEqualI(ext, L".dib")) return FormatType::ImageBMP;
    if (StrEqualI(ext, L".gif")) return FormatType::ImageGIF;
    if (StrEqualI(ext, L".tif") || StrEqualI(ext, L".tiff")) return FormatType::ImageTIFF;
    if (StrEqualI(ext, L".webp")) return FormatType::ImageWEBP;
    if (StrEqualI(ext, L".avif")) return FormatType::ImageAVIF;
    if (StrEqualI(ext, L".heif") || StrEqualI(ext, L".heic")) return FormatType::ImageHEIF;
    if (StrEqualI(ext, L".jxl")) return FormatType::ImageJXL;
    if (StrEqualI(ext, L".ico")) return FormatType::ImageICO;
    if (StrEqualI(ext, L".raw") || StrEqualI(ext, L".cr2") || StrEqualI(ext, L".nef") || 
        StrEqualI(ext, L".arw") || StrEqualI(ext, L".dng")) return FormatType::ImageRAW;
    
    // Archive formats
    if (StrEqualI(ext, L".zip") || StrEqualI(ext, L".cbz")) return FormatType::ArchiveZIP;
    if (StrEqualI(ext, L".rar") || StrEqualI(ext, L".cbr")) return FormatType::ArchiveRAR;
    if (StrEqualI(ext, L".7z") || StrEqualI(ext, L".cb7")) return FormatType::Archive7Z;
    if (StrEqualI(ext, L".tar") || StrEqualI(ext, L".cbt")) return FormatType::ArchiveTAR;
    
    // Document formats
    if (StrEqualI(ext, L".pdf")) return FormatType::DocumentPDF;
    if (StrEqualI(ext, L".epub")) return FormatType::DocumentEPUB;
    
    // Video formats
    if (StrEqualI(ext, L".mp4")) return FormatType::VideoMP4;
    if (StrEqualI(ext, L".mkv")) return FormatType::VideoMKV;
    if (StrEqualI(ext, L".avi")) return FormatType::VideoAVI;
    
    // Audio formats
    if (StrEqualI(ext, L".mp3")) return FormatType::AudioMP3;
    if (StrEqualI(ext, L".flac")) return FormatType::AudioFLAC;
    
    return FormatType::Unknown;
}

FormatType FormatDetector::DetectFromSignature(const wchar_t* filePath)
{
    if (!filePath) {
        return FormatType::Unknown;
    }
    
    // Open file for reading
    HANDLE hFile = CreateFileW(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return FormatType::Unknown;
    }
    
    // Read first 16 bytes (enough for most signatures)
    BYTE signature[16] = {0};
    DWORD bytesRead = 0;
    BOOL success = ReadFile(hFile, signature, sizeof(signature), &bytesRead, nullptr);
    CloseHandle(hFile);
    
    if (!success || bytesRead < 4) {
        return FormatType::Unknown;
    }
    
    // Check signatures
    
    // JPEG: FF D8 FF
    if (signature[0] == 0xFF && signature[1] == 0xD8 && signature[2] == 0xFF) {
        return FormatType::ImageJPEG;
    }
    
    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (signature[0] == 0x89 && signature[1] == 0x50 && 
        signature[2] == 0x4E && signature[3] == 0x47) {
        return FormatType::ImagePNG;
    }
    
    // BMP: 42 4D
    if (signature[0] == 0x42 && signature[1] == 0x4D) {
        return FormatType::ImageBMP;
    }
    
    // GIF: 47 49 46 38 (GIF8)
    if (signature[0] == 0x47 && signature[1] == 0x49 && 
        signature[2] == 0x46 && signature[3] == 0x38) {
        return FormatType::ImageGIF;
    }
    
    // TIFF: 49 49 2A 00 (little-endian) or 4D 4D 00 2A (big-endian)
    if ((signature[0] == 0x49 && signature[1] == 0x49 && signature[2] == 0x2A && signature[3] == 0x00) ||
        (signature[0] == 0x4D && signature[1] == 0x4D && signature[2] == 0x00 && signature[3] == 0x2A)) {
        return FormatType::ImageTIFF;
    }
    
    // WebP: RIFF .... WEBP
    if (signature[0] == 0x52 && signature[1] == 0x49 && 
        signature[2] == 0x46 && signature[3] == 0x46 &&
        bytesRead >= 12 && signature[8] == 0x57 && signature[9] == 0x45 && 
        signature[10] == 0x42 && signature[11] == 0x50) {
        return FormatType::ImageWEBP;
    }
    
    // ZIP (and CBZ, EPUB): 50 4B 03 04 or 50 4B 05 06 or 50 4B 07 08
    if (signature[0] == 0x50 && signature[1] == 0x4B && 
        (signature[2] == 0x03 || signature[2] == 0x05 || signature[2] == 0x07)) {
        return FormatType::ArchiveZIP;
    }
    
    // RAR: 52 61 72 21 1A 07 (Rar!)
    if (signature[0] == 0x52 && signature[1] == 0x61 && 
        signature[2] == 0x72 && signature[3] == 0x21) {
        return FormatType::ArchiveRAR;
    }
    
    // 7z: 37 7A BC AF 27 1C
    if (signature[0] == 0x37 && signature[1] == 0x7A && 
        signature[2] == 0xBC && signature[3] == 0xAF && 
        signature[4] == 0x27 && signature[5] == 0x1C) {
        return FormatType::Archive7Z;
    }
    
    // PDF: 25 50 44 46 (%PDF)
    if (signature[0] == 0x25 && signature[1] == 0x50 && 
        signature[2] == 0x44 && signature[3] == 0x46) {
        return FormatType::DocumentPDF;
    }
    
    return FormatType::Unknown;
}

//==============================================================================
// Public Methods - Format Checking
//==============================================================================

bool FormatDetector::IsImageFormat(const wchar_t* extension) const
{
    FormatType type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);
    
    return (type >= FormatType::ImageJPEG && type <= FormatType::ImageRAW);
}

bool FormatDetector::IsArchiveFormat(const wchar_t* extension) const
{
    FormatType type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);
    
    return (type >= FormatType::ArchiveZIP && type <= FormatType::ArchiveTAR);
}

bool FormatDetector::IsDocumentFormat(const wchar_t* extension) const
{
    FormatType type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);
    
    return (type >= FormatType::DocumentPDF && type <= FormatType::DocumentEPUB);
}

bool FormatDetector::IsVideoFormat(const wchar_t* extension) const
{
    FormatType type = const_cast<FormatDetector*>(this)->DetectFromExtension(extension);
    
    return (type >= FormatType::VideoMP4 && type <= FormatType::VideoAVI);
}

const wchar_t* FormatDetector::GetExtension(const wchar_t* filePath) const
{
    if (!filePath) {
        return nullptr;
    }
    
    // Use PathFindExtension from shlwapi
    return PathFindExtensionW(filePath);
}

} // namespace Engine
} // namespace DarkThumbs
