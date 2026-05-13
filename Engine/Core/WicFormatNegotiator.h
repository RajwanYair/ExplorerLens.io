// Engine/Core/WicFormatNegotiator.h
#pragma once

// WicFormatNegotiator — WIC encoder/decoder capability query (S370)
//
// Implements H24: "Use WIC codec passthrough first. Use Windows Imaging
// Component for all natively-supported formats; custom decoders only for
// formats WIC cannot handle." (ROADMAP v8.0 §5, Phase 2).
//
// WicFormatNegotiator queries the WIC component registry at runtime to
// determine which file extensions and MIME types have registered WIC decoders,
// and whether a WIC encoder can produce thumbnails for a given format.
//
// Relation to WicPassthroughSelector.h (S324):
//   WicPassthroughSelector — decides at decode time whether to use WIC vs custom.
//   WicFormatNegotiator   — inventories what WIC can handle (capability discovery).
//
// Thread safety: capabilities are cached after first query; thread-safe reads.
//
// ROADMAP ref: H24 Phase 2 — WIC passthrough first
//
// Usage:
//   bool canDecode = WicFormatNegotiator::CanDecode(L"tiff");
//   bool canEncode = WicFormatNegotiator::CanEncode(L"png");
//   auto caps = WicFormatNegotiator::QueryCaps(L"heic");

#ifndef EXPLORERLENS_ENGINE_WICFORMATNEGOTIATOR_H
#define EXPLORERLENS_ENGINE_WICFORMATNEGOTIATOR_H

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// WicNegotiatorStatus
// ---------------------------------------------------------------------------
enum class WicNegotiatorStatus : std::uint8_t {
    OK                     = 0,
    NOT_WIN32              = 1,  ///< Non-Windows stub
    WIC_NOT_AVAILABLE      = 2,  ///< CoCreateInstance(IWICImagingFactory) failed
    FORMAT_NOT_SUPPORTED   = 3,  ///< No WIC decoder registered for this extension
    MIME_NOT_RESOLVED      = 4,  ///< Could not resolve extension to MIME type
    QUERY_FAILED           = 5,  ///< IWICImagingFactory::CreateDecoder failed
};

// ---------------------------------------------------------------------------
// WicCodecCapabilities — what WIC can do with a specific format
// ---------------------------------------------------------------------------
struct WicCodecCapabilities {
    bool          canDecode          = false; ///< WIC decoder registered
    bool          canEncode          = false; ///< WIC encoder registered
    bool          hasMetadataReader  = false; ///< IWICMetadataQueryReader support
    bool          hasThumbnailReader = false; ///< IWICBitmapDecoder::GetThumbnail
    bool          supportsStreaming  = false; ///< Progressive / streaming decode
    std::uint32_t decoderCount       = 0u;   ///< Registered decoders for this MIME
    std::wstring  mimeType;                  ///< e.g. L"image/tiff"
    std::wstring  friendlyName;              ///< e.g. L"TIFF Decoder"

    [[nodiscard]] bool IsPassthroughCandidate() const noexcept {
        // WIC passthrough is worthwhile only if decode + thumbnail are both available.
        return canDecode && hasThumbnailReader;
    }
};

// ---------------------------------------------------------------------------
// WicFormatEntry — a format registered in the WIC component registry
// ---------------------------------------------------------------------------
struct WicFormatEntry {
    std::wstring           extension;   ///< Lower-case without dot (e.g. L"jpeg")
    WicCodecCapabilities   caps;
};

// ---------------------------------------------------------------------------
// WicFormatNegotiator — static WIC capability query (results are cached)
// ---------------------------------------------------------------------------
class WicFormatNegotiator final {
public:
    WicFormatNegotiator()                                          = delete;
    WicFormatNegotiator(const WicFormatNegotiator&)                = delete;
    WicFormatNegotiator& operator=(const WicFormatNegotiator&)     = delete;

    /// Returns true if WIC has a registered decoder for ext (lower-case, no dot).
    [[nodiscard]] static bool CanDecode(const wchar_t* ext) noexcept;

    /// Returns true if WIC has a registered encoder for ext.
    [[nodiscard]] static bool CanEncode(const wchar_t* ext) noexcept;

    /// Returns the full capability record for ext.
    [[nodiscard]] static WicCodecCapabilities QueryCaps(const wchar_t* ext) noexcept;

    /// Enumerates all formats with WIC decoder support.
    /// Results are cached after the first call.
    [[nodiscard]] static const std::vector<WicFormatEntry>& AllDecodableFormats() noexcept;

    /// Returns the count of formats WIC can decode on this machine.
    [[nodiscard]] static std::uint32_t DecodableFormatCount() noexcept;

    /// Clears the internal capability cache (forces re-query on next call).
    static void InvalidateCache() noexcept;

    /// Returns true if the WIC component registry has been enumerated.
    [[nodiscard]] static bool IsCachePopulated() noexcept;

    // Known-passthrough extension list (formats where WIC is always preferred).
    // These are statically known — no registry query needed.
    static constexpr const wchar_t* kAlwaysPassthrough[] = {
        L"bmp", L"gif", L"ico", L"jpeg", L"jpg", L"png", L"tiff", L"tif",
        L"wmp",  // Windows Media Photo / HD Photo
    };
    static constexpr std::uint32_t kAlwaysPassthroughCount = 9u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WICFORMATNEGOTIATOR_H
