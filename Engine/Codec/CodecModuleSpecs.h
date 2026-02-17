//==============================================================================
// DarkThumbs Engine — Per-Format Codec DLL Specifications
// Sprint 36+: Execution Optimization — Per-Format DLL Architecture
// Copyright (c) 2026 — DarkThumbs Project
//
// PURPOSE:
//   Define the concrete codec DLL modules that implement ICodecModule.h.
//   Each DLL is a self-contained decoder that links only against its required
//   libraries, keeping idle memory near zero for unused formats.
//
// ARCHITECTURE:
//   ┌─────────────────────────────────────────────────────────────────┐
//   │  CBXShell.dll  (COM shell extension — always loaded)           │
//   │   ├─ WIC ImageDecoder (JPEG/PNG/BMP/GIF/TIFF) — in-process    │
//   │   ├─ ICODecoder, DDSDecoder, EXRDecoder       — in-process    │
//   │   └─ CodecLoader (demand-loads codec DLLs below)               │
//   ├─────────────────────────────────────────────────────────────────┤
//   │  DarkThumbs_Codec_WebP.dll    (~4 MB)  — libwebp 1.5.0        │
//   │  DarkThumbs_Codec_HEIF.dll    (~8 MB)  — libheif + libde265   │
//   │  DarkThumbs_Codec_JXL.dll     (~12 MB) — libjxl + brotli + hwy│
//   │  DarkThumbs_Codec_RAW.dll     (~18 MB) — LibRaw 0.21.3        │
//   │  DarkThumbs_Codec_AVIF.dll    (~6 MB)  — libavif + dav1d      │
//   │  DarkThumbs_Codec_Archive.dll (~5 MB)  — minizip + zstd + lz4 │
//   │  DarkThumbs_Codec_SVG.dll     (~1 MB)  — GDI+ + zlib          │
//   │  DarkThumbs_Codec_HDR.dll     (~0.5MB) — custom (SSE)         │
//   │  DarkThumbs_Codec_PSD.dll     (~0.5MB) — custom parser        │
//   │  DarkThumbs_Codec_Video.dll   (~2 MB)  — Media Foundation     │
//   │  DarkThumbs_Codec_Audio.dll   (~1 MB)  — ID3v2/FLAC parser    │
//   │  DarkThumbs_Codec_Document.dll(~1 MB)  — ZIP + Shell fallback │
//   │  DarkThumbs_Codec_Font.dll    (~1 MB)  — DirectWrite          │
//   │  DarkThumbs_Codec_Model.dll   (~3 MB)  — Direct3D 11          │
//   │  DarkThumbs_Codec_QOI.dll     (~0.2MB) — custom (qoi.h)       │
//   │  DarkThumbs_Codec_TGA.dll     (~0.3MB) — custom (RLE)         │
//   │  DarkThumbs_Codec_PPM.dll     (~0.2MB) — custom parser        │
//   └─────────────────────────────────────────────────────────────────┘
//
// MEMORY SAVINGS EXAMPLE:
//   Scenario: User browses a folder containing only 500 JPEG files.
//   Before (monolithic):  All decoders statically linked → ~65 MB working set
//   After  (modular):     Only WIC path (in-process)     → ~12 MB working set
//                          Savings: ~53 MB (81% reduction)
//
//   Scenario: Mixed folder with JPEGs + HEIFs + WebPs
//   Before (monolithic):  All decoders → ~65 MB
//   After  (modular):     WIC + HEIF + WebP DLLs → ~24 MB
//                          Savings: ~41 MB (63% reduction)
//==============================================================================

#pragma once

#include "ICodecModule.h"
#include <string>
#include <vector>
#include <cstring>

namespace DarkThumbs {
namespace Engine {
namespace Codec {

//==============================================================================
// Codec Module Specification — describes one codec DLL's contents
//==============================================================================
struct CodecModuleSpec
{
    const char*     codecId;            ///< e.g. "darkthumbs.codec.webp"
    const wchar_t*  dllName;            ///< e.g. L"DarkThumbs_Codec_WebP.dll"
    const wchar_t*  displayName;        ///< e.g. L"WebP Codec"
    const wchar_t*  version;            ///< e.g. L"1.5.0 (libwebp)"
    uint64_t        estimatedMemoryMB;  ///< Working-set when loaded
    int             priority;           ///< Lower = higher priority 
    uint32_t        capabilities;       ///< DtCodecCaps bitmask
    bool            requiresExternalLib;///< true if links against 3rd-party lib

    /// Extensions this codec handles (with dots, lowercase)
    std::vector<std::wstring> extensions;

