// main.cpp — lens.exe Entry Point (wmain)
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 23 (v15.4.0 "Zenith-U"): Wide-character entry point that wires up
// signal handling, initialises COM (STA), and dispatches to CommandRouter.
// Prints usage and exits 127 for unknown subcommands; exits 0 on --help.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>
#include <signal.h>
#include "CommandRouter.h"
#include <iostream>

#pragma comment(lib, "ole32.lib")

//==============================================================================
// Signal handling — graceful Ctrl-C / SIGTERM
//==============================================================================

static volatile bool g_shutdown = false;

static void HandleSignal(int /*signum*/) noexcept
{
    g_shutdown = true;
}

//==============================================================================
// wmain — wide-character entry point required for correct Unicode path handling
//==============================================================================

int wmain(int argc, wchar_t* argv[])
{
    signal(SIGINT,  HandleSignal);
    signal(SIGTERM, HandleSignal);

    // Enable virtual terminal sequences so ANSI colours work in Windows Terminal / conhost
    HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode = 0;
    if (::GetConsoleMode(hOut, &consoleMode)) {
        ::SetConsoleMode(hOut, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    // Initialise COM (single-threaded apartment; required for shell APIs)
    HRESULT hrCom = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom)) {
        std::wcerr << L"lens: COM initialisation failed (0x"
                   << std::hex << hrCom << L")\n";
        return static_cast<int>(ExplorerLens::CLI::ExitCode::GeneralError);
    }

    auto router = ExplorerLens::CLI::CreateLensCLI();
    int exitCode = router->Dispatch(argc, argv);

    ::CoUninitialize();
    return exitCode;
}
