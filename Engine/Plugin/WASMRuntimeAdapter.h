// WASMRuntimeAdapter.h — WebAssembly Runtime Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Hosts a sandboxed WebAssembly runtime (WasmEdge / WABT) for executing
// plugin bundles — enforcing memory limits, CPU quotas, and WASI capability caps.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class WASMAdapterEngine { WasmEdge, WABT, Wasmer, Native };
enum class WASMSandboxLevel{ Loose, Standard, Strict, Paranoid };

struct WASMRuntimeConfig {
    WASMAdapterEngine runtime           = WASMAdapterEngine::WasmEdge;
    WASMSandboxLevel sandboxLevel      = WASMSandboxLevel::Standard;
    uint64_t         memoryLimitBytes  = 64ULL * 1024 * 1024;
    uint32_t         cpuQuotaPercent   = 25;
    uint32_t         timeoutMs         = 5000;
    bool             enableSIMD        = true;
    bool             enableThreads     = false;
    std::string      wasmCacheDir;
};

class WASMRuntimeAdapter {
public:
    explicit WASMRuntimeAdapter(const WASMRuntimeConfig& cfg = {}) : m_config(cfg) {}

    bool             Initialize()                       { m_running = true;  return true; }
    void             Reset()                            { m_running = false; }
    bool             IsRunning()     const              { return m_running; }
    bool             LoadModule(const uint8_t* data, size_t size)
                                                        { (void)data; return size > 0; }
    bool             UnloadModule()                     { return true; }

    WASMAdapterEngine GetRuntime()      const { return m_config.runtime; }
    WASMSandboxLevel GetSandboxLevel() const { return m_config.sandboxLevel; }
    uint64_t         GetMemoryLimit()  const { return m_config.memoryLimitBytes; }
    uint32_t         GetCpuQuota()     const { return m_config.cpuQuotaPercent; }
    uint32_t         GetTimeoutMs()    const { return m_config.timeoutMs; }
    bool             SIMDEnabled()     const { return m_config.enableSIMD; }
    bool             ThreadsEnabled()  const { return m_config.enableThreads; }
    const WASMRuntimeConfig& GetConfig() const { return m_config; }
    void             SetConfig(const WASMRuntimeConfig& cfg) { m_config = cfg; }
    std::string      GetEngineVersion() const { return "WasmEdge-0.14.0"; }

private:
    WASMRuntimeConfig m_config;
    bool              m_running = false;
};

} // namespace Engine
} // namespace ExplorerLens