    /// External library dependencies needed on PATH / in codec dir
    std::vector<std::wstring> dependencies;
};

//==============================================================================
// Complete Codec Module Registry — all 17 codec DLLs
//==============================================================================
inline std::vector<CodecModuleSpec> GetAllCodecSpecs()
{
    std::vector<CodecModuleSpec> specs;
    specs.reserve(17);

    //--- 1) WebP Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.webp";
        s.dllName = L"DarkThumbs_Codec_WebP.dll";
        s.displayName = L"WebP Codec";
        s.version = L"1.5.0 (libwebp)";
        s.estimatedMemoryMB = 4;
        s.priority = 10;
        s.capabilities = DT_CAP_ANIMATION | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = { L".webp" };
        s.dependencies = { L"libwebp.dll", L"libsharpyuv.dll" };
        specs.push_back(std::move(s));
    }

    //--- 2) HEIF Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.heif";
        s.dllName = L"DarkThumbs_Codec_HEIF.dll";
        s.displayName = L"HEIF/HEIC Codec";
        s.version = L"1.19.5 (libheif + libde265)";
        s.estimatedMemoryMB = 8;
        s.priority = 10;
        s.capabilities = DT_CAP_METADATA | DT_CAP_ICC_PROFILE | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = { L".heif", L".heic", L".hif", L".avci", L".heifs", L".heics" };
        s.dependencies = { L"libheif.dll", L"libde265.dll" };
        specs.push_back(std::move(s));
    }

    //--- 3) JPEG XL Codec ----------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.jxl";
        s.dllName = L"DarkThumbs_Codec_JXL.dll";
        s.displayName = L"JPEG XL Codec";
        s.version = L"0.11.1 (libjxl)";
        s.estimatedMemoryMB = 12;
        s.priority = 10;
        s.capabilities = DT_CAP_ANIMATION | DT_CAP_HDR | DT_CAP_ICC_PROFILE | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = { L".jxl" };
        s.dependencies = { L"jxl.dll", L"jxl_threads.dll", L"brotlicommon.dll",
                           L"brotlidec.dll", L"hwy.dll" };
        specs.push_back(std::move(s));
    }

