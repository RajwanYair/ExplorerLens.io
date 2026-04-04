#pragma once
// RustFFIBridge.h — Rust FFI Bridge
// Infrastructure for calling Rust-compiled decoders via C ABI,
// enabling memory-safe decoder implementations while maintaining
// performance through zero-copy buffer sharing.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Rust library loading status
enum class RustLibStatus : uint8_t {
    NotLoaded = 0,    // Library not loaded
    Loaded,           // Successfully loaded
    VersionMismatch,  // ABI version incompatible
    MissingSymbols,   // Required exports missing
    LoadFailed,       // DLL load failed
    COUNT
};

/// FFI calling convention
enum class FFICallingConv : uint8_t {
    CDecl = 0,  // __cdecl (default C ABI)
    StdCall,    // __stdcall (Win32)
    FastCall,   // __fastcall (register params)
    COUNT
};

struct RustDecoderInfo
{
    const char* name = nullptr;
    uint32_t abiVersion = 0;
    uint32_t decoderCount = 0;
    RustLibStatus status = RustLibStatus::NotLoaded;
    FFICallingConv convention = FFICallingConv::CDecl;
    bool threadSafe = false;
    bool panicHandler = false;
    size_t heapUsageBytes = 0;
};

struct FFIBridgeConfig
{
    bool enabled = false;  // Disabled by default until Rust libs available
    uint32_t minABIVersion = 1;
    uint32_t maxABIVersion = 10;
    bool isolateHeap = true;  // Separate Rust allocator heap
    bool catchPanics = true;  // Catch Rust panics at boundary
    uint32_t callTimeoutMs = 5000;
};

class RustFFIBridge
{
  public:
    static constexpr size_t StatusCount()
    {
        return static_cast<size_t>(RustLibStatus::COUNT);
    }
    static constexpr size_t ConventionCount()
    {
        return static_cast<size_t>(FFICallingConv::COUNT);
    }

    static const wchar_t* StatusName(RustLibStatus s)
    {
        switch (s) {
            case RustLibStatus::NotLoaded:
                return L"Not Loaded";
            case RustLibStatus::Loaded:
                return L"Loaded";
            case RustLibStatus::VersionMismatch:
                return L"Version Mismatch";
            case RustLibStatus::MissingSymbols:
                return L"Missing Symbols";
            case RustLibStatus::LoadFailed:
                return L"Load Failed";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* ConventionName(FFICallingConv c)
    {
        switch (c) {
            case FFICallingConv::CDecl:
                return L"cdecl";
            case FFICallingConv::StdCall:
                return L"stdcall";
            case FFICallingConv::FastCall:
                return L"fastcall";
            default:
                return L"Unknown";
        }
    }

    /// Check if ABI version is in supported range
    static bool IsABICompatible(uint32_t version, const FFIBridgeConfig& cfg)
    {
        return version >= cfg.minABIVersion && version <= cfg.maxABIVersion;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
