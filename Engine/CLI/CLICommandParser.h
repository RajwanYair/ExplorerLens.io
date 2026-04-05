// CLICommandParser.h — Argument Parsing for lens.exe
// Copyright (c) 2026 ExplorerLens Project
//
// Parses argc/argv into typed option structures consumed by LensCLI.
// Supports positional args, long flags (--flag), short flags (-f),
// key=value pairs, and generates formatted --help output.
//
#pragma once
#include <windows.h>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "LensCLI.h"

namespace ExplorerLens {
namespace Engine {

// Token types after lexing
enum class TokenType {
    Flag,
    ShortFlag,
    Positional,
    KeyValue
};

struct Token
{
    TokenType type;
    std::wstring key;    // flag name (without --)
    std::wstring value;  // value (may be empty for boolean flags)
};

// Declared option schema for --help generation
struct OptionDef
{
    std::wstring longName;  // e.g. L"output"
    wchar_t shortName;      // e.g. L'o'; 0 if none
    std::wstring metavar;   // e.g. L"<path>"
    std::wstring help;
    bool required = false;
    bool isFlag = false;  // true = boolean switch (no value)
};

class CLICommandParser
{
  public:
    // Parse argv and return the detected subcommand + filled options
    // Returns CLICommand::None on parse error (message printed to stderr)
    static CLICommand Parse(int argc, wchar_t* argv[], GenerateOptions& genOpts, BatchOptions& batchOpts,
                            std::wstring& infoPath);

    // Print formatted help for a given subcommand (or global help if None)
    static void PrintHelp(CLICommand cmd = CLICommand::None)
    {
        if (cmd == CLICommand::Generate || cmd == CLICommand::None) {
            if (cmd == CLICommand::Generate) {
                wprintf(L"\n  \033[1mlens generate\033[0m — Create a single thumbnail image\n\n");
                wprintf(L"  \033[1mUsage:\033[0m  lens generate <input-file> [options]\n\n");
                wprintf(L"  \033[1mOptions:\033[0m\n");
                wprintf(L"    -o, --output <path>   Output file path (default: <input>_thumb.png)\n");
                wprintf(L"    -s, --size <px>       Thumbnail size in pixels (default: 256)\n");
                wprintf(L"    -q, --quality         High-quality mode: Lanczos resize, better color.\n");
                wprintf(L"                          3-5x slower. Use for archival or print previews.\n");
                wprintf(L"        --cpu             Force CPU-only rendering (skip GPU acceleration)\n");
                wprintf(L"        --timeout <ms>    Max processing time in milliseconds (default: 15000)\n");
                wprintf(L"\n");
                wprintf(L"  \033[1mExamples:\033[0m\n");
                wprintf(L"    \033[36m$\033[0m lens generate vacation.heic\n");
                wprintf(L"      → vacation_thumb.png (256×256, auto GPU backend)\n\n");
                wprintf(L"    \033[36m$\033[0m lens generate blueprint.dwg -o preview.png -s 1024 -q\n");
                wprintf(L"      → preview.png (1024×1024, high-quality)\n\n");
                wprintf(L"    \033[36m$\033[0m lens generate scan.dcm --cpu --timeout 30000\n");
                wprintf(L"      → scan_thumb.png (CPU fallback, 30s timeout for large DICOM)\n\n");
                wprintf(L"  \033[1mOutput:\033[0m PNG file matching the input's visual content.\n");
                wprintf(L"  Supports 200+ formats: images, RAW, PDF, archives, 3D, CAD, video...\n\n");
                return;
            }
        }
        if (cmd == CLICommand::Batch || cmd == CLICommand::None) {
            if (cmd == CLICommand::Batch) {
                wprintf(L"\n  \033[1mlens batch\033[0m — Generate thumbnails for every file in a folder\n\n");
                wprintf(L"  \033[1mUsage:\033[0m  lens batch <directory> [options]\n\n");
                wprintf(L"  \033[1mOptions:\033[0m\n");
                wprintf(L"    -o, --output <dir>    Output directory (default: <input>/thumbs/)\n");
                wprintf(L"    -s, --size <px>       Thumbnail size in pixels (default: 256)\n");
                wprintf(L"    -f, --filter <glob>   File filter, e.g. \"*.psd\" (default: all supported)\n");
                wprintf(L"    -r, --recursive       Include all subdirectories\n");
                wprintf(L"    -j, --threads <n>     Worker threads, 0 = auto-detect (default: auto)\n");
                wprintf(L"        --no-skip         Regenerate even if thumbnail already exists\n");
                wprintf(L"        --no-progress     Disable the progress bar\n");
                wprintf(L"\n");
                wprintf(L"  \033[1mExamples:\033[0m\n");
                wprintf(L"    \033[36m$\033[0m lens batch ./photos -r -j 8\n");
                wprintf(L"      → Recursive, 8 threads, skip existing. Fast for large libraries.\n\n");
                wprintf(L"    \033[36m$\033[0m lens batch C:\\Assets -f \"*.psd\" -o C:\\Previews -s 512\n");
                wprintf(L"      → PSD files only, 512px, output to separate directory.\n\n");
                wprintf(L"    \033[36m$\033[0m lens batch ./project --no-skip --no-progress > report.txt\n");
                wprintf(L"      → Regenerate all, pipe status to file for CI integration.\n\n");
                wprintf(L"  \033[1mProgress:\033[0m [████████░░] 80/100 — photo-5.cr2\n\n");
                return;
            }
        }
        if (cmd == CLICommand::Cache) {
            wprintf(L"\n  \033[1mlens cache\033[0m — Manage the thumbnail cache\n\n");
            wprintf(L"  \033[1mUsage:\033[0m  lens cache [subcommand]\n\n");
            wprintf(L"  \033[1mSubcommands:\033[0m\n");
            wprintf(L"    stats    Show cache hit rate, entry count, and memory usage\n");
            wprintf(L"    clear    Remove all cached thumbnails\n");
            wprintf(L"    warm     Pre-populate cache for a directory\n\n");
            wprintf(L"  \033[1mExamples:\033[0m\n");
            wprintf(L"    \033[36m$\033[0m lens cache stats\n");
            wprintf(L"      → Hit rate: 94.2%% | Entries: 12,340 | Budget: 412/512 MB\n\n");
            wprintf(L"    \033[36m$\033[0m lens cache clear\n");
            wprintf(L"      → Cache cleared (freed 412 MB)\n\n");
            return;
        }
        if (cmd == CLICommand::Info) {
            wprintf(L"\n  \033[1mlens info\033[0m — Display file metadata and decoder information\n\n");
            wprintf(L"  \033[1mUsage:\033[0m  lens info <file>\n\n");
            wprintf(L"  \033[1mOutput fields:\033[0m\n");
            wprintf(L"    Format, MIME type, decoder class, GPU acceleration status,\n");
            wprintf(L"    file size, dimensions (images), page count (documents),\n");
            wprintf(L"    color space, bit depth, embedded thumbnail presence.\n\n");
            wprintf(L"  \033[1mExample:\033[0m\n");
            wprintf(L"    \033[36m$\033[0m lens info sunset.avif\n");
            wprintf(L"      → Format: AVIF | Decoder: AVIFDecoder (dav1d) | GPU: Yes\n");
            wprintf(L"        Size: 4.2 MB | Dimensions: 6000×4000 | Bit depth: 10\n\n");
            return;
        }
        if (cmd == CLICommand::Formats) {
            wprintf(L"\n  \033[1mlens formats\033[0m — List all supported file extensions\n\n");
            wprintf(L"  \033[1mUsage:\033[0m  lens formats [--category <name>]\n\n");
            wprintf(L"  \033[1mCategories:\033[0m images, archives, documents, raw, video, audio,\n");
            wprintf(L"              3d, cad, scientific, fonts, code\n\n");
            wprintf(L"  \033[1mExamples:\033[0m\n");
            wprintf(L"    \033[36m$\033[0m lens formats\n");
            wprintf(L"      → Full list: .jpg .png .webp .avif .jxl .heic .pdf .psd ...\n\n");
            wprintf(L"    \033[36m$\033[0m lens formats --category raw\n");
            wprintf(L"      → .cr2 .cr3 .nef .arw .raf .orf .dng .rw2 .pef .srw ...\n\n");
            wprintf(L"    \033[36m$\033[0m lens formats | findstr /i web\n");
            wprintf(L"      → .webp .webm (pipe-friendly output for scripts)\n\n");
            return;
        }
        // Global help — delegate to LensCLI::PrintUsage()
    }