    //--- 4) RAW Codec ---------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.raw";
        s.dllName = L"DarkThumbs_Codec_RAW.dll";
        s.displayName = L"Camera RAW Codec";
        s.version = L"0.21.3 (LibRaw)";
        s.estimatedMemoryMB = 18;
        s.priority = 20;
        s.capabilities = DT_CAP_METADATA | DT_CAP_ICC_PROFILE | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = {
            L".cr2", L".cr3", L".nef", L".nrw", L".arw", L".srf", L".sr2",
            L".dng", L".orf", L".rw2", L".pef", L".raf", L".x3f", L".3fr",
            L".dcr", L".kdc", L".mrw", L".erf", L".mef", L".mos", L".iiq",
            L".rwl", L".srw"
        };
        s.dependencies = { L"libraw.dll" };
        specs.push_back(std::move(s));
    }

    //--- 5) AVIF Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.avif";
        s.dllName = L"DarkThumbs_Codec_AVIF.dll";
        s.displayName = L"AVIF Codec";
        s.version = L"1.3.0 (libavif + dav1d)";
        s.estimatedMemoryMB = 6;
        s.priority = 10;
        s.capabilities = DT_CAP_HDR | DT_CAP_ICC_PROFILE | DT_CAP_ANIMATION | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = { L".avif" };
        s.dependencies = { L"avif.dll", L"dav1d.dll" };
        specs.push_back(std::move(s));
    }

    //--- 6) Archive Codec -----------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.archive";
        s.dllName = L"DarkThumbs_Codec_Archive.dll";
        s.displayName = L"Archive Codec";
        s.version = L"4.0.10 (minizip-ng + zstd + lz4 + LZMA)";
        s.estimatedMemoryMB = 5;
        s.priority = 5;
        s.capabilities = DT_CAP_ARCHIVE | DT_CAP_MULTI_PAGE | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = true;
        s.extensions = {
            L".zip", L".cbz", L".rar", L".cbr", L".7z", L".cb7",
            L".tar", L".cbt", L".gz", L".bz2", L".xz", L".zst"
        };
        s.dependencies = { L"minizip.dll", L"zstd.dll", L"lz4.dll", L"lzma.dll" };
        specs.push_back(std::move(s));
    }

    //--- 7) SVG Codec ---------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.svg";
        s.dllName = L"DarkThumbs_Codec_SVG.dll";
        s.displayName = L"SVG Vector Codec";
        s.version = L"1.0.0 (GDI+ + zlib)";
        s.estimatedMemoryMB = 1;
        s.priority = 30;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".svg", L".svgz" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 8) HDR Codec ---------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.hdr";
        s.dllName = L"DarkThumbs_Codec_HDR.dll";
        s.displayName = L"Radiance HDR Codec";
        s.version = L"1.0.0 (custom RGBE + SSE)";
        s.estimatedMemoryMB = 0;  // < 0.5 MB
        s.priority = 40;
        s.capabilities = DT_CAP_HDR | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".hdr" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 9) PSD Codec ---------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.psd";
        s.dllName = L"DarkThumbs_Codec_PSD.dll";
        s.displayName = L"Photoshop Codec";
        s.version = L"1.0.0 (custom parser)";
        s.estimatedMemoryMB = 0;
        s.priority = 40;
        s.capabilities = DT_CAP_METADATA | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".psd", L".psb" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 10) Video Codec ------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.video";
        s.dllName = L"DarkThumbs_Codec_Video.dll";
        s.displayName = L"Video Thumbnail Codec";
        s.version = L"1.0.0 (Media Foundation)";
        s.estimatedMemoryMB = 2;
        s.priority = 50;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = {
            L".mp4", L".mkv", L".avi", L".wmv", L".mov", L".flv",
            L".webm", L".m4v", L".mpg", L".mpeg", L".3gp", L".ts",
            L".m2ts", L".vob", L".ogv", L".rm", L".rmvb", L".asf"
        };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 11) Audio Codec ------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.audio";
        s.dllName = L"DarkThumbs_Codec_Audio.dll";
        s.displayName = L"Audio Album Art Codec";
        s.version = L"1.0.0 (ID3v2/FLAC parser)";
        s.estimatedMemoryMB = 1;
        s.priority = 60;
        s.capabilities = DT_CAP_METADATA | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = {
            L".mp3", L".flac", L".wma", L".aac", L".m4a",
            L".ogg", L".opus", L".ape", L".wv", L".mpc"
        };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 12) Document Codec ---------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.document";
        s.dllName = L"DarkThumbs_Codec_Document.dll";
        s.displayName = L"Document Thumbnail Codec";
        s.version = L"1.0.0 (ZIP + Shell)";
        s.estimatedMemoryMB = 1;
        s.priority = 70;
        s.capabilities = DT_CAP_MULTI_PAGE | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = {
            L".pdf", L".epub", L".mobi", L".azw", L".azw3",
            L".docx", L".pptx", L".xlsx", L".odt", L".odp"
        };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 13) Font Codec -------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.font";
        s.dllName = L"DarkThumbs_Codec_Font.dll";
        s.displayName = L"Font Preview Codec";
        s.version = L"1.0.0 (DirectWrite)";
        s.estimatedMemoryMB = 1;
        s.priority = 70;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".ttf", L".otf", L".woff", L".woff2", L".ttc" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 14) 3D Model Codec ---------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.model";
        s.dllName = L"DarkThumbs_Codec_Model.dll";
        s.displayName = L"3D Model Codec";
        s.version = L"1.0.0 (Direct3D 11)";
        s.estimatedMemoryMB = 3;
        s.priority = 70;
        s.capabilities = DT_CAP_GPU_ACCEL | DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".obj", L".stl", L".gltf", L".glb", L".fbx", L".3ds" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 15) QOI Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.qoi";
        s.dllName = L"DarkThumbs_Codec_QOI.dll";
        s.displayName = L"QOI Codec";
        s.version = L"1.0.0 (reference impl)";
        s.estimatedMemoryMB = 0;
        s.priority = 30;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".qoi" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 16) TGA Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.tga";
        s.dllName = L"DarkThumbs_Codec_TGA.dll";
        s.displayName = L"TGA Codec";
        s.version = L"1.0.0 (custom RLE)";
        s.estimatedMemoryMB = 0;
        s.priority = 30;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".tga", L".tpic" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    //--- 17) PPM Codec --------------------------------------------------------
    {
        CodecModuleSpec s{};
        s.codecId = "darkthumbs.codec.ppm";
        s.dllName = L"DarkThumbs_Codec_PPM.dll";
        s.displayName = L"Netpbm Codec";
        s.version = L"1.0.0 (custom parser)";
        s.estimatedMemoryMB = 0;
        s.priority = 30;
        s.capabilities = DT_CAP_THREAD_SAFE;
        s.requiresExternalLib = false;
        s.extensions = { L".ppm", L".pgm", L".pbm", L".pnm", L".pam", L".pfm" };
        s.dependencies = {};
        specs.push_back(std::move(s));
    }

    return specs;
}

