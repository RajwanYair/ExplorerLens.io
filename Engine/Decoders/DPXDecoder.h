//==============================================================================
// DarkThumbs Engine — Sprint 255: DPX/Cineon Decoder
// Film/TV post-production formats. DPX (SMPTE 268M) and Cineon film scan.
//==============================================================================
#pragma once
#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// DPX file header (simplified — SMPTE 268M-2003)
struct DPXHeader {
    uint32_t magic       = 0;         // 0x53445058 ('SDPX') or 0x58504453 ('XPDS')
    uint32_t imageOffset = 0;
    char     version[8]  = {};
    uint32_t fileSize    = 0;
    uint32_t width       = 0;
    uint32_t height      = 0;
    uint8_t  bitDepth    = 10;        // typically 10-bit log
    uint8_t  channels    = 3;         // RGB
    bool     bigEndian   = true;
    bool     valid       = false;
};

/// Cineon file header (Kodak)
struct CineonHeader {
    uint32_t magic       = 0;         // 0x802A5FD7
    uint32_t width       = 0;
    uint32_t height      = 0;
    uint8_t  bitDepth    = 10;
    bool     valid       = false;
};

/// Transfer characteristic for DPX
enum class DPXTransfer : uint8_t {
    Linear,
    LogarithmicPrintingDensity,
    ITU_R_709,
    SMPTE_ST_2084,      // PQ/HDR
    Unknown
};

/// DPX/Cineon decoder
class DPXDecoder : public IThumbnailDecoder {
public:
    DPXDecoder() {
        m_name = L"DPXDecoder";
        m_extensions = { L".dpx", L".cin" };
    }

    const wchar_t* GetName() const override { return m_name.c_str(); }

    bool CanDecode(const std::wstring& ext) const override {
        for (auto& e : m_extensions)
            if (e == ext) return true;
        return false;
    }

    DecoderInfo GetInfo() const override {
        DecoderInfo info;
        info.name = m_name;
        info.version = L"1.0";
        info.description = L"DPX/Cineon film format decoder (10-bit log)";
        info.extensionCount = static_cast<int>(m_extensions.size());
        return info;
    }

    ThumbnailResult Decode(const ThumbnailRequest& request, ThumbnailResult& result) override {
        result.success = false;
        result.errorMessage = L"DPX decode requires file I/O — stub";
        return result;
    }

    /// Check DPX magic bytes
    static bool IsDPXFile(const uint8_t* data, size_t size) {
        if (size < 4) return false;
        uint32_t magic = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        return (magic == 0x53445058 || magic == 0x58504453);
    }

    /// Check Cineon magic bytes
    static bool IsCineonFile(const uint8_t* data, size_t size) {
        if (size < 4) return false;
        uint32_t magic = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        return (magic == 0x802A5FD7);
    }

    /// DPX transfer name
    static const wchar_t* TransferName(DPXTransfer t) {
        switch (t) {
            case DPXTransfer::Linear:                      return L"Linear";
            case DPXTransfer::LogarithmicPrintingDensity:  return L"Log Print Density";
            case DPXTransfer::ITU_R_709:                   return L"ITU-R BT.709";
            case DPXTransfer::SMPTE_ST_2084:               return L"SMPTE ST2084 (PQ)";
            default: return L"Unknown";
        }
    }

    /// Convert 10-bit log to 8-bit linear (simplified Cineon log-to-lin)
    static uint8_t LogToLinear(uint16_t logValue) {
        float normalized = static_cast<float>(logValue) / 1023.0f;
        float lin = powf(10.0f, (normalized - 0.685f) * 2.046f);
        lin = (lin < 0.0f) ? 0.0f : (lin > 1.0f) ? 1.0f : lin;
        return static_cast<uint8_t>(lin * 255.0f);
    }

private:
    std::wstring m_name;
    std::vector<std::wstring> m_extensions;
};

}} // namespace DarkThumbs::Engine
