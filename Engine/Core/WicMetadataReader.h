// Engine/Core/WicMetadataReader.h
// ExplorerLens — WIC IWICMetadataQueryReader wrapper for EXIF/XMP/IPTC (H44 / ROADMAP v8.0 Phase 2)
// Sprint S334.
//
// Purpose:
//   ExplorerLens currently uses custom EXIF parsing in some decoders.  This is
//   fragile, duplicates logic already in Windows, and adds attack surface.
//
//   H44 says: use WIC `IWICMetadataQueryReader` for EXIF/XMP instead of custom
//   parsing.  WIC already handles JPEG, PNG, TIFF, GIF, BMP, and HEIF metadata
//   natively — no third-party parsing library needed for standard fields.
//
//   WicMetadataReader wraps `IWICMetadataQueryReader` into a simple synchronous
//   field-query interface.  It is deliberately simple:
//     - Query standard fields by enum (WicMetaFieldId)
//     - Return typed value or empty optional
//     - Thread-safe: instances are not shared; caller constructs per-decode
//
//   Integration:
//     WicMetadataReader reader{ pWicDecoder };
//     if (auto orient = reader.ReadUint16(WicMetaFieldId::EXIF_ORIENTATION))
//         ApplyOrientation(*orient, pixels);
//
//   All WIC COM calls are _WIN32-gated for cross-platform build compatibility.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_WIC_METADATA_READER_H
#define EXPLORERLENS_ENGINE_WIC_METADATA_READER_H

#include <cstdint>
#include <string>
#include <optional>
#include <string_view>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <wincodec.h>
#endif

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// WicMetaFieldId — canonical identifiers for common image metadata fields
// ---------------------------------------------------------------------------
enum class WicMetaFieldId : std::uint32_t {
    // EXIF core
    EXIF_ORIENTATION          = 0x00010112,  ///< IFD tag 0x0112 — 1-8 rotation/flip
    EXIF_IMAGE_WIDTH          = 0x0001A002,
    EXIF_IMAGE_HEIGHT         = 0x0001A003,
    EXIF_DATETIME_ORIGINAL    = 0x00019003,
    EXIF_DATETIME_DIGITIZED   = 0x00019004,
    EXIF_MAKE                 = 0x0001010F,  ///< Camera make string
    EXIF_MODEL                = 0x00010110,  ///< Camera model string
    EXIF_ISO                  = 0x00018827,
    EXIF_EXPOSURE_TIME        = 0x0001829A,
    EXIF_FNUMBER              = 0x0001829D,
    EXIF_FOCAL_LENGTH         = 0x0001920A,
    EXIF_GPS_LATITUDE         = 0x00010002,
    EXIF_GPS_LONGITUDE        = 0x00010004,
    // XMP
    XMP_RATING                = 0x10000001,  ///< xmp:Rating (0–5 stars)
    XMP_LABEL                 = 0x10000002,  ///< xmp:Label color label
    XMP_CREATE_DATE           = 0x10000003,
    // IPTC
    IPTC_HEADLINE             = 0x20000001,
    IPTC_KEYWORDS             = 0x20000002,
    IPTC_COPYRIGHT            = 0x20000003,
};

// ---------------------------------------------------------------------------
// WicMetaReadStatus
// ---------------------------------------------------------------------------
enum class WicMetaReadStatus : std::uint8_t {
    OK              = 0,
    NOT_FOUND       = 1,  ///< Field not present in metadata block
    TYPE_MISMATCH   = 2,  ///< Field exists but type cannot be coerced
    NO_READER       = 3,  ///< WIC metadata reader unavailable (non-Windows or error)
    DISABLED        = 4,  ///< Format does not support the requested metadata block
};

// ---------------------------------------------------------------------------
// WicMetadataReader
// ---------------------------------------------------------------------------
class WicMetadataReader final {
public:
    WicMetadataReader() noexcept = default;

#ifdef _WIN32
    explicit WicMetadataReader(IWICBitmapDecoder* pDecoder) noexcept
        : m_valid(false)
    {
        if (!pDecoder) return;

        IWICBitmapFrameDecode* pFrame = nullptr;
        if (SUCCEEDED(pDecoder->GetFrame(0u, &pFrame))) {
            pFrame->GetMetadataQueryReader(&m_pReader);
            pFrame->Release();
            m_valid = (m_pReader != nullptr);
        }
    }

    ~WicMetadataReader() noexcept
    {
        if (m_pReader) { m_pReader->Release(); m_pReader = nullptr; }
    }
#else
    // Non-Windows stub constructor
    explicit WicMetadataReader(void* /*pDecoder*/) noexcept : m_valid(false) {}
    ~WicMetadataReader() noexcept = default;
#endif

