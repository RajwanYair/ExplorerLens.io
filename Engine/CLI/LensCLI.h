// LensCLI.h — Command-Line Thumbnail Generation Tool Core
// Copyright (c) 2026 ExplorerLens Project
//
// Core driver for lens.exe: routes CLI subcommands (generate, batch, cache,
// info, formats) to the appropriate engine operations.
//
#pragma once
#include <windows.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// Opaque SDK handle types for the lens.exe C API.
// Full definitions live in SDK/plugin_api.h at runtime.
using LENS_ENGINE_HANDLE = void*;
using LENS_THUMBNAIL_HANDLE = void*;

namespace ExplorerLens {
namespace Engine {

// Exit codes for lens.exe
enum class CLIExitCode : int {
    Success = 0,
    GeneralError = 1,
    InvalidArgs = 2,
    FormatUnknown = 3,
    LicenseError = 4,
    IOError = 5,
    PartialFailure = 6,  // Batch: some files failed
};

// Subcommand identifiers
enum class CLICommand {
    None,
    Generate,  // Single thumbnail → file
    Batch,     // Directory or list → output dir
    Cache,     // Cache management (clear, stats, warm)
    Info,      // File metadata / format info
    Formats,   // List supported extensions
    Version,   // Print version + exit
    Help,      // Print usage + exit
};

// Parsed options for the 'generate' subcommand
struct GenerateOptions
{
    std::wstring inputPath;
    std::wstring outputPath;  // If empty: <input>_thumb.png
    uint32_t width = 256;
    uint32_t height = 256;
    bool highQuality = false;
    bool forceCPU = false;
    uint32_t timeoutMs = 15000;
};

// Parsed options for the 'batch' subcommand
struct BatchOptions
{
    std::wstring inputDir;
    std::wstring outputDir;
    std::wstring filter;  // Glob or extension e.g. "*.psd"
    uint32_t width = 256;
    uint32_t height = 256;
    uint32_t threads = 0;  // 0 = auto
    bool recursive = false;
    bool skipExisting = true;
    bool progressBar = true;
};

// Progress callback for batch operations: (done, total, currentFile)
using ProgressCallback = std::function<void(uint32_t, uint32_t, const std::wstring&)>;

class LensCLI
{
  public:
    explicit LensCLI(LENS_ENGINE_HANDLE hEngine) : m_hEngine(hEngine) {}

    // Entry point — call from wmain() with argc/argv
    CLIExitCode Run(int argc, wchar_t* argv[]);

    // Individual subcommand handlers
    CLIExitCode RunGenerate(const GenerateOptions& opts);
    CLIExitCode RunBatch(const BatchOptions& opts, ProgressCallback cb = nullptr);
    CLIExitCode RunCacheInfo();
    CLIExitCode RunCacheClear();
    CLIExitCode RunFileInfo(const std::wstring& path);
    CLIExitCode RunListFormats();

    // ANSI color output helpers
    static void PrintSuccess(const wchar_t* msg);
    static void PrintError(const wchar_t* msg);
    static void PrintWarning(const wchar_t* msg);
    static void PrintInfo(const wchar_t* msg);
    static void PrintProgressBar(uint32_t done, uint32_t total, uint32_t width = 40);

  private:
    LENS_ENGINE_HANDLE m_hEngine;

    static bool SaveBitmapToPNG(LENS_THUMBNAIL_HANDLE hThumb, const std::wstring& outPath);
    static std::wstring DefaultOutputPath(const std::wstring& inputPath);

