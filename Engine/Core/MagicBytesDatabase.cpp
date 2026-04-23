// MagicBytesDatabase.cpp — Centralized Format Magic-Byte Registry
// Copyright (c) 2026 ExplorerLens Project
//
// This file is the authoritative source of format signatures.
// Add new formats in the s_sigs table only (do NOT scatter magic byte checks
// through individual decoders).
//
#include "MagicBytesDatabase.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstring>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Magic byte constant pools (static storage for span references)
// ---------------------------------------------------------------------------

namespace {

// ---- core image formats ----
constexpr uint8_t kJpeg[]       = { 0xFF, 0xD8, 0xFF };
constexpr uint8_t kPng[]        = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
constexpr uint8_t kGif87[]      = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
constexpr uint8_t kGif89[]      = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
constexpr uint8_t kBmpSig[]     = { 0x42, 0x4D };
constexpr uint8_t kIcoSig[]     = { 0x00, 0x00, 0x01, 0x00 };
constexpr uint8_t kTiffLE[]     = { 0x49, 0x49, 0x2A, 0x00 };
constexpr uint8_t kTiffBE[]     = { 0x4D, 0x4D, 0x00, 0x2A };
constexpr uint8_t kWebPRiff[]   = { 0x52, 0x49, 0x46, 0x46 }; // "RIFF"; check [8..11]=="WEBP"
constexpr uint8_t kWebPWebp[]   = { 0x57, 0x45, 0x42, 0x50 }; // "WEBP" at offset 8
// ---- modern formats ----
constexpr uint8_t kJxl[]        = { 0xFF, 0x0A };                            // naked codestream
constexpr uint8_t kJxlBox[]     = { 0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20 }; // container
constexpr uint8_t kAvif[]       = { 0x00, 0x00, 0x00, 0x1C, 0x66, 0x74, 0x79, 0x70,
                                    0x61, 0x76, 0x69, 0x66 }; // ftyp avif
constexpr uint8_t kHeic[]       = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70,
                                    0x68, 0x65, 0x69, 0x63 }; // ftyp heic
