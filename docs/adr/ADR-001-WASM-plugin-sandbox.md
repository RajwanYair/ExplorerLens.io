# ADR-001: WASM Plugin Sandbox over Native DLL

**Status:** Accepted  
**Date:** 2026-01-15  
**Implemented in:** v25.0.0 "Rigel"  
**Affected components:** `Engine/Plugin/`, `SDK/plugin_api.h`, `LENSShell.dll`

---

## Context

ExplorerLens v24.x and earlier used a native DLL plugin model: plugins are C++ DLLs loaded
into the LENSShell.dll process via `LoadLibrary`. This created three critical problems:

1. **Trust** — A malicious or buggy plugin can crash Explorer.exe, corrupt heap state, or
   exfiltrate credentials. The host process has zero isolation from the plugin.
2. **Toolchain coupling** — Plugin developers must build against the exact MSVC toolset
   version (14.40+) and CRT linkage (/MD) that the host was built with. Minor toolchain
   mismatches cause mysterious crashes.
3. **Deployment complexity** — Native DLLs require code signing, COM registration, and
   Windows Defender SmartScreen onboarding. This creates a high barrier for community
   plugin developers.

## Decision

Replace the native DLL plugin model with a **WebAssembly (WASM) sandbox** using the
WasmEdge 0.14+ runtime (Component Model). Plugin developers ship a `.wasm` bundle.
The host loads it inside a WasmEdge instance with configured WASI capability scope.

Key design choices:
- **Component Model** (WIT bindings) as the plugin ABI — no C++ virtual vtables
- **JIT compilation** of WASM → native on first load, cached to disk per plugin version
- **Resource limits** enforced by `WASMHostController` (memory, CPU ticks, wall time)
- **Capability declaration** required in plugin manifest — deny-by-default

## Rationale

| Criterion | Native DLL | WASM Sandbox |
|-----------|-----------|--------------|
| Crash isolation | ❌ None | ✅ Subprocess boundary |
| Toolchain independence | ❌ MSVC-locked | ✅ Any language → WASM |
| Memory safety | ❌ Shared heap | ✅ 4 GB linear address space |
| Deployment | ❌ Signing + COM | ✅ Drop `.wasm` file |
| Performance (JIT warm) | 1× | ~0.85× (acceptable) |
| Debug support | ✅ VS debugger | ✅ CDP/DAP via bridge |

The 15% JIT overhead is well within the rendering time budget for thumbnail decoding,
which is dominated by I/O and image decode time (not plugin control flow).

## Consequences

**Positive:**
- Plugin market opened to non-MSVC developers (Rust, Go, Zig, C, AssemblyScript)
- Crashes in plugins are fully isolated — LENSShell.dll never crashes due to plugin fault
- Community plugins no longer require code-signing certificates

**Negative:**
- JIT compile adds ~200 ms to first plugin load (mitigated by `WASMHotSwapEngine` caching)
- WASM runtime adds ~2 MB to LENSShell.dll size
- Debugging experience is slightly worse than native (CDP bridge required)

## Alternatives Considered

1. **Out-of-process COM server** — Provides crash isolation but retains MSVC coupling
   and requires IPC marshalling for every thumbnail call (~5 ms overhead per call).
   Rejected: IPC latency unacceptable for sub-17 ms thumbnail targets.

2. **AppContainer sandbox** — Windows-only, complex manifest requirements, no
   language portability. Rejected: Not cross-platform; same toolchain coupling.

3. **V8 / QuickJS scripting** — JavaScript is unsuitable for performance-critical
   binary decode. Rejected: Not appropriate for image decoding use case.
