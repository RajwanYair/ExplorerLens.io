// Engine/Pipeline/FallbackChainManager.h
// ExplorerLens — Graceful decode fallback chain (H20 + H37 / ROADMAP v8.0 Phase 2)
// Sprint S333.
//
// Purpose:
//   Today, when ExplorerLens's custom decoder fails, Explorer receives S_OK with
//   a blank white HBITMAP.  The user sees a white square.  There is no recovery.
//
//   The fallback chain (H20 + H37) defines a priority-ordered list of decode
//   strategies to attempt before returning E_FAIL:
//
//     1. ExplorerLens custom decoder       — preferred (best quality)
//     2. WIC native codec                  — for BMP/GIF/TIFF/ICO/WDP/DDS
//     3. Windows built-in thumbnail cache  — delegate to Shell PIDL thumbnail
//     4. Format-specific generic icon      — pre-rendered 256×256 per file class
//     5. Generic file icon                 — last resort; known format, decode failed
//
//   Stage transitions fire only when the previous stage fails (returns E_FAIL or
//   throws).  The manager logs each stage transition via DecodeErrorTracker so that
//   failure frequency drives decoder prioritization.
//
//   Integration:
//     FallbackChainManager chain{};
//     chain.SetCustomResult(hbmpOrNull, hr);   // stage 1 outcome
//     auto [hbmp, finalHr] = chain.Resolve(filePath, cx);
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_FALLBACK_CHAIN_MANAGER_H
#define EXPLORERLENS_ENGINE_FALLBACK_CHAIN_MANAGER_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <atomic>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// FallbackStageId — which stage ultimately produced the bitmap
// ---------------------------------------------------------------------------
enum class FallbackStageId : std::uint8_t {
    CUSTOM_DECODER   = 0,  ///< ExplorerLens custom decoder succeeded
    WIC_CODEC        = 1,  ///< Windows Imaging Component native codec
    WINDOWS_CACHE    = 2,  ///< Delegated to Windows built-in thumbnail cache
    GENERIC_ICON     = 3,  ///< Format-specific pre-rendered icon
    FILE_ICON        = 4,  ///< Generic file-type icon (last resort)
    ALL_FAILED       = 5,  ///< All stages failed — E_FAIL returned to Explorer
};

// ---------------------------------------------------------------------------
// FallbackChainConfig
// ---------------------------------------------------------------------------
struct FallbackChainConfig final {
    bool enableWicFallback      = true;   ///< Allow Stage 2 (WIC)
    bool enableWindowsCache     = true;   ///< Allow Stage 3 (Windows cache)
    bool enableGenericIcon      = true;   ///< Allow Stage 4 (format icon)
    bool enableFileIcon         = true;   ///< Allow Stage 5 (generic icon)
    bool logEachStage           = true;   ///< ETW event on every stage transition
};

// ---------------------------------------------------------------------------
// FallbackChainResult
// ---------------------------------------------------------------------------
struct FallbackChainResult final {
    FallbackStageId stage{ FallbackStageId::ALL_FAILED };
    bool            succeeded{ false };
#ifdef _WIN32
    HRESULT         hr{ E_FAIL };
    HBITMAP         hbmp{ nullptr };
#else
    int             hr{ -1 };      // Stub for non-Windows builds
    void*           hbmp{ nullptr };
#endif
};

// ---------------------------------------------------------------------------
// FallbackChainManager
// ---------------------------------------------------------------------------
class FallbackChainManager final {
public:
    explicit FallbackChainManager(FallbackChainConfig cfg = {}) noexcept
        : m_cfg(cfg) {}

    // Non-copyable
    FallbackChainManager(const FallbackChainManager&)            = delete;
    FallbackChainManager& operator=(const FallbackChainManager&) = delete;

    // ------------------------------------------------------------------
    // SetCustomResult() — record the outcome of the custom decoder (Stage 1).
    // Must be called before Resolve().
    // ------------------------------------------------------------------
#ifdef _WIN32
    void SetCustomResult(HBITMAP hbmp, HRESULT hr) noexcept
    {
        m_customHbmp = hbmp;
        m_customHr   = hr;
        m_customSet  = true;
    }
#else
    void SetCustomResult(void* hbmp, int hr) noexcept
    {
        m_customHbmp = hbmp;
        m_customHr   = hr;
        m_customSet  = true;
    }
#endif

