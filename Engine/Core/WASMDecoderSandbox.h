#pragma once
// WASMDecoderSandbox.h — WASM Decoder Sandbox
// WebAssembly-based decoder sandboxing using WAMR (WebAssembly Micro Runtime)
// for isolating untrusted format decoders in a memory-safe sandbox.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// WASM runtime engine
enum class WASMRuntime : uint8_t {
    WAMR_Interpreter = 0,  // Slow but portable
    WAMR_AOT,              // Ahead-of-time compiled
    WAMR_JIT,              // Just-in-time compiled
    Wasmtime,              // Bytecode Alliance runtime
    Wasmer,                // Wasmer runtime
    COUNT
};

/// Sandbox memory limit tier
enum class SandboxMemoryTier : uint8_t {
    Minimal_16MB = 0,
    Standard_64MB,
    Extended_256MB,
    Large_1GB,
    COUNT
};

struct WASMModuleInfo
{
    const char* moduleName = nullptr;
    WASMRuntime runtime = WASMRuntime::WAMR_AOT;
    SandboxMemoryTier memoryTier = SandboxMemoryTier::Standard_64MB;
    uint32_t exportCount = 0;
    uint32_t importCount = 0;
    size_t codeSizeBytes = 0;
    bool simdEnabled = false;
    bool threadsEnabled = false;
    double loadTimeMs = 0.0;
};

struct WASMSandboxConfig
{
    bool enabled = false;  // Experimental
    WASMRuntime preferredRuntime = WASMRuntime::WAMR_AOT;
    SandboxMemoryTier memoryLimit = SandboxMemoryTier::Standard_64MB;
    uint32_t stackSizeKB = 512;
    uint32_t executionTimeoutMs = 10000;
    bool allowFileIO = false;
    bool allowNetworkIO = false;
    bool enableSIMD = true;
};

class WASMDecoderSandbox
{
  public:
    static constexpr size_t RuntimeCount()
    {
        return static_cast<size_t>(WASMRuntime::COUNT);
    }
    static constexpr size_t MemoryTierCount()
    {
        return static_cast<size_t>(SandboxMemoryTier::COUNT);
    }

    static const wchar_t* RuntimeName(WASMRuntime r)
    {
        switch (r) {
            case WASMRuntime::WAMR_Interpreter:
                return L"WAMR Interpreter";
            case WASMRuntime::WAMR_AOT:
                return L"WAMR AOT";
            case WASMRuntime::WAMR_JIT:
                return L"WAMR JIT";
            case WASMRuntime::Wasmtime:
                return L"Wasmtime";
            case WASMRuntime::Wasmer:
                return L"Wasmer";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* MemoryTierName(SandboxMemoryTier t)
    {
        switch (t) {
            case SandboxMemoryTier::Minimal_16MB:
                return L"16 MB";
            case SandboxMemoryTier::Standard_64MB:
                return L"64 MB";
            case SandboxMemoryTier::Extended_256MB:
                return L"256 MB";
            case SandboxMemoryTier::Large_1GB:
                return L"1 GB";
            default:
                return L"Unknown";
        }
    }

    /// Get memory limit in bytes
    static constexpr size_t MemoryLimitBytes(SandboxMemoryTier tier)
    {
        switch (tier) {
            case SandboxMemoryTier::Minimal_16MB:
                return 16ULL * 1024 * 1024;
            case SandboxMemoryTier::Standard_64MB:
                return 64ULL * 1024 * 1024;
            case SandboxMemoryTier::Extended_256MB:
                return 256ULL * 1024 * 1024;
            case SandboxMemoryTier::Large_1GB:
                return 1024ULL * 1024 * 1024;
            default:
                return 64ULL * 1024 * 1024;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