//==============================================================================
// Codec Manifest Generator — creates the JSON sidecar for the CodecLoader
//==============================================================================
inline std::string GenerateCodecManifest()
{
    auto specs = GetAllCodecSpecs();
    std::string json = "{\n  \"version\": 1,\n  \"codecs\": [\n";

    for (size_t i = 0; i < specs.size(); i++) {
        auto& s = specs[i];
        json += "    {\n";
        json += "      \"id\": \"" + std::string(s.codecId) + "\",\n";

        // Convert wchar_t to narrow for JSON
        std::string dll(s.dllName, s.dllName + wcslen(s.dllName));
        json += "      \"dll\": \"" + dll + "\",\n";

        json += "      \"extensions\": [";
        for (size_t j = 0; j < s.extensions.size(); j++) {
            std::string ext(s.extensions[j].begin(), s.extensions[j].end());
            json += "\"" + ext + "\"";
            if (j + 1 < s.extensions.size()) json += ", ";
        }
        json += "],\n";

        json += "      \"estimatedMemoryMB\": " + std::to_string(s.estimatedMemoryMB) + ",\n";
        json += "      \"priority\": " + std::to_string(s.priority) + ",\n";
        json += "      \"capabilities\": " + std::to_string(s.capabilities) + ",\n";

        json += "      \"dependencies\": [";
        for (size_t j = 0; j < s.dependencies.size(); j++) {
            std::string dep(s.dependencies[j].begin(), s.dependencies[j].end());
            json += "\"" + dep + "\"";
            if (j + 1 < s.dependencies.size()) json += ", ";
        }
        json += "]\n";

        json += "    }";
        if (i + 1 < specs.size()) json += ",";
        json += "\n";
    }

    json += "  ]\n}\n";
    return json;
}

//==============================================================================
// Memory Impact Analysis — calculates savings for a given set of extensions
//==============================================================================
struct MemoryImpactReport
{
    uint64_t    monolithicMemoryMB;       ///< All decoders statically linked
    uint64_t    modularMemoryMB;          ///< Only needed codec DLLs loaded
    uint64_t    savingsMB;                ///< Difference
    double      savingsPercent;           ///< (savings / monolithic) * 100
    uint32_t    codecsLoaded;             ///< Number of codec DLLs loaded
    uint32_t    codecsAvoided;            ///< Number of codec DLLs NOT loaded
    std::vector<std::string> loadedIds;   ///< Codec IDs that would be loaded
    std::vector<std::string> avoidedIds;  ///< Codec IDs that stay unloaded
};

inline MemoryImpactReport AnalyzeMemoryImpact(
    const std::vector<std::wstring>& activeExtensions)
{
    auto specs = GetAllCodecSpecs();
    MemoryImpactReport report{};

    // Monolithic: sum of ALL codec memory estimates + 12 MB base (WIC in-process)
    uint64_t totalMemory = 0;
    for (auto& s : specs) {
        totalMemory += s.estimatedMemoryMB;
    }
    report.monolithicMemoryMB = totalMemory + 12;  // +12 MB for WIC base

    // Modular: find which codecs are needed for the active extensions
    std::unordered_set<std::string> neededCodecs;

    for (auto& ext : activeExtensions) {
        std::wstring lower = ext;
        for (auto& c : lower) {
            if (c >= L'A' && c <= L'Z') c += 32;
        }

        for (auto& s : specs) {
            for (auto& codecExt : s.extensions) {
                if (codecExt == lower) {
                    neededCodecs.insert(s.codecId);
                    break;
                }
            }
        }
    }

    uint64_t modularMemory = 12; // WIC base always loaded
    for (auto& s : specs) {
        if (neededCodecs.count(s.codecId)) {
            modularMemory += s.estimatedMemoryMB;
            report.loadedIds.push_back(s.codecId);
        } else {
            report.avoidedIds.push_back(s.codecId);
        }
    }

    report.modularMemoryMB = modularMemory;
    report.savingsMB = report.monolithicMemoryMB - report.modularMemoryMB;
    report.savingsPercent = (report.monolithicMemoryMB > 0)
        ? (static_cast<double>(report.savingsMB) / report.monolithicMemoryMB) * 100.0
        : 0.0;
    report.codecsLoaded = static_cast<uint32_t>(report.loadedIds.size());
    report.codecsAvoided = static_cast<uint32_t>(report.avoidedIds.size());

    return report;
}

} // namespace Codec
} // namespace Engine
} // namespace DarkThumbs