    // ------------------------------------------------------------------
    // Resolve() — walk the fallback chain and return the best bitmap.
    //   filePath  — absolute path (for Windows cache delegation)
    //   cx        — requested thumbnail size in pixels
    // ------------------------------------------------------------------
    [[nodiscard]]
    FallbackChainResult Resolve(std::wstring_view filePath,
                                std::uint32_t     cx) noexcept
    {
        FallbackChainResult result{};

        // Stage 1: custom decoder
#ifdef _WIN32
        if (m_customSet && m_customHbmp != nullptr && SUCCEEDED(m_customHr)) {
#else
        if (m_customSet && m_customHbmp != nullptr && m_customHr == 0) {
#endif
            result.stage     = FallbackStageId::CUSTOM_DECODER;
            result.succeeded = true;
            result.hbmp      = m_customHbmp;
            result.hr        = m_customHr;
            m_stageHits[0].fetch_add(1u, std::memory_order_relaxed);
            return result;
        }

        // Stage 2: WIC codec (stub — wired to IWICImagingFactory in Phase 3)
        if (m_cfg.enableWicFallback) {
            m_stageAttempts[1].fetch_add(1u, std::memory_order_relaxed);
            // Phase 3: invoke WIC for BMP/GIF/TIFF/ICO etc.
            // For Phase 2: fall through
        }

        // Stage 3: Windows built-in thumbnail cache
        // (stub — Phase 3 will call SHGetImageList + IExtractImage)
        if (m_cfg.enableWindowsCache && !filePath.empty()) {
            m_stageAttempts[2].fetch_add(1u, std::memory_order_relaxed);
            (void)cx;
            // Phase 3: IShellFolder + IExtractImage delegation
        }

        // Stage 4: format-specific generic icon (stub)
        if (m_cfg.enableGenericIcon) {
            m_stageAttempts[3].fetch_add(1u, std::memory_order_relaxed);
            result.stage     = FallbackStageId::GENERIC_ICON;
            result.succeeded = false;  // icon not yet rendered in Phase 2
            result.hbmp      = nullptr;
        }

        // Stage 5: generic file icon (last resort)
        if (m_cfg.enableFileIcon) {
            m_stageAttempts[4].fetch_add(1u, std::memory_order_relaxed);
            result.stage = FallbackStageId::FILE_ICON;
            // Phase 3: SHGetFileInfo(SHGFI_ICON) → CreateBitmap
        }

#ifdef _WIN32
        result.hr = E_FAIL;
#else
        result.hr = -1;
#endif
        m_allFailedCount.fetch_add(1u, std::memory_order_relaxed);
        return result;
    }

    // ------------------------------------------------------------------
    // Diagnostics
    // ------------------------------------------------------------------
    [[nodiscard]] std::uint64_t StageAttempts(FallbackStageId s) const noexcept
    {
        const auto idx = static_cast<std::size_t>(s);
        if (idx >= kStageCount) return 0u;
        return m_stageAttempts[idx].load(std::memory_order_relaxed);
    }

    [[nodiscard]] std::uint64_t StageHits(FallbackStageId s) const noexcept
    {
        const auto idx = static_cast<std::size_t>(s);
        if (idx >= kStageCount) return 0u;
        return m_stageHits[idx].load(std::memory_order_relaxed);
    }

    [[nodiscard]] std::uint64_t AllFailedCount() const noexcept
    { return m_allFailedCount.load(std::memory_order_relaxed); }

    static constexpr std::size_t kStageCount = 5u;

private:
#ifdef _WIN32
    HBITMAP m_customHbmp{ nullptr };
    HRESULT m_customHr{ E_FAIL };
#else
    void*   m_customHbmp{ nullptr };
    int     m_customHr{ -1 };
#endif
    bool    m_customSet{ false };
    FallbackChainConfig m_cfg{};

    std::atomic<std::uint64_t> m_stageAttempts[kStageCount]{};
    std::atomic<std::uint64_t> m_stageHits[kStageCount]{};
    std::atomic<std::uint64_t> m_allFailedCount{ 0u };
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_FALLBACK_CHAIN_MANAGER_H
