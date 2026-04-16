// main.cpp — LensServer entry point
// Copyright (c) 2026 ExplorerLens Project
//
// Usage: LensServer [--port <n>] [--bind <addr>] [--gpu-off] [--verbose]
//
#include "LensServer.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <csignal>

static ExplorerLens::Server::LensServer* g_server = nullptr;

static void SignalHandler(int sig) {
    std::cout << "\n[LensServer] signal " << sig << " — shutting down\n";
    if (g_server) g_server->Stop();
}

static std::string GetArg(int argc, char* argv[], const char* flag, const char* def = "") {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == flag) return argv[i + 1];
    }
    return def;
}

static bool HasFlag(int argc, char* argv[], const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (HasFlag(argc, argv, "--help") || HasFlag(argc, argv, "-h")) {
        std::cout << "Usage: LensServer [options]\n\n"
                     "Options:\n"
                     "  --port  <n>       Listen port (default: 8765)\n"
                     "  --bind  <addr>    Bind address (default: 127.0.0.1)\n"
                     "  --gpu-off         Disable GPU acceleration\n"
                     "  --verbose         Log every request\n"
                     "  --max-size <px>   Maximum thumbnail size (default: 1024)\n\n"
                     "Endpoints:\n"
                     "  GET /health\n"
                     "  GET /metrics\n"
                     "  GET /thumbnail?path=<encoded>&size=<px>\n";
        return 0;
    }

    ExplorerLens::Server::ServerConfig cfg;
    cfg.port        = static_cast<uint16_t>(std::stoul(GetArg(argc, argv, "--port",     "8765")));
    cfg.bindAddress = GetArg(argc, argv, "--bind",     "127.0.0.1");
    cfg.maxSize     = static_cast<uint32_t>(std::stoul(GetArg(argc, argv, "--max-size", "1024")));
    cfg.gpuEnabled  = !HasFlag(argc, argv, "--gpu-off");
    cfg.verboseLog  = HasFlag(argc, argv, "--verbose");

    ExplorerLens::Server::LensServer server(cfg);
    g_server = &server;

    std::signal(SIGINT,  SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    if (!server.Start()) {
        std::cerr << "[LensServer] Failed to start — check port "
                  << cfg.port << " is not in use\n";
        return 1;
    }

    std::cout << "[LensServer] Running. Press Ctrl+C to stop.\n";

    // Wait until stopped by signal
    while (server.IsRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
