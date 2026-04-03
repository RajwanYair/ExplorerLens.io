// CLICommandParser.h — Argument Parsing for lens.exe
// Copyright (c) 2026 ExplorerLens Project
//
// Parses argc/argv into typed option structures consumed by LensCLI.
// Supports positional args, long flags (--flag), short flags (-f),
// key=value pairs, and generates formatted --help output.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <functional>
#include "LensCLI.h"

namespace ExplorerLens { namespace Engine {

// Token types after lexing
enum class TokenType { Flag, ShortFlag, Positional, KeyValue };

struct Token {
    TokenType    type;
    std::wstring key;    // flag name (without --)
    std::wstring value;  // value (may be empty for boolean flags)
};

// Declared option schema for --help generation
struct OptionDef {
    std::wstring  longName;   // e.g. L"output"
    wchar_t       shortName;  // e.g. L'o'; 0 if none
    std::wstring  metavar;    // e.g. L"<path>"
    std::wstring  help;
    bool          required = false;
    bool          isFlag   = false; // true = boolean switch (no value)
};

class CLICommandParser {
public:
    // Parse argv and return the detected subcommand + filled options
    // Returns CLICommand::None on parse error (message printed to stderr)
    static CLICommand Parse(int argc, wchar_t* argv[],
                            GenerateOptions& genOpts,
                            BatchOptions&    batchOpts,
                            std::wstring&    infoPath);

    // Print formatted help for a given subcommand (or global help if None)
    static void PrintHelp(CLICommand cmd = CLICommand::None) {
        if (cmd == CLICommand::Generate || cmd == CLICommand::None) {
            if (cmd == CLICommand::Generate) wprintf(L"\n  lens generate — Create a single thumbnail\n\n");
            if (cmd == CLICommand::Generate) wprintf(L"  Usage: lens generate <input-file> [options]\n\n");
            if (cmd == CLICommand::Generate) {
                wprintf(L"  Options:\n");
                wprintf(L"    -o, --output <path>   Output file path (default: <input>_thumb.png)\n");
                wprintf(L"    -s, --size <px>       Thumbnail size in pixels (default: 256)\n");
                wprintf(L"    -q, --quality         Enable high-quality mode (slower)\n");
                wprintf(L"        --cpu             Force CPU-only rendering (no GPU)\n");
                wprintf(L"        --timeout <ms>    Max processing time (default: 15000)\n");
                wprintf(L"\n");
                return;
            }
        }
        if (cmd == CLICommand::Batch || cmd == CLICommand::None) {
            if (cmd == CLICommand::Batch) wprintf(L"\n  lens batch — Batch-generate thumbnails for a folder\n\n");
            if (cmd == CLICommand::Batch) wprintf(L"  Usage: lens batch <directory> [options]\n\n");
            if (cmd == CLICommand::Batch) {
                wprintf(L"  Options:\n");
                wprintf(L"    -o, --output <dir>    Output directory (default: <input>/thumbs/)\n");
                wprintf(L"    -s, --size <px>       Thumbnail size in pixels (default: 256)\n");
                wprintf(L"    -f, --filter <glob>   File filter (e.g. \"*.psd\", default: all)\n");
                wprintf(L"    -r, --recursive       Include subdirectories\n");
                wprintf(L"    -j, --threads <n>     Worker threads (default: auto)\n");
                wprintf(L"        --no-skip         Regenerate existing thumbnails\n");
                wprintf(L"        --no-progress     Disable progress bar\n");
                wprintf(L"\n");
                return;
            }
        }
        // Global help — delegate to LensCLI::PrintUsage()
    }

private:
    static std::vector<Token> Lex(int argc, wchar_t* argv[], int startFrom = 1);

    static bool ParseGenerate(const std::vector<Token>& tokens, GenerateOptions& opts);
    static bool ParseBatch(const std::vector<Token>& tokens, BatchOptions& opts);

    static std::optional<std::wstring> FindValue(const std::vector<Token>& tokens,
                                                  const wchar_t* longName,
                                                  wchar_t shortName = 0);
    static bool HasFlag(const std::vector<Token>& tokens,
                        const wchar_t* longName, wchar_t shortName = 0);

    static uint32_t ParseUInt(const std::wstring& s, uint32_t defaultVal);

    // Option schemas for each subcommand
    static const std::vector<OptionDef> s_generateSchema;
    static const std::vector<OptionDef> s_batchSchema;
};

inline std::vector<Token> CLICommandParser::Lex(int argc, wchar_t* argv[], int startFrom) {
    std::vector<Token> tokens;
    for (int i = startFrom; i < argc; ++i) {
        std::wstring arg(argv[i]);
        if (arg.size() > 2 && arg.substr(0, 2) == L"--") {
            auto eq = arg.find(L'=');
            if (eq != std::wstring::npos)
                tokens.push_back({TokenType::KeyValue, arg.substr(2, eq-2), arg.substr(eq+1)});
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

inline bool CLICommandParser::HasFlag(const std::vector<Token>& tokens,
                                       const wchar_t* longName, wchar_t shortName) {
    for (auto& t : tokens) {
        if (t.type == TokenType::Flag && t.key == longName) return true;
        if (shortName && t.type == TokenType::ShortFlag && t.key[0] == shortName) return true;
    }
    return false;
}

inline std::optional<std::wstring> CLICommandParser::FindValue(
        const std::vector<Token>& tokens, const wchar_t* longName, wchar_t shortName) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto& t = tokens[i];
        if (t.type == TokenType::KeyValue && t.key == longName) return t.value;
        if ((t.type == TokenType::Flag && t.key == longName) ||
            (shortName && t.type == TokenType::ShortFlag && t.key[0] == shortName)) {
            if (i + 1 < tokens.size() && tokens[i+1].type == TokenType::Positional)
                return tokens[i+1].value;
        }
    }
    return std::nullopt;
}

inline uint32_t CLICommandParser::ParseUInt(const std::wstring& s, uint32_t def) {
    try { return static_cast<uint32_t>(std::stoul(s)); }
    catch (...) { return def; }
}

}} // namespace ExplorerLens::Engine
