// DecoderHealthCheck.h - Decoder availability and health status
// ExplorerLens v6.2.0
// Reports which decoders are available and their library dependencies

#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace ExplorerLens {

struct DecoderHealthInfo {
    std::wstring name;           // e.g., "WebP", "HEIF", "JXL"
    std::wstring version;        // Library version or "built-in"
    std::wstring dllName;        // Required DLL name (empty if none)
    bool isAvailable;            // Whether the decoder can function
    bool hasExternalDependency;  // Requires external library
    uint32_t extensionCount;     // Number of supported extensions
    std::wstring statusMessage;  // Human-readable status
};

class DecoderHealthCheck {
public:
    /// Run health check on all decoders
    static std::vector<DecoderHealthInfo> CheckAll() {
        std::vector<DecoderHealthInfo> results;

        // Built-in decoders (no external dependencies)
        results.push_back(MakeBuiltIn(L"Image (WIC)", L"1.0", 10,
            L"JPEG, PNG, BMP, GIF, TIFF, ICO via Windows Imaging Component"));
        results.push_back(MakeBuiltIn(L"Archive", L"1.0", 12,
            L"CBZ, CBR, CB7, CBT, ZIP, RAR, 7Z, TAR, EPUB, PHZ"));
        results.push_back(MakeBuiltIn(L"PSD", L"1.0", 2,
            L"Adobe Photoshop PSD/PSB - native parser"));
        results.push_back(MakeBuiltIn(L"DDS", L"1.0", 1,
            L"DirectDraw Surface - native parser"));
        results.push_back(MakeBuiltIn(L"HDR", L"1.0", 1,
            L"Radiance RGBE HDR - native parser with SSE tone mapping"));
        results.push_back(MakeBuiltIn(L"PPM", L"1.0", 6,
            L"NetPBM PPM/PGM/PBM/PNM/PAM/PFM - native parser"));
        results.push_back(MakeBuiltIn(L"EXR", L"1.0", 1,
            L"OpenEXR - native parser (uncompressed)"));
        results.push_back(MakeBuiltIn(L"TGA", L"1.0", 1,
            L"Targa TGA - native parser"));
        results.push_back(MakeBuiltIn(L"QOI", L"1.0", 1,
            L"Quite OK Image - native parser"));
        results.push_back(MakeBuiltIn(L"ICO", L"1.0", 2,
            L"Windows Icon/Cursor - native parser"));
        results.push_back(MakeBuiltIn(L"SVG", L"1.0", 2,
            L"SVG/SVGZ - placeholder with dimensions"));
        results.push_back(MakeBuiltIn(L"Video", L"1.0", 20,
            L"MP4/MKV/AVI/MOV/WMV via Media Foundation"));
        results.push_back(MakeBuiltIn(L"Audio", L"1.0", 14,
            L"MP3/FLAC/OGG/WAV - album art extraction"));
        results.push_back(MakeBuiltIn(L"PDF", L"1.0", 1,
            L"PDF - Shell thumbnail provider"));
        results.push_back(MakeBuiltIn(L"Document", L"1.0", 19,
            L"EPUB/DOCX/XLS/PPT - Shell thumbnail provider"));
        results.push_back(MakeBuiltIn(L"Font", L"1.0", 7,
            L"TTF/OTF/WOFF - Shell font preview provider"));

        // Decoders with external library dependencies
        results.push_back(CheckLibrary(L"WebP", L"libwebp.dll", 1,
            L"Google WebP image format"));
        results.push_back(CheckLibrary(L"AVIF", L"libavif.dll", 1,
            L"AV1 Image File Format"));
        results.push_back(CheckLibrary(L"JXL", L"jxl.dll", 1,
            L"JPEG XL next-generation image format"));
        results.push_back(CheckLibrary(L"HEIF", L"libheif.dll", 8,
            L"HEIF/HEIC (iPhone photos)"));
        results.push_back(CheckLibrary(L"RAW", L"libraw.dll", 25,
            L"Camera RAW (Canon CR2/CR3, Nikon NEF, Sony ARW, DNG...)"));

        return results;
    }

    /// Get a summary string for display
    static std::wstring GetSummary() {
        auto results = CheckAll();
        std::wstring summary;
        int available = 0, total = 0, totalExts = 0;

        for (const auto& info : results) {
            total++;
            if (info.isAvailable) {
                available++;
                totalExts += info.extensionCount;
            }
        }

        summary += L"Decoder Health Check\r\n";
        summary += L"====================\r\n\r\n";
        summary += L"Active Decoders: " + std::to_wstring(available) +
                   L" / " + std::to_wstring(total) + L"\r\n";
        summary += L"Total Extensions: " + std::to_wstring(totalExts) + L"\r\n\r\n";

        for (const auto& info : results) {
            summary += info.isAvailable ? L"[OK] " : L"[--] ";
            summary += info.name + L" (" + std::to_wstring(info.extensionCount) + L" ext)";
            if (!info.isAvailable && info.hasExternalDependency) {
                summary += L" - Missing: " + info.dllName;
            }
            summary += L"\r\n";
            summary += L"     " + info.statusMessage + L"\r\n";
        }

        return summary;
    }

private:
    static DecoderHealthInfo MakeBuiltIn(const std::wstring& name,
        const std::wstring& version, uint32_t extCount,
        const std::wstring& description) {
        DecoderHealthInfo info;
        info.name = name;
        info.version = version;
        info.isAvailable = true;
        info.hasExternalDependency = false;
        info.extensionCount = extCount;
        info.statusMessage = description;
        return info;
    }

    static DecoderHealthInfo CheckLibrary(const std::wstring& name,
        const std::wstring& dllName, uint32_t extCount,
        const std::wstring& description) {
        DecoderHealthInfo info;
        info.name = name;
        info.dllName = dllName;
        info.extensionCount = extCount;
        info.hasExternalDependency = true;

        // Try to load the DLL
        HMODULE hMod = LoadLibraryExW(dllName.c_str(), nullptr,
            LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (hMod) {
            info.isAvailable = true;
            info.statusMessage = description + L" - Library found";
            info.version = L"detected";
            FreeLibrary(hMod);
        } else {
            info.isAvailable = false;
            info.statusMessage = description + L" - Library not found (" + dllName + L")";
            info.version = L"N/A";
        }

        return info;
    }
};

} // namespace ExplorerLens