    static void PrintUsage()
    {
        wprintf(L"\n");
        wprintf(L"  \033[1mExplorerLens CLI\033[0m — GPU-accelerated thumbnail generator\n");
        wprintf(L"  Version 39.8.0 (Betelgeuse)\n");
        wprintf(L"\n");
        wprintf(L"  \033[1mUSAGE\033[0m\n");
        wprintf(L"    lens <command> [options]\n");
        wprintf(L"\n");
        wprintf(L"  \033[1mCOMMANDS\033[0m\n");
        wprintf(L"    generate  <file>           Create a thumbnail from any supported file\n");
        wprintf(L"    batch     <dir>            Batch-generate thumbnails for an entire folder\n");
        wprintf(L"    info      <file>           Show file metadata, detected format, and decoder\n");
        wprintf(L"    formats   [--category]     List all 200+ supported file extensions\n");
        wprintf(L"    cache     [clear|stats]    View cache statistics or clear stale entries\n");
        wprintf(L"    version                    Print version, build info, and GPU backend\n");
        wprintf(L"    help      [command]        Show detailed help for any command\n");
        wprintf(L"\n");
        wprintf(L"  \033[1mQUICK START\033[0m\n");
        wprintf(L"    \033[36m$\033[0m lens generate photo.cr2\n");
        wprintf(L"      → photo_thumb.png (256×256, GPU-accelerated)\n");
        wprintf(L"\n");
        wprintf(L"    \033[36m$\033[0m lens generate design.psd --size 512 --quality\n");
        wprintf(L"      → design_thumb.png (512×512, high-quality Lanczos resize)\n");
        wprintf(L"\n");
        wprintf(L"    \033[36m$\033[0m lens batch ./photos --recursive --threads 8\n");
        wprintf(L"      → ./photos/thumbs/*.png (all supported files, 8 threads)\n");
        wprintf(L"\n");
        wprintf(L"    \033[36m$\033[0m lens formats --category images\n");
        wprintf(L"      → .jpg .png .webp .avif .jxl .heic .tiff .bmp .gif ...\n");
        wprintf(L"\n");
        wprintf(L"    \033[36m$\033[0m lens info document.pdf\n");
        wprintf(L"      → Format: PDF | Decoder: PDFDecoder (MuPDF) | Pages: 42\n");
        wprintf(L"\n");
        wprintf(L"  \033[1mEXIT CODES\033[0m\n");
        wprintf(L"    0  Success           3  Unknown format     6  Partial failure (batch)\n");
        wprintf(L"    1  General error     4  License error\n");
        wprintf(L"    2  Invalid args      5  I/O error\n");
        wprintf(L"\n");
        wprintf(L"  Run \033[1mlens help <command>\033[0m for detailed options and examples.\n");
        wprintf(L"\n");
    }

    // ANSI escape guards
    static bool s_ansiEnabled;
    static bool EnableAnsiIfPossible();
};

bool LensCLI::s_ansiEnabled = LensCLI::EnableAnsiIfPossible();

inline bool LensCLI::EnableAnsiIfPossible()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode))
        return false;
    return SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}

inline void LensCLI::PrintSuccess(const wchar_t* msg)
{
    if (s_ansiEnabled)
        wprintf(L"\033[32m✔ %s\033[0m\n", msg);
    else
        wprintf(L"[OK] %s\n", msg);
}
inline void LensCLI::PrintError(const wchar_t* msg)
{
    if (s_ansiEnabled)
        fwprintf(stderr, L"\033[31m✖ %s\033[0m\n", msg);
    else
        fwprintf(stderr, L"[ERR] %s\n", msg);
}
inline void LensCLI::PrintWarning(const wchar_t* msg)
{
    if (s_ansiEnabled)
        wprintf(L"\033[33m⚠ %s\033[0m\n", msg);
    else
        wprintf(L"[WARN] %s\n", msg);
}
inline void LensCLI::PrintInfo(const wchar_t* msg)
{
    if (s_ansiEnabled)
        wprintf(L"\033[36mℹ %s\033[0m\n", msg);
    else
        wprintf(L"[INFO] %s\n", msg);
}

inline void LensCLI::PrintProgressBar(uint32_t done, uint32_t total, uint32_t width)
{
    if (!total)
        return;
    uint32_t filled = (done * width) / total;
    wprintf(L"\r[");
    for (uint32_t i = 0; i < width; ++i)
        wprintf(i < filled ? L"█" : L"░");
    wprintf(L"] %u/%u", done, total);
    if (done == total)
        wprintf(L"\n");
    fflush(stdout);
}

}  // namespace Engine
}  // namespace ExplorerLens