    // Non-copyable (owns COM pointer)
    WicMetadataReader(const WicMetadataReader&)            = delete;
    WicMetadataReader& operator=(const WicMetadataReader&) = delete;
    WicMetadataReader(WicMetadataReader&&)                 = default;
    WicMetadataReader& operator=(WicMetadataReader&&)      = default;

    [[nodiscard]] bool IsValid() const noexcept { return m_valid; }

    // ------------------------------------------------------------------
    // ReadUint16() — read a UINT16 field (e.g., EXIF_ORIENTATION)
    // ------------------------------------------------------------------
    [[nodiscard]]
    std::optional<std::uint16_t> ReadUint16(WicMetaFieldId field) const noexcept
    {
#ifdef _WIN32
        if (!m_pReader) return std::nullopt;

        PROPVARIANT pv{};
        PropVariantInit(&pv);

        const std::wstring query = BuildQuery(field);
        if (FAILED(m_pReader->GetMetadataByName(query.c_str(), &pv)))
            return std::nullopt;

        std::optional<std::uint16_t> result;
        if (pv.vt == VT_UI2) result = pv.uiVal;
        else if (pv.vt == VT_UI4 && pv.ulVal <= 0xFFFFu)
            result = static_cast<std::uint16_t>(pv.ulVal);

        PropVariantClear(&pv);
        return result;
#else
        (void)field;
        return std::nullopt;
#endif
    }

    // ------------------------------------------------------------------
    // ReadString() — read a string field (e.g., EXIF_MAKE)
    // ------------------------------------------------------------------
    [[nodiscard]]
    std::optional<std::wstring> ReadString(WicMetaFieldId field) const noexcept
    {
#ifdef _WIN32
        if (!m_pReader) return std::nullopt;

        PROPVARIANT pv{};
        PropVariantInit(&pv);

        const std::wstring query = BuildQuery(field);
        if (FAILED(m_pReader->GetMetadataByName(query.c_str(), &pv)))
            return std::nullopt;

        std::optional<std::wstring> result;
        if (pv.vt == VT_LPWSTR && pv.pwszVal)
            result = std::wstring{ pv.pwszVal };
        else if (pv.vt == VT_BSTR && pv.bstrVal)
            result = std::wstring{ pv.bstrVal };

        PropVariantClear(&pv);
        return result;
#else
        (void)field;
        return std::nullopt;
#endif
    }

    // ------------------------------------------------------------------
    // ReadFloat() — read a rational/float field (e.g., EXIF_FNUMBER)
    // ------------------------------------------------------------------
    [[nodiscard]]
    std::optional<double> ReadFloat(WicMetaFieldId field) const noexcept
    {
#ifdef _WIN32
        if (!m_pReader) return std::nullopt;

        PROPVARIANT pv{};
        PropVariantInit(&pv);

        const std::wstring query = BuildQuery(field);
        if (FAILED(m_pReader->GetMetadataByName(query.c_str(), &pv)))
            return std::nullopt;

        std::optional<double> result;
        if      (pv.vt == VT_R8)  result = pv.dblVal;
        else if (pv.vt == VT_R4)  result = static_cast<double>(pv.fltVal);
        else if (pv.vt == VT_UI4) result = static_cast<double>(pv.ulVal);

        PropVariantClear(&pv);
        return result;
#else
        (void)field;
        return std::nullopt;
#endif
    }

    // ------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kExifOrientationTagId = 0x0112u;
    static constexpr std::uint16_t kOrientationNormal    = 1u;
    static constexpr std::uint16_t kOrientationMax       = 8u;

private:
#ifdef _WIN32
    IWICMetadataQueryReader* m_pReader{ nullptr };
#endif
    bool m_valid{ false };

#ifdef _WIN32
    static std::wstring BuildQuery(WicMetaFieldId field) noexcept
    {
        // Standard WIC query paths for EXIF
        switch (field) {
        case WicMetaFieldId::EXIF_ORIENTATION:       return L"/app1/ifd/{ushort=274}";
        case WicMetaFieldId::EXIF_MAKE:              return L"/app1/ifd/{ushort=271}";
        case WicMetaFieldId::EXIF_MODEL:             return L"/app1/ifd/{ushort=272}";
        case WicMetaFieldId::EXIF_DATETIME_ORIGINAL: return L"/app1/ifd/exif/{ushort=36867}";
        case WicMetaFieldId::EXIF_ISO:               return L"/app1/ifd/exif/{ushort=34855}";
        case WicMetaFieldId::EXIF_FNUMBER:           return L"/app1/ifd/exif/{ushort=33437}";
        case WicMetaFieldId::EXIF_FOCAL_LENGTH:      return L"/app1/ifd/exif/{ushort=37386}";
        case WicMetaFieldId::XMP_RATING:             return L"/xmp/<xmpattr>Rating";
        default:                                     return L"";
        }
    }
#endif
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_WIC_METADATA_READER_H