// ftyp box, offset 4: both AVIF and HEIF share 'ftyp'; check brand at offset 8
constexpr uint8_t kFtyp[]       = { 0x66, 0x74, 0x79, 0x70 }; // "ftyp" at offset 4
// ---- RAW ----
constexpr uint8_t kCr2[]        = { 0x49, 0x49, 0x2A, 0x00 }; // TIFF LE + CR2 maker note
constexpr uint8_t kCr3Ftyp[]    = { 0x66, 0x74, 0x79, 0x70, 0x63, 0x72, 0x78, 0x20 }; // ftypcrx
constexpr uint8_t kRafMagic[]   = { 0x46, 0x55, 0x4A, 0x49, 0x46, 0x49, 0x4C, 0x4D }; // FUJIFILM
constexpr uint8_t kMrwMagic[]   = { 0x00, 0x4D, 0x52, 0x4D }; // \0MRM
// ---- archives ----
constexpr uint8_t kZip[]        = { 0x50, 0x4B, 0x03, 0x04 };
constexpr uint8_t kZipEmpty[]   = { 0x50, 0x4B, 0x05, 0x06 };
constexpr uint8_t kRar4[]       = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
constexpr uint8_t kRar5[]       = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 };
constexpr uint8_t k7z[]         = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
constexpr uint8_t kGzip[]       = { 0x1F, 0x8B };
constexpr uint8_t kBzip2[]      = { 0x42, 0x5A, 0x68 };
constexpr uint8_t kXz[]         = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
constexpr uint8_t kZstd[]       = { 0x28, 0xB5, 0x2F, 0xFD };
constexpr uint8_t kTar[]        = { 0x75, 0x73, 0x74, 0x61, 0x72 }; // offset 257
// ---- documents ----
constexpr uint8_t kPdf[]        = { 0x25, 0x50, 0x44, 0x46 };
constexpr uint8_t kDocx[]       = { 0x50, 0x4B, 0x03, 0x04 }; // ZIP with [Content_Types].xml
constexpr uint8_t kEpub[]       = { 0x50, 0x4B, 0x03, 0x04 }; // ZIP, check mimetype entry
// ---- media ----
constexpr uint8_t kMp4Ftyp[]    = { 0x66, 0x74, 0x79, 0x70 }; // "ftyp" at offset 4
constexpr uint8_t kMkvEbml[]    = { 0x1A, 0x45, 0xDF, 0xA3 }; // EBML header
constexpr uint8_t kFlac[]       = { 0x66, 0x4C, 0x61, 0x43 }; // "fLaC"
constexpr uint8_t kMp3Id3[]     = { 0x49, 0x44, 0x33 };
constexpr uint8_t kMp3Sync[]    = { 0xFF, 0xFB };
constexpr uint8_t kOgg[]        = { 0x4F, 0x67, 0x67, 0x53 }; // "OggS"
constexpr uint8_t kWave[]       = { 0x52, 0x49, 0x46, 0x46 }; // RIFF; check [8..11]=="WAVE"
// ---- HDR ----
constexpr uint8_t kExrMagic[]   = { 0x76, 0x2F, 0x31, 0x01 }; // OpenEXR
constexpr uint8_t kHdrRadiance[]= { 0x23, 0x3F, 0x52, 0x41, 0x44, 0x49, 0x41, 0x4E, 0x43, 0x45 }; // "#?RADIANCE"
// ---- DDS / GPU texture ----
constexpr uint8_t kDds[]        = { 0x44, 0x44, 0x53, 0x20 }; // "DDS "
// ---- Photoshop ----
constexpr uint8_t kPsd[]        = { 0x38, 0x42, 0x50, 0x53 }; // "8BPS"
// ---- font ----
constexpr uint8_t kTtf[]        = { 0x00, 0x01, 0x00, 0x00, 0x00 }; // TrueType
constexpr uint8_t kOtf[]        = { 0x4F, 0x54, 0x54, 0x4F };       // "OTTO" CFF
constexpr uint8_t kWoff[]       = { 0x77, 0x4F, 0x46, 0x46 };       // "wOFF"
constexpr uint8_t kWoff2[]      = { 0x77, 0x4F, 0x46, 0x32 };       // "wOF2"
// ---- 3D ----
constexpr uint8_t kGlb[]        = { 0x67, 0x6C, 0x54, 0x46 }; // "glTF"
constexpr uint8_t kStlBin[]     = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// ---- scientific ----
constexpr uint8_t kFits[]       = { 0x53, 0x49, 0x4D, 0x50, 0x4C, 0x45, 0x20, 0x20 }; // "SIMPLE  "

// ---------------------------------------------------------------------------
// Extension→formatId map
// ---------------------------------------------------------------------------

struct ExtEntry { std::string_view ext; std::string_view formatId; };

