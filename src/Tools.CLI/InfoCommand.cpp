// InfoCommand.cpp — lens info Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Detects the file format using the engine's FormatDetector and reports
// structured metadata both as human-readable text and as JSON (-j/--json).
//
#include <windows.h>
#include "InfoCommand.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <array>
#include <cstdint>

namespace fs = std::filesystem;

namespace ExplorerLens {
namespace CLI {

//==============================================================================
// Execute
//==============================================================================

int InfoCommand::Execute(const ParsedArgs& args)
{
    if (args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        std::wcout << L"Usage: " << Usage() << L"\n\n"
                   << L"Options:\n"
                   << L"  --json, -j     Output as JSON object\n";
        return static_cast<int>(ExitCode::Success);
    }

    if (args.positional.empty()) {
        std::wcerr << L"lens info: missing file path\nUsage: " << Usage() << L"\n";
        return static_cast<int>(ExitCode::InvalidArguments);
    }

    const std::wstring filePath = args.positional[0];

    if (!fs::exists(filePath)) {
        std::wcerr << L"lens info: file not found: " << filePath << L"\n";
        return static_cast<int>(ExitCode::FileNotFound);
    }

    FileInfo info = DetectFile(filePath);

    if (args.JsonOutput()) {
        PrintJson(info);
    } else {
        PrintText(info);
    }

    return static_cast<int>(ExitCode::Success);
}

//==============================================================================
// DetectFormat — lightweight extension-only format detection.
// Public API: does NOT open the file or stat the filesystem.
// Used by unit tests and the EngineTests CLI test suite.
//==============================================================================

FileInfo InfoCommand::DetectFormat(const std::wstring& path) const
{
    FileInfo info;
    info.filePath = path;

    fs::path p(path);
    std::wstring ext = p.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    // Short-name extension table — returns concise formatName values.
    struct Entry { std::wstring_view ext; std::wstring_view name; };
    static const Entry table[] = {
        { L".jpg",   L"JPEG"      }, { L".jpeg",  L"JPEG"      },
        { L".png",   L"PNG"       }, { L".gif",   L"GIF"       },
        { L".bmp",   L"BMP"       }, { L".webp",  L"WebP"      },
        { L".jxl",   L"JPEG XL"   }, { L".avif",  L"AVIF"      },
        { L".heic",  L"HEIC"      }, { L".heif",  L"HEIF"      },
        { L".pdf",   L"PDF"       }, { L".zip",   L"ZIP"       },
        { L".rar",   L"RAR"       }, { L".7z",    L"7-Zip"     },
        { L".cbz",   L"CBZ"       }, { L".cbr",   L"CBR"       },
        { L".epub",  L"EPUB"      }, { L".mp4",   L"MP4"       },
        { L".mkv",   L"MKV"       }, { L".cr2",   L"CR2 RAW"   },
        { L".cr3",   L"CR3 RAW"   }, { L".nef",   L"NEF RAW"   },
        { L".arw",   L"ARW RAW"   }, { L".dng",   L"DNG"       },
        { L".ttf",   L"TTF"       }, { L".otf",   L"OTF"       },
        { L".gltf",  L"glTF"      }, { L".glb",   L"GLB"       },
        { L".stl",   L"STL"       }, { L".exr",   L"OpenEXR"   },
        { L".hdr",   L"HDR"       }, { L".psd",   L"PSD"       },
        { L".dds",   L"DDS"       }, { L".tiff",  L"TIFF"      },
        { L".tif",   L"TIFF"      }, { L".ico",   L"ICO"       },
        { L".svg",   L"SVG"       },
    };

    for (const auto& e : table) {
        if (ext == e.ext) {
            info.formatName      = std::wstring(e.name);
            info.detectedFormat  = info.formatName;
            return info;
        }
    }

    info.formatName     = L"Unknown";
    info.detectedFormat = L"Unknown";
    return info;
}

//==============================================================================
// DetectFile — introspect a file's magic bytes to identify format/decoder
//==============================================================================

FileInfo InfoCommand::DetectFile(const std::wstring& path)
{
    FileInfo info;
    info.filePath = path;

    fs::path p(path);
    info.fileSizeBytes = fs::file_size(p);

    // Read first 16 bytes for magic-byte detection
    std::array<uint8_t, 16> magic{};
    {
        std::ifstream f(path, std::ios::binary);
        f.read(reinterpret_cast<char*>(magic.data()),
               static_cast<std::streamsize>(magic.size()));
    }

    std::wstring ext = p.extension().wstring();
    // Normalise extension to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    // ---- Format detection via magic bytes + extension ----
    // PNG
    if (magic[0]==0x89 && magic[1]=='P' && magic[2]=='N' && magic[3]=='G') {
        info.detectedFormat = L"PNG";
        info.decoderName    = L"ImageDecoder (GDI+)";
        info.mimeType       = L"image/png";
        info.hasAlpha       = true;
    }
    // JPEG
    else if (magic[0]==0xFF && magic[1]==0xD8 && magic[2]==0xFF) {
        info.detectedFormat = L"JPEG";
        info.decoderName    = L"ImageDecoder (GDI+)";
        info.mimeType       = L"image/jpeg";
    }
    // GIF
    else if (magic[0]=='G' && magic[1]=='I' && magic[2]=='F') {
        info.detectedFormat = L"GIF";
        info.decoderName    = L"ImageDecoder (GDI+)";
        info.mimeType       = L"image/gif";
        info.isAnimated     = true;
    }
    // BMP
    else if (magic[0]=='B' && magic[1]=='M') {
        info.detectedFormat = L"BMP";
        info.decoderName    = L"ImageDecoder (GDI+)";
        info.mimeType       = L"image/bmp";
    }
    // WebP
    else if (magic[0]=='R' && magic[1]=='I' && magic[2]=='F' && magic[3]=='F'
          && magic[8]=='W' && magic[9]=='E' && magic[10]=='B' && magic[11]=='P') {
        info.detectedFormat = L"WebP";
        info.decoderName    = L"ImageDecoder (libwebp)";
        info.mimeType       = L"image/webp";
        info.hasAlpha       = true;
    }
    // JPEG XL
    else if ((magic[0]==0xFF && magic[1]==0x0A)
          || (magic[0]==0x00 && magic[1]==0x00 && magic[2]==0x00 && magic[3]==0x0C
           && magic[4]=='J' && magic[5]=='X' && magic[6]=='L' && magic[7]==' ')) {
        info.detectedFormat = L"JPEG XL";
        info.decoderName    = L"JXLDecoder (libjxl)";
        info.mimeType       = L"image/jxl";
        info.hasAlpha       = true;
    }
    // PDF
    else if (magic[0]=='%' && magic[1]=='P' && magic[2]=='D' && magic[3]=='F') {
        info.detectedFormat = L"PDF";
        info.decoderName    = L"PDFDecoder (MuPDF)";
        info.mimeType       = L"application/pdf";
    }
    // 7z
    else if (magic[0]=='7' && magic[1]=='z' && magic[2]==0xBC && magic[3]==0xAF) {
        info.detectedFormat = L"7-Zip Archive";
        info.decoderName    = L"ArchiveDecoder (LZMA SDK)";
        info.mimeType       = L"application/x-7z-compressed";
    }
    // ZIP
    else if (magic[0]=='P' && magic[1]=='K') {
        info.detectedFormat = L"ZIP Archive";
        info.decoderName    = L"ArchiveDecoder (minizip-ng)";
        info.mimeType       = L"application/zip";
    }
    // RAR
    else if (magic[0]=='R' && magic[1]=='a' && magic[2]=='r' && magic[3]=='!') {
        info.detectedFormat = L"RAR Archive";
        info.decoderName    = L"ArchiveDecoder (UnRAR)";
        info.mimeType       = L"application/x-rar-compressed";
    }
    // DDS (DirectX Texture)
    else if (magic[0]=='D' && magic[1]=='D' && magic[2]=='S' && magic[3]==' ') {
        info.detectedFormat = L"DDS (DirectX Texture)";
        info.decoderName    = L"DDSDecoder";
        info.mimeType       = L"image/vnd.ms-dds";
    }
    // Fallback — extension-based
    else {
        struct ExtEntry { std::wstring_view ext; std::wstring_view fmt; std::wstring_view dec; std::wstring_view mime; };
        static const ExtEntry s_table[] = {
            { L".heic",  L"HEIC/HEIF",       L"HEIFDecoder (libheif)",   L"image/heic"          },
            { L".heif",  L"HEIF",             L"HEIFDecoder (libheif)",   L"image/heif"          },
            { L".avif",  L"AVIF",             L"AVIFDecoder (libavif)",   L"image/avif"          },
            { L".cr2",   L"Canon RAW CR2",    L"RawDecoder (LibRaw)",     L"image/x-canon-cr2"   },
            { L".cr3",   L"Canon RAW CR3",    L"RawDecoder (LibRaw)",     L"image/x-canon-cr3"   },
            { L".nef",   L"Nikon RAW NEF",    L"RawDecoder (LibRaw)",     L"image/x-nikon-nef"   },
            { L".arw",   L"Sony RAW ARW",     L"RawDecoder (LibRaw)",     L"image/x-sony-arw"    },
            { L".dng",   L"Adobe DNG",        L"RawDecoder (LibRaw)",     L"image/x-adobe-dng"   },
            { L".cbz",   L"Comic Book ZIP",   L"ArchiveDecoder (CBZ)",    L"application/x-cbz"   },
            { L".cbr",   L"Comic Book RAR",   L"ArchiveDecoder (CBR)",    L"application/x-cbr"   },
            { L".epub",  L"EPUB E-Book",      L"EBookDecoder",            L"application/epub+zip"},
            { L".mp4",   L"MP4 Video",        L"VideoDecoder (MF)",       L"video/mp4"           },
            { L".mkv",   L"Matroska Video",   L"VideoDecoder (MF)",       L"video/x-matroska"    },
            { L".mp3",   L"MP3 Audio",        L"AudioDecoder (MF)",       L"audio/mpeg"          },
            { L".flac",  L"FLAC Audio",       L"AudioDecoder (MF)",       L"audio/flac"          },
            { L".ttf",   L"TrueType Font",    L"FontDecoder",             L"font/ttf"            },
            { L".otf",   L"OpenType Font",    L"FontDecoder",             L"font/otf"            },
            { L".gltf",  L"glTF 3D Model",    L"ModelDecoder (glTF)",     L"model/gltf+json"     },
            { L".glb",   L"GLB 3D Model",     L"ModelDecoder (glTF)",     L"model/gltf-binary"   },
            { L".stl",   L"STL 3D Model",     L"ModelDecoder (STL)",      L"model/stl"           },
            { L".exr",   L"OpenEXR HDR",      L"EXRDecoder",              L"image/x-exr"         },
            { L".hdr",   L"Radiance HDR",     L"HDRDecoder",              L"image/vnd.radiance"  },
            { L".psd",   L"Photoshop PSD",    L"ImageDecoder (GDI+)",     L"image/vnd.adobe.photoshop"},
        };

        bool matched = false;
        for (const auto& e : s_table) {
            if (ext == e.ext) {
                info.detectedFormat = std::wstring(e.fmt);
                info.decoderName    = std::wstring(e.dec);
                info.mimeType       = std::wstring(e.mime);
                matched = true;
                break;
            }
        }
        if (!matched) {
            info.detectedFormat = L"Unknown";
            info.decoderName    = L"None";
            info.mimeType       = L"application/octet-stream";
        }
    }

    // Keep formatName in sync with detectedFormat for unified API consumers.
    info.formatName = info.detectedFormat;
    return info;
}

//==============================================================================
// PrintText — human-readable one-pager
//==============================================================================

void InfoCommand::PrintText(const FileInfo& info) const
{
    auto hr = [](std::wstring_view label, std::wstring_view value) {
        std::wcout << L"  " << std::left << std::setw(18) << label << L": " << value << L"\n";
    };

    std::wcout << L"\nFile: " << info.filePath << L"\n"
               << L"  " << std::wstring(50, L'-') << L"\n";
    hr(L"Format",  info.detectedFormat);
    hr(L"Decoder", info.decoderName);
    hr(L"MIME",    info.mimeType);

    std::wostringstream sz;
    sz << info.fileSizeBytes << L" bytes ("
       << (info.fileSizeBytes / 1024) << L" KB)";
    hr(L"File size", sz.str());

    if (info.widthPx > 0)
        hr(L"Dimensions", std::to_wstring(info.widthPx) + L" x " + std::to_wstring(info.heightPx));
    if (info.bitDepth > 0)
        hr(L"Bit depth", std::to_wstring(info.bitDepth) + L"-bit");
    if (!info.colorSpace.empty())
        hr(L"Color space", info.colorSpace);
    if (info.hasAlpha)
        hr(L"Alpha channel", L"yes");
    if (info.isAnimated)
        hr(L"Animated", L"yes");
    if (!info.extraMetadata.empty())
        hr(L"Extra", info.extraMetadata);

    std::wcout << L"\n";
}

//==============================================================================
// PrintJson — machine-readable JSON object
//==============================================================================

void InfoCommand::PrintJson(const FileInfo& info) const
{
    auto esc = [](const std::wstring& s) -> std::wstring {
        std::wstring out;
        for (wchar_t c : s) {
            if (c == L'"')  out += L"\\\"";
            else if (c == L'\\') out += L"\\\\";
            else out += c;
        }
        return out;
    };

    std::wcout << L"{\n"
               << L"  \"file\": \""       << esc(info.filePath)      << L"\",\n"
               << L"  \"format\": \""     << esc(info.detectedFormat) << L"\",\n"
               << L"  \"decoder\": \""    << esc(info.decoderName)    << L"\",\n"
               << L"  \"mime\": \""       << esc(info.mimeType)       << L"\",\n"
               << L"  \"fileSize\": "     << info.fileSizeBytes       << L",\n"
               << L"  \"width\": "        << info.widthPx             << L",\n"
               << L"  \"height\": "       << info.heightPx            << L",\n"
               << L"  \"bitDepth\": "     << info.bitDepth            << L",\n"
               << L"  \"hasAlpha\": "     << (info.hasAlpha ? L"true" : L"false") << L",\n"
               << L"  \"isAnimated\": "   << (info.isAnimated ? L"true" : L"false") << L"\n"
               << L"}\n";
}

} // namespace CLI
} // namespace ExplorerLens
