// Engine/Core/LazyLibraryLoader.h
#pragma once

// LazyLibraryLoader — deferred LoadLibrary for optional codec DLLs (S363)
//
// Phase 3 exit criterion H43: libheif, libjxl, and libavif are loaded only on
// first use, reducing cold-start memory footprint of LENSShell.dll.
//
// Design:
//   Each LazyLibTarget maps to one DLL. The loader calls LoadLibrary on the
//   first request for a symbol from that library. Subsequent requests return
//   the cached HMODULE. Thread-safe: guarded by a per-entry SRWLOCK / mutex.
//
// ROADMAP ref: ROADMAP v8.0 §8.4 — H43 Lazy library loading (Phase 3)
//
// Usage:
//   auto& L = LazyLibraryLoader::Global();
//   LazyLibStatus s = L.Load(LazyLibTarget::LIBHEIF);
//   if (s == LazyLibStatus::OK) {
//       auto fn = L.GetProc<heif_image*>(LazyLibTarget::LIBHEIF, "heif_image_create");
//   }

#ifndef EXPLORERLENS_ENGINE_LAZYLIBLOADER_H
#define EXPLORERLENS_ENGINE_LAZYLIBLOADER_H

#include <cstdint>
#include <string>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// LazyLibTarget — which optional codec DLL to load on demand
// ---------------------------------------------------------------------------
enum class LazyLibTarget : std::uint8_t {
    LIBHEIF   = 0,  ///< libheif-1.19.5 HEIF/HEIC container
    LIBJXL    = 1,  ///< libjxl-0.11.1  JPEG XL decode
    LIBAVIF   = 2,  ///< libavif-1.3.0  AVIF container
    LIBRAW    = 3,  ///< LibRaw-0.21.2  Camera RAW (rarely needed lazily)
    MUPDF     = 4,  ///< MuPDF-1.24.11  PDF rasterization
    COUNT     = 5,
};

// ---------------------------------------------------------------------------
// LazyLibStatus
// ---------------------------------------------------------------------------
enum class LazyLibStatus : std::uint8_t {
    OK              = 0,
    ALREADY_LOADED  = 1,  ///< DLL was already loaded — not an error
    NOT_FOUND       = 2,  ///< LoadLibrary failed (DLL not in search path)
    ALREADY_FAILED  = 3,  ///< Previous load attempt failed; won't retry
    INVALID_TARGET  = 4,
};

// ---------------------------------------------------------------------------
// LazyLibEntry — per-DLL state record
// ---------------------------------------------------------------------------
struct LazyLibEntry {
    LazyLibTarget   target    = LazyLibTarget::LIBHEIF;
    LazyLibStatus   status    = LazyLibStatus::NOT_FOUND;
    std::wstring    dllName;    ///< DLL filename (e.g. L"libheif.dll")
    std::uint32_t   loadCount = 0u; ///< Times Load() was called successfully

    [[nodiscard]] bool IsLoaded()  const noexcept { return status == LazyLibStatus::OK || status == LazyLibStatus::ALREADY_LOADED; }
    [[nodiscard]] bool HasFailed() const noexcept { return status == LazyLibStatus::NOT_FOUND || status == LazyLibStatus::ALREADY_FAILED; }
};

// ---------------------------------------------------------------------------
// LazyLibConfig — search path and retry policy
// ---------------------------------------------------------------------------
struct LazyLibConfig {
    bool   retryOnFailure     = false;   ///< Retry on next Load() if previous failed
    bool   preferSystemPaths  = false;   ///< Prefer System32 over app-local DLLs
    std::uint32_t searchDepth = 1u;      ///< Directory depth for DLL probing

    [[nodiscard]] static LazyLibConfig Default() noexcept {
        return LazyLibConfig{};
    }

    [[nodiscard]] static LazyLibConfig ForShellExtension() noexcept {
        LazyLibConfig cfg{};
        cfg.retryOnFailure    = false;  // Shell extensions must not stall on repeated load
        cfg.preferSystemPaths = true;
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// LazyLibraryLoader — singleton deferred DLL loader
// ---------------------------------------------------------------------------
class LazyLibraryLoader final {
public:
    LazyLibraryLoader(const LazyLibraryLoader&)            = delete;
    LazyLibraryLoader& operator=(const LazyLibraryLoader&) = delete;

    /// Returns the process-wide singleton.
    [[nodiscard]] static LazyLibraryLoader& Global() noexcept;

    /// Loads the DLL for target if not already loaded.
    /// Returns ALREADY_LOADED (not an error) if previously successful.
    [[nodiscard]] LazyLibStatus Load(LazyLibTarget target) noexcept;

    /// Returns the current load status without triggering a load.
    [[nodiscard]] LazyLibStatus Status(LazyLibTarget target) const noexcept;

    /// Returns the cached entry for a target (always non-null).
    [[nodiscard]] const LazyLibEntry& Entry(LazyLibTarget target) const noexcept;

    /// Resolves a proc address from an already-loaded DLL.
    /// Returns nullptr if not loaded or symbol not found.
    [[nodiscard]] void* GetProc(LazyLibTarget target, const char* procName) noexcept;

    /// Unloads a DLL (FreeLibrary). Use only during shutdown.
    void Unload(LazyLibTarget target) noexcept;

    /// Unloads all loaded DLLs. Use only during shutdown.
    void UnloadAll() noexcept;

    /// Applies configuration. Call before first Load.
    void Configure(const LazyLibConfig& cfg) noexcept;

    /// Returns the count of successfully loaded DLLs.
    [[nodiscard]] std::uint32_t LoadedCount() const noexcept;

    /// kDllNames[target] — canonical filenames for each lazy-load target.
    static constexpr const wchar_t* kDllNames[static_cast<std::size_t>(LazyLibTarget::COUNT)] = {
        L"libheif.dll",
        L"jxl.dll",
        L"avif.dll",
        L"libraw.dll",
        L"mupdf.dll",
    };

private:
    LazyLibraryLoader() noexcept = default;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_LAZYLIBLOADER_H
