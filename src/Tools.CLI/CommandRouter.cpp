// CommandRouter.cpp — CLI Command Dispatch Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Implements CommandRouter: argv parsing, help formatting, and dispatch to
// registered subcommand handlers. All subcommand objects are registered in
// CreateLensCLI() using the factory pattern.
//
#include <windows.h>
#include "CommandRouter.h"
#include "GenerateCommand.h"
#include "InfoCommand.h"
#include "CacheCommand.h"
#include "RegisterCommand.h"
#include "BenchmarkCommand.h"
#include "DoctorCommand.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace ExplorerLens {
namespace CLI {

//==============================================================================
// CommandRouter
//==============================================================================

CommandRouter::CommandRouter() = default;

void CommandRouter::Register(std::unique_ptr<ISubCommand> cmd)
{
    if (!cmd) return;
    std::wstring name(cmd->Name());
    m_commands[name] = std::move(cmd);
}

ParsedArgs CommandRouter::Parse(int argc, wchar_t* argv[])
{
    ParsedArgs result;

    // argv[0] = executable path; argv[1] = subcommand (if any)
    int startIdx = 1;
    if (argc > 1 && argv[1][0] != L'-') {
        result.subcommand = argv[1];
        startIdx = 2;
    }

    for (int i = startIdx; i < argc; ++i)
    {
        std::wstring tok(argv[i]);

        if (tok.starts_with(L"--") || (tok.size() == 2 && tok[0] == L'-'))
        {
            // Check if next token is a value for this option
            bool isFlag = true;
            if (i + 1 < argc) {
                std::wstring next(argv[i + 1]);
                if (next[0] != L'-') {
                    result.options[tok] = next;
                    ++i;
                    isFlag = false;
                }
            }
            if (isFlag) result.flags.push_back(tok);
        }
        else
        {
            result.positional.push_back(tok);
        }
    }

    return result;
}

void CommandRouter::PrintHelp() const
{
    std::wcout << L"\nlens.exe v" << VERSION << L" — ExplorerLens CLI Tool\n"
               << L"CLSID: " << CLSID << L"\n\n"
               << L"Usage: lens <subcommand> [options]\n\n"
               << L"Subcommands:\n";

    for (const auto& [name, cmd] : m_commands) {
        std::wcout << L"  " << std::left << std::setw(16) << name
                   << L"  " << cmd->ShortDesc() << L"\n";
    }

    std::wcout << L"\nOptions:\n"
               << L"  --verbose, -v     Enable verbose output\n"
               << L"  --json, -j        JSON output format\n"
               << L"  --help, -h        Show this help\n"
               << L"  --version         Print version and exit\n\n"
               << L"Run 'lens <subcommand> --help' for subcommand-specific options.\n";
}

int CommandRouter::Dispatch(int argc, wchar_t* argv[])
{
    auto args = Parse(argc, argv);

    // Handle global flags before subcommand dispatch
    if (args.subcommand.empty() || args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        PrintHelp();
        return static_cast<int>(ExitCode::Success);
    }

    if (args.HasFlag(L"--version")) {
        std::wcout << L"lens.exe " << VERSION << L"\n";
        return static_cast<int>(ExitCode::Success);
    }

    auto it = m_commands.find(args.subcommand);
    if (it == m_commands.end()) {
        std::wcerr << L"lens: unknown subcommand '" << args.subcommand << L"'\n"
                   << L"Run 'lens --help' for available subcommands.\n";
        return static_cast<int>(ExitCode::UnknownCommand);
    }

    return it->second->Execute(args);
}

//==============================================================================
// Factory
//==============================================================================

std::unique_ptr<CommandRouter> CreateLensCLI()
{
    auto router = std::make_unique<CommandRouter>();
    router->Register(std::make_unique<GenerateCommand>());
    router->Register(std::make_unique<InfoCommand>());
    router->Register(std::make_unique<CacheCommand>());
    router->Register(std::make_unique<RegisterCommand>());
    router->Register(std::make_unique<BenchmarkCommand>());
    router->Register(std::make_unique<DoctorCommand>());
    return router;
}

} // namespace CLI
} // namespace ExplorerLens
