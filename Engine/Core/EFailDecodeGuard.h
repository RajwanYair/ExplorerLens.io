// Engine/Core/EFailDecodeGuard.h
// ExplorerLens — COM-layer E_FAIL policy: never return S_OK with a blank bitmap (ROADMAP v8.0 Phase 2)
// Sprint S327.
//
// Purpose:
//   The current IThumbnailProvider::GetThumbnail() implementation returns S_OK
//   with a blank (1×1 white) HBITMAP when a decoder fails.  This causes
//   Explorer to cache the blank thumbnail permanently — the user sees a white
//   square that persists until they manually clear the thumbnail cache.
//
//   The correct behaviour (per Microsoft's IThumbnailProvider contract) is:
//     - Return E_FAIL (or another error HRESULT) when decode fails.
//     - Set *phbmp = nullptr and *pdwAlpha = WTSAT_UNKNOWN.
//     - Explorer will then retry on next folder refresh instead of caching.
//
//   EFailDecodeGuard provides:
//     1. A ValidateResult() check that rejects blank / near-blank bitmaps.
//     2. A CoalesceHresult() helper that maps decoder error codes to the
//        correct COM HRESULT for the shell layer.
//     3. A policy enum so the COM server can switch between STRICT (E_FAIL on
//        any decode error) and LENIENT (return placeholder on transient errors).
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_EFAIL_DECODE_GUARD_H
#define EXPLORERLENS_ENGINE_EFAIL_DECODE_GUARD_H

#include <cstdint>
#include <cstddef>
#include <span>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>  // HRESULT, S_OK, E_FAIL, E_ABORT, E_OUTOFMEMORY
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DecodeResultPolicy
// ---------------------------------------------------------------------------
enum class DecodeResultPolicy : std::uint8_t {
    /// Return E_FAIL whenever the bitmap is empty, null, or blank.
    /// Explorer retries on next access.  RECOMMENDED for production.
    STRICT_EFAIL  = 0,

    /// Return a placeholder (1×1 transparent) bitmap with S_OK on transient
    /// failures (timeout, OOM).  Return E_FAIL only on permanent errors.
    LENIENT_OK = 1,
};

// ---------------------------------------------------------------------------
// EFailValidationResult
// ---------------------------------------------------------------------------
enum class EFailValidationResult : std::uint8_t {
    VALID               = 0,  ///< Bitmap is non-null and non-blank; proceed
    NULL_BITMAP         = 1,  ///< *phbmp is null → must return E_FAIL
    BLANK_BITMAP        = 2,  ///< Bitmap is all-white or all-transparent → E_FAIL
    ZERO_DIMENSIONS     = 3,  ///< Width or height is 0 → E_FAIL
    BUDGET_EXCEEDED     = 4,  ///< Decode exceeded per-format memory budget → E_OUTOFMEMORY
    TIMEOUT_EXPIRED     = 5,  ///< DecodeTimeoutGuard fired → E_ABORT
};

// ---------------------------------------------------------------------------
// EFailDecodeGuard
// ---------------------------------------------------------------------------
class EFailDecodeGuard final {
public:
    // ------------------------------------------------------------------
    // ValidatePixels() — check raw pixel buffer before creating HBITMAP.
    //   pixels : BGRA-8888 pixel data
    //   width  : image width in pixels
    //   height : image height in pixels
    // Returns VALID if the buffer contains meaningful image data.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static EFailValidationResult ValidatePixels(
        std::span<const std::byte> pixels,
        std::uint32_t              width,
        std::uint32_t              height) noexcept
    {
        if (width == 0u || height == 0u)
            return EFailValidationResult::ZERO_DIMENSIONS;
        if (pixels.empty())
            return EFailValidationResult::NULL_BITMAP;

        const std::size_t expected = static_cast<std::size_t>(width) * height * 4u;
        if (pixels.size() < expected)
            return EFailValidationResult::NULL_BITMAP;

        // Sample up to 64 pixels to detect all-white or all-transparent content.
        const std::size_t sampleCount  = (std::min<std::size_t>)(64u, width * height);
        const std::size_t stride       = (width * height) / sampleCount;

        bool allWhite       = true;
        bool allTransparent = true;

        for (std::size_t i = 0; i < sampleCount; ++i) {
            const auto* px = reinterpret_cast<const unsigned char*>(
                pixels.data()) + (i * stride * 4u);
            const unsigned char b = px[0], g = px[1], r = px[2], a = px[3];
            if (!(r == 0xFF && g == 0xFF && b == 0xFF)) allWhite = false;
            if (a != 0x00) allTransparent = false;
            if (!allWhite && !allTransparent) break;
        }

        if (allWhite || allTransparent)
            return EFailValidationResult::BLANK_BITMAP;

        return EFailValidationResult::VALID;
    }

    // ------------------------------------------------------------------
    // CoalesceHresult() — map validation result to COM HRESULT.
    // ------------------------------------------------------------------
#ifdef _WIN32
    [[nodiscard]]
    static HRESULT CoalesceHresult(EFailValidationResult r,
                                   DecodeResultPolicy    policy) noexcept
    {
        switch (r) {
        case EFailValidationResult::VALID:
            return S_OK;
        case EFailValidationResult::TIMEOUT_EXPIRED:
            return E_ABORT;
        case EFailValidationResult::BUDGET_EXCEEDED:
            return E_OUTOFMEMORY;
        case EFailValidationResult::BLANK_BITMAP:
            return (policy == DecodeResultPolicy::LENIENT_OK) ? S_OK : E_FAIL;
        default:
            return E_FAIL;
        }
    }
#else
    // Non-Windows stub: return 0 (S_OK equivalent) for valid, -1 for errors
    [[nodiscard]]
    static int CoalesceHresult(EFailValidationResult r,
                               DecodeResultPolicy    policy) noexcept
    {
        (void)policy;
        return (r == EFailValidationResult::VALID) ? 0 : -1;
    }
#endif

    // ------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------
    /// Minimum non-blank pixel count required to consider a bitmap valid.
    static constexpr std::size_t  kMinSamplePixels  = 64u;

    /// Default policy for production COM server.
    static constexpr DecodeResultPolicy kDefaultPolicy = DecodeResultPolicy::STRICT_EFAIL;

private:
    EFailDecodeGuard() = delete;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EFAIL_DECODE_GUARD_H