constexpr ExtEntry kExtMap[] = {
    // JPEG
    { ".jpg",  "JPEG" }, { ".jpeg", "JPEG" }, { ".jpe",  "JPEG" },
    { ".jfif", "JPEG" }, { ".jif",  "JPEG" },
    // PNG
    { ".png",  "PNG" },
    // GIF
    { ".gif",  "GIF" },
    // BMP
    { ".bmp",  "BMP" }, { ".dib",  "BMP" },
    // TIFF
    { ".tif",  "TIFF" }, { ".tiff", "TIFF" },
    // WebP
    { ".webp", "WEBP" },
    // JPEG XL
    { ".jxl",  "JXL" },
    // AVIF
    { ".avif", "AVIF" },
    // HEIC/HEIF
    { ".heic", "HEIC" }, { ".heif", "HEIF" }, { ".hif", "HEIF" },
    // RAW
    { ".cr2",  "CR2" },  { ".cr3",  "CR3" },
    { ".nef",  "NEF" },  { ".nrw",  "NEF" },
    { ".arw",  "ARW" },  { ".srf",  "ARW" },
    { ".orf",  "ORF" },  { ".rw2",  "RW2" },
    { ".raf",  "RAF" },  { ".pef",  "PEF" },
    { ".dng",  "DNG" },  { ".dcr",  "DCR" },
    { ".mrw",  "MRW" },  { ".x3f",  "X3F" },
    { ".erf",  "ERF" },  { ".mef",  "MEF" },
    { ".ptx",  "PTX" },  { ".k25",  "K25" },
    // Archives
    { ".zip",  "ZIP" },  { ".cbz",  "CBZ" },
    { ".rar",  "RAR" },  { ".cbr",  "CBR" },
    { ".7z",   "7Z"  },  { ".cb7",  "CB7" },
    { ".tar",  "TAR" },  { ".gz",   "GZ"  },
    { ".bz2",  "BZ2" },  { ".xz",   "XZ"  },
    { ".zst",  "ZSTD"},
    // Documents
    { ".pdf",  "PDF"  },
    { ".epub", "EPUB" },
    { ".docx", "DOCX" }, { ".docm", "DOCX" },
    { ".xlsx", "XLSX" }, { ".xlsm", "XLSX" },
    { ".pptx", "PPTX" }, { ".pptm", "PPTX" },
    // Media
    { ".mp4",  "MP4" }, { ".m4v",  "MP4" }, { ".m4a",  "MP4" },
    { ".mkv",  "MKV" }, { ".webm", "WEBM"},
    { ".avi",  "AVI" }, { ".mov",  "MOV" },
    { ".flac", "FLAC"},
    { ".mp3",  "MP3" }, { ".ogg",  "OGG" },
    { ".wav",  "WAV" }, { ".opus", "OGG" },
    // HDR
    { ".exr",  "EXR" }, { ".hdr",  "HDR" },
    // Photoshop
    { ".psd",  "PSD" }, { ".psb",  "PSD" },
    // DDS
    { ".dds",  "DDS" }, { ".ktx",  "KTX" }, { ".ktx2", "KTX2"},
    // SVG
    { ".svg",  "SVG" }, { ".svgz", "SVG" },
    // Fonts
    { ".ttf",  "FONT" }, { ".otf",  "FONT" },
    { ".woff", "FONT" }, { ".woff2","FONT" }, { ".ttc",  "FONT" },
    // 3D
    { ".glb",  "GLB" }, { ".gltf", "GLTF"},
    { ".stl",  "STL" }, { ".obj",  "OBJ" }, { ".fbx",  "FBX" },
    // Scientific
    { ".fits", "FITS"}, { ".fit",  "FITS"}, { ".fts",  "FITS"},
    { ".dcm",  "DICOM"}, { ".dic", "DICOM"},
};

// ---------------------------------------------------------------------------
// FormatId → MIME type map
// ---------------------------------------------------------------------------

struct MimeEntry { std::string_view fmtId; std::string_view mime; };

