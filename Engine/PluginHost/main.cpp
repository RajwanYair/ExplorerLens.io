/******************************************************************************
 * ExplorerLens Plugin Host
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Separate process host for running untrusted plugins in isolation.
 * 
 * Usage:
 * PluginHost.exe --plugin "path\to\plugin.dll" --pipe "pipe-name" [--timeout 30000]
 *****************************************************************************/

#include "PluginHostServer.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <cstdlib>

namespace {

struct CommandLineArgs {
 std::wstring plugin_path;
 std::wstring pipe_name;
 uint32_t timeout_ms = 30000;
 bool valid = false;
};

void PrintUsage() {
 std::wcout << L"ExplorerLens PluginHost v1.0\n";
 std::wcout << L"\n";
 std::wcout << L"Usage:\n";
 std::wcout << L" PluginHost.exe --plugin <path> --pipe <name> [--timeout <ms>]\n";
 std::wcout << L"\n";
 std::wcout << L"Options:\n";
 std::wcout << L" --plugin <path> Path to plugin DLL\n";
 std::wcout << L" --pipe <name> Named pipe name for IPC\n";
 std::wcout << L" --timeout <ms> Request timeout in milliseconds (default: 30000)\n";
 std::wcout << L" --help Show this help message\n";
 std::wcout << L"\n";
}

CommandLineArgs ParseCommandLine(int argc, wchar_t* argv[]) {
 CommandLineArgs args;
 
 for (int i = 1; i < argc; ++i) {
 std::wstring arg = argv[i];
 
 if (arg == L"--help" || arg == L"-h" || arg == L"/?") {
 PrintUsage();
 return args;
 }
 
 if (arg == L"--plugin" && i + 1 < argc) {
 args.plugin_path = argv[++i];
 }
 else if (arg == L"--pipe" && i + 1 < argc) {
 args.pipe_name = argv[++i];
 }
 else if (arg == L"--timeout" && i + 1 < argc) {
 args.timeout_ms = static_cast<uint32_t>(_wtoi(argv[++i]));
 }
 else {
 std::wcerr << L"Unknown argument: " << arg << L"\n";
 return args;
 }
 }
 
 // Validate required arguments
 if (args.plugin_path.empty()) {
 std::wcerr << L"Error: --plugin is required\n";
 return args;
 }
 
 if (args.pipe_name.empty()) {
 std::wcerr << L"Error: --pipe is required\n";
 return args;
 }
 
 args.valid = true;
 return args;
}

} // anonymous namespace

//============================================================================
// Main Entry Point
//============================================================================

int wmain(int argc, wchar_t* argv[]) {
 // Set console code page to UTF-8
 SetConsoleOutputCP(CP_UTF8);
 
 // Parse command line
 CommandLineArgs args = ParseCommandLine(argc, argv);
 if (!args.valid) {
 PrintUsage();
 return 1;
 }
 
 std::wcout << L"ExplorerLens PluginHost starting...\n";
 std::wcout << L" Plugin: " << args.plugin_path << L"\n";
 std::wcout << L" Pipe: " << args.pipe_name << L"\n";
 std::wcout << L" Timeout: " << args.timeout_ms << L" ms\n";
 
 // Create plugin host server
 ExplorerLens::PluginHost::PluginHostServer server;
 
 // Initialize
 if (!server.Initialize(args.plugin_path, args.pipe_name, args.timeout_ms)) {
 std::wcerr << L"Failed to initialize PluginHost\n";
 return 2;
 }
 
 std::wcout << L"PluginHost initialized successfully\n";
 
 // Run message loop
 server.Run();
 
 std::wcout << L"PluginHost shutting down...\n";
 
 // Cleanup
 server.Shutdown();
 
 return 0;
}

