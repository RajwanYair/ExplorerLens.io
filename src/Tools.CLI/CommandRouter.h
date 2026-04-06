// CommandRouter.h — lens.exe CLI Entry Point & Subcommand Router
// Copyright (c) 2026 ExplorerLens Project
//
// Provides the main entry point for lens.exe.
// Parses argv, dispatches to typed command handlers, and manages exit codes.
// Subcommands: generate, info, cache, register, unregister, benchmark, doctor.
//
#pragma once

#include <windows.h>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include "../../Engine/Core/BuildValidation.h"

namespace ExplorerLens {
namespace CLI {

//==============================================================================
// Exit Codes
//==============================================================================

enum class ExitCode : int {
    Success           = 0,
    GeneralError      = 1,
    InvalidArguments  = 2,
    FileNotFound      = 3,
    AccessDenied      = 4,
    NotRegistered     = 5,
    AdminRequired     = 6,
    DecoderError      = 10,
    CacheError        = 11,
    GPUError          = 12,
    UnknownCommand    = 127
};

//==============================================================================
// ParsedArgs — structured argument bag for all subcommands
//==============================================================================

struct ParsedArgs {
    std::wstring subcommand;
    std::vector<std::wstring> positional;
    std::map<std::wstring, std::wstring> options;   // --key value
    std::vector<std::wstring> flags;                // --flag (boolean)

    bool HasFlag(std::wstring_view name) const noexcept {
        return std::find(flags.begin(), flags.end(),
                         std::wstring(name)) != flags.end();
    }

    std::wstring GetOption(std::wstring_view name,
                           std::wstring_view defaultVal = L"") const {
        auto it = options.find(std::wstring(name));
        return it != options.end() ? it->second : std::wstring(defaultVal);
    }

    bool HasOption(std::wstring_view name) const noexcept {
        return options.count(std::wstring(name)) > 0;
    }

    bool Verbose()    const noexcept { return HasFlag(L"--verbose") || HasFlag(L"-v"); }
    bool JsonOutput() const noexcept { return HasFlag(L"--json") || HasFlag(L"-j"); }
    bool Recursive()  const noexcept { return HasFlag(L"--recursive") || HasFlag(L"-r"); }
};

//==============================================================================
// ISubCommand — interface every subcommand handler implements
//==============================================================================

class ISubCommand {
public:
    virtual ~ISubCommand() = default;
    virtual int Execute(const ParsedArgs& args) = 0;
    virtual std::wstring_view Name()        const noexcept = 0;
    virtual std::wstring_view ShortDesc()   const noexcept = 0;
    virtual std::wstring_view Usage()       const noexcept = 0;
};

//==============================================================================
// CommandRouter — dispatches CLI args to registered subcommands
//==============================================================================

class CommandRouter {
public:
    CommandRouter();

    void Register(std::unique_ptr<ISubCommand> cmd);
    int  Dispatch(int argc, wchar_t* argv[]);
    void PrintHelp() const;

    // Parse raw argv into a ParsedArgs structure.
    static ParsedArgs Parse(int argc, wchar_t* argv[]);

private:
    std::map<std::wstring, std::unique_ptr<ISubCommand>> m_commands;

    static constexpr std::wstring_view CLSID = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";
};

//==============================================================================
// Factory — creates a fully-wired CommandRouter with all subcommands
//==============================================================================

std::unique_ptr<CommandRouter> CreateLensCLI();

} // namespace CLI
} // namespace ExplorerLens