constexpr MimeEntry kMimeMap[] = {
    { "JPEG",  "image/jpeg" },
    { "PNG",   "image/png" },
    { "GIF",   "image/gif" },
    { "BMP",   "image/bmp" },
    { "TIFF",  "image/tiff" },
    { "WEBP",  "image/webp" },
    { "JXL",   "image/jxl" },
    { "AVIF",  "image/avif" },
    { "HEIC",  "image/heic" },
    { "HEIF",  "image/heif" },
    { "CR2",   "image/x-canon-cr2" },
    { "CR3",   "image/x-canon-cr3" },
    { "NEF",   "image/x-nikon-nef" },
    { "ARW",   "image/x-sony-arw" },
    { "DNG",   "image/x-adobe-dng" },
    { "RAF",   "image/x-fuji-raf" },
    { "ORF",   "image/x-olympus-orf" },
    { "RW2",   "image/x-panasonic-raw" },
    { "ZIP",   "application/zip" },
    { "CBZ",   "application/vnd.comicbook+zip" },
    { "RAR",   "application/x-rar-compressed" },
    { "CBR",   "application/vnd.comicbook-rar" },
    { "7Z",    "application/x-7z-compressed" },
    { "GZ",    "application/gzip" },
    { "PDF",   "application/pdf" },
    { "EPUB",  "application/epub+zip" },
    { "DOCX",  "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { "MP4",   "video/mp4" },
    { "MKV",   "video/x-matroska" },
    { "WEBM",  "video/webm" },
    { "FLAC",  "audio/flac" },
    { "MP3",   "audio/mpeg" },
    { "OGG",   "audio/ogg" },
    { "WAV",   "audio/wav" },
    { "EXR",   "image/x-exr" },
    { "HDR",   "image/vnd.radiance" },
    { "PSD",   "image/vnd.adobe.photoshop" },
    { "DDS",   "image/vnd.ms-dds" },
    { "SVG",   "image/svg+xml" },
    { "FONT",  "font/ttf" },
    { "GLB",   "model/gltf-binary" },
    { "GLTF",  "model/gltf+json" },
    { "STL",   "model/stl" },
    { "FITS",  "image/fits" },
    { "DICOM", "application/dicom" },
};

// Matches magic bytes in header at given offset
static bool MatchMagic(std::span<const uint8_t> header,
                       uint32_t offset,
                       std::span<const uint8_t> magic,
                       std::span<const uint8_t> mask = {}) {
    if (header.size() < offset + magic.size()) return false;
    for (size_t i = 0; i < magic.size(); ++i) {
        uint8_t byte = header[offset + i];
        if (!mask.empty()) byte &= mask[i];
        if (byte != magic[i]) return false;
    }
    return true;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// MagicBytesDatabase implementation
// ---------------------------------------------------------------------------

DetectResult MagicBytesDatabase::Detect(std::span<const uint8_t> header,
                                         std::string_view extHintLower) {
    DetectResult result;

    if (header.size() < 2) {
        // Too short — fall back to extension
        if (!extHintLower.empty()) {
            result.formatId  = ExtensionToFormatId(extHintLower);
            result.mimeType  = FormatIdToMimeType(result.formatId);
            result.confidence = 0.4f;
        }
        return result;
    }

    // ---- Try each magic pattern in priority order ----

    // JPEG
    if (MatchMagic(header, 0, kJpeg)) {
        result.formatId = "JPEG"; result.confidence = 1.0f;
    }
    // PNG
    else if (MatchMagic(header, 0, kPng)) {
        result.formatId = "PNG"; result.confidence = 1.0f;
    }
    // GIF
    else if (MatchMagic(header, 0, kGif89) || MatchMagic(header, 0, kGif87)) {
        result.formatId = "GIF"; result.confidence = 1.0f;
    }
    // WebP: RIFF....WEBP
    else if (MatchMagic(header, 0, kWebPRiff) && header.size() >= 12 &&
             MatchMagic(header, 8, kWebPWebp)) {
        result.formatId = "WEBP"; result.confidence = 1.0f;
    }
    // RIFF WAVE
    else if (MatchMagic(header, 0, kWebPRiff) && header.size() >= 12 &&
             header[8]=='W' && header[9]=='A' && header[10]=='V' && header[11]=='E') {
        result.formatId = "WAV"; result.confidence = 1.0f;
    }
    // BMP
    else if (MatchMagic(header, 0, kBmpSig)) {
        result.formatId = "BMP"; result.confidence = 1.0f;
    }
    // ICO
    else if (MatchMagic(header, 0, kIcoSig)) {
        result.formatId = "ICO"; result.confidence = 1.0f;
    }
    // TIFF LE / CR2 (distinguish by extension)
    else if (MatchMagic(header, 0, kTiffLE)) {
        // CR2 has "CR" at offset 8
        if (header.size() >= 10 && header[8]=='C' && header[9]=='R') {
            result.formatId = "CR2";
        } else {
            result.formatId = extHintLower == ".dng" ? "DNG" : "TIFF";
        }
        result.confidence = 1.0f;
    }
    // TIFF BE
    else if (MatchMagic(header, 0, kTiffBE)) {
        result.formatId = "TIFF"; result.confidence = 1.0f;
    }
    // FITS
    else if (MatchMagic(header, 0, kFits)) {
        result.formatId = "FITS"; result.confidence = 1.0f;
    }
    // OpenEXR
    else if (MatchMagic(header, 0, kExrMagic)) {
        result.formatId = "EXR"; result.confidence = 1.0f;
    }
    // Radiance HDR "#?RADIANCE"
    else if (MatchMagic(header, 0, kHdrRadiance)) {
        result.formatId = "HDR"; result.confidence = 1.0f;
    }
    // DDS
    else if (MatchMagic(header, 0, kDds)) {
        result.formatId = "DDS"; result.confidence = 1.0f;
    }
    // Photoshop
    else if (MatchMagic(header, 0, kPsd)) {
        result.formatId = "PSD"; result.confidence = 1.0f;
    }
    // JXL naked codestream
    else if (MatchMagic(header, 0, kJxl)) {
        result.formatId = "JXL"; result.confidence = 0.95f;
    }
    // JXL container
    else if (MatchMagic(header, 0, kJxlBox)) {
        result.formatId = "JXL"; result.confidence = 1.0f;
    }
    // AVIF / CR3 / MP4 / EPUB — all are ISO base media "ftyp" boxes
    else if (header.size() >= 12 && MatchMagic(header, 4, kFtyp)) {
        std::string brand(reinterpret_cast<const char*>(header.data() + 8), 4);
        if (brand == "avif" || brand == "avis") result.formatId = "AVIF";
        else if (brand == "heic" || brand == "mif1") result.formatId = "HEIC";
        else if (brand == "crx " || brand == "CRX ") result.formatId = "CR3";
        else result.formatId = "MP4";
        result.confidence = 1.0f;
    }
    // Fujifilm RAF
    else if (MatchMagic(header, 0, kRafMagic)) {
        result.formatId = "RAF"; result.confidence = 1.0f;
    }
    // Minolta MRW
    else if (MatchMagic(header, 0, kMrwMagic)) {
        result.formatId = "MRW"; result.confidence = 1.0f;
    }
    // ZIP (ZIP, DOCX, XLSX, PPTX, EPUB, CBZ)
    else if (MatchMagic(header, 0, kZip) || MatchMagic(header, 0, kZipEmpty)) {
        // Distinguish by extension
        if      (extHintLower == ".epub") result.formatId = "EPUB";
        else if (extHintLower == ".cbz")  result.formatId = "CBZ";
        else if (extHintLower == ".docx" || extHintLower == ".docm") result.formatId = "DOCX";
        else if (extHintLower == ".xlsx" || extHintLower == ".xlsm") result.formatId = "XLSX";
        else if (extHintLower == ".pptx" || extHintLower == ".pptm") result.formatId = "PPTX";
        else result.formatId = "ZIP";
        result.confidence = 0.9f;
    }
    // RAR4
    else if (MatchMagic(header, 0, kRar4)) {
        result.formatId = (extHintLower == ".cbr") ? "CBR" : "RAR";
        result.confidence = 1.0f;
    }
    // RAR5
    else if (MatchMagic(header, 0, kRar5)) {
        result.formatId = (extHintLower == ".cbr") ? "CBR" : "RAR";
        result.confidence = 1.0f;
    }
    // 7-Zip
    else if (MatchMagic(header, 0, k7z)) {
        result.formatId = (extHintLower == ".cb7") ? "CB7" : "7Z";
        result.confidence = 1.0f;
    }
    // GZip
    else if (MatchMagic(header, 0, kGzip)) {
        result.formatId = "GZ"; result.confidence = 1.0f;
    }
    // BZip2
    else if (MatchMagic(header, 0, kBzip2)) {
        result.formatId = "BZ2"; result.confidence = 1.0f;
    }
    // XZ
    else if (MatchMagic(header, 0, kXz)) {
        result.formatId = "XZ"; result.confidence = 1.0f;
    }
    // Zstandard
    else if (MatchMagic(header, 0, kZstd)) {
        result.formatId = "ZSTD"; result.confidence = 1.0f;
    }
    // tar (magic at offset 257)
    else if (header.size() > 262 && MatchMagic(header, 257, kTar)) {
        result.formatId = "TAR"; result.confidence = 1.0f;
    }
    // PDF
    else if (MatchMagic(header, 0, kPdf)) {
        result.formatId = "PDF"; result.confidence = 1.0f;
    }
    // MKV / WebM
    else if (MatchMagic(header, 0, kMkvEbml)) {
        result.formatId = "MKV"; result.confidence = 0.9f;
    }
    // FLAC
    else if (MatchMagic(header, 0, kFlac)) {
        result.formatId = "FLAC"; result.confidence = 1.0f;
    }
    // MP3 (ID3 tag)
    else if (MatchMagic(header, 0, kMp3Id3)) {
        result.formatId = "MP3"; result.confidence = 1.0f;
    }
    // MP3 sync
    else if (MatchMagic(header, 0, kMp3Sync)) {
        result.formatId = "MP3"; result.confidence = 0.8f;
    }
    // OGG
    else if (MatchMagic(header, 0, kOgg)) {
        result.formatId = "OGG"; result.confidence = 1.0f;
    }
    // TTF
    else if (MatchMagic(header, 0, kTtf)) {
        result.formatId = "FONT"; result.confidence = 0.9f;
    }
    // OTF
    else if (MatchMagic(header, 0, kOtf)) {
        result.formatId = "FONT"; result.confidence = 1.0f;
    }
    // WOFF
    else if (MatchMagic(header, 0, kWoff)) {
        result.formatId = "FONT"; result.confidence = 1.0f;
    }
    // WOFF2
    else if (MatchMagic(header, 0, kWoff2)) {
        result.formatId = "FONT"; result.confidence = 1.0f;
    }
    // GLB (glTF binary)
    else if (MatchMagic(header, 0, kGlb)) {
        result.formatId = "GLB"; result.confidence = 1.0f;
    }
    else {
        // No magic match — fall back to extension
        if (!extHintLower.empty()) {
            result.formatId  = ExtensionToFormatId(extHintLower);
            result.confidence = 0.5f;
        }
    }

    if (!result.formatId.empty()) {
        result.mimeType = FormatIdToMimeType(result.formatId);
    }
    return result;
}

std::string MagicBytesDatabase::ExtensionToFormatId(std::string_view extLower) {
    for (const auto& e : kExtMap) {
        if (e.ext == extLower) return std::string(e.formatId);
    }
    return {};
}

std::string MagicBytesDatabase::FormatIdToMimeType(std::string_view formatId) {
    for (const auto& e : kMimeMap) {
        if (e.fmtId == formatId) return std::string(e.mime);
    }
    return "application/octet-stream";
}

std::vector<std::string> MagicBytesDatabase::ExtensionsForFormat(std::string_view formatId) {
    std::vector<std::string> out;
    for (const auto& e : kExtMap) {
        if (e.formatId == formatId) out.emplace_back(e.ext);
    }
    return out;
}

size_t MagicBytesDatabase::FormatCount() {
    // Count distinct formatIds in kMimeMap
    return std::size(kMimeMap);
}

std::span<const FormatSignature> MagicBytesDatabase::Signatures() {
    // Return empty span — the runtime database is currently implemented as
    // inline code in Detect().  This accessor is reserved for future table-
    // driven implementations or third-party extenders.
    return {};
}

} // namespace Engine
} // namespace ExplorerLens