  private:
    static std::vector<Token> Lex(int argc, wchar_t* argv[], int startFrom = 1);

    static bool ParseGenerate(const std::vector<Token>& tokens, GenerateOptions& opts);
    static bool ParseBatch(const std::vector<Token>& tokens, BatchOptions& opts);

    static std::optional<std::wstring> FindValue(const std::vector<Token>& tokens, const wchar_t* longName,
                                                 wchar_t shortName = 0);
    static bool HasFlag(const std::vector<Token>& tokens, const wchar_t* longName, wchar_t shortName = 0);

    static uint32_t ParseUInt(const std::wstring& s, uint32_t defaultVal);

    // Option schemas for each subcommand
    static const std::vector<OptionDef> s_generateSchema;
    static const std::vector<OptionDef> s_batchSchema;
};

inline std::vector<Token> CLICommandParser::Lex(int argc, wchar_t* argv[], int startFrom)
{
    std::vector<Token> tokens;
    for (int i = startFrom; i < argc; ++i) {
        std::wstring arg(argv[i]);
        if (arg.size() > 2 && arg.substr(0, 2) == L"--") {
            auto eq = arg.find(L'=');
            if (eq != std::wstring::npos)
                tokens.push_back({TokenType::KeyValue, arg.substr(2, eq - 2), arg.substr(eq + 1)});
            else
                tokens.push_back({TokenType::Flag, arg.substr(2), L""});
        } else if (arg.size() == 2 && arg[0] == L'-') {
            tokens.push_back({TokenType::ShortFlag, std::wstring(1, arg[1]), L""});
        } else {
            tokens.push_back({TokenType::Positional, L"", arg});
        }
    }
    return tokens;
}

inline bool CLICommandParser::HasFlag(const std::vector<Token>& tokens, const wchar_t* longName, wchar_t shortName)
{
    for (auto& t : tokens) {
        if (t.type == TokenType::Flag && t.key == longName)
            return true;
        if (shortName && t.type == TokenType::ShortFlag && t.key[0] == shortName)
            return true;
    }
    return false;
}

inline std::optional<std::wstring> CLICommandParser::FindValue(const std::vector<Token>& tokens,
                                                               const wchar_t* longName, wchar_t shortName)
{
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto& t = tokens[i];
        if (t.type == TokenType::KeyValue && t.key == longName)
            return t.value;
        if ((t.type == TokenType::Flag && t.key == longName)
            || (shortName && t.type == TokenType::ShortFlag && t.key[0] == shortName)) {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Positional)
                return tokens[i + 1].value;
        }
    }
    return std::nullopt;
}

inline uint32_t CLICommandParser::ParseUInt(const std::wstring& s, uint32_t def)
{
    try {
        return static_cast<uint32_t>(std::stoul(s));
    } catch (...) {
        return def;
    }
}

}  // namespace Engine
}  // namespace ExplorerLens
