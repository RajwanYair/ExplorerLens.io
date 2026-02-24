//==============================================================================
// ExplorerLens Engine — DPX/Cineon Decoder
// Film/TV post-production formats. DPX (SMPTE 268M) and Cineon film scan.
//==============================================================================
#pragma once
#include "../Core/IThumbnailDecoder.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// DPX file header (simplified — SMPTE 268M-2003)
struct DPXHeader {
  uint32_t magic = 0; // 0x53445058 ('SDPX') or 0x58504453 ('XPDS')
  uint32_t imageOffset = 0;
  char version[8] = {};
  uint32_t fileSize = 0;
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t bitDepth = 10; // typically 10-bit log
  uint8_t channels = 3;  // RGB
  bool bigEndian = true;
  bool valid = false;
};

/// Cineon file header (Kodak)
struct CineonHeader {
  uint32_t magic = 0; // 0x802A5FD7
  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t bitDepth = 10;
  bool valid = false;
};

/// Transfer characteristic for DPX
enum class DPXTransfer : uint8_t {
  Linear,
  LogFilm, ///< Kodak Cineon logarithmic film
  LogarithmicPrintingDensity,
  ITU_R_709,
  SMPTE_ST_2084, // PQ/HDR
  Unknown
};

/// DPX/Cineon decoder
class DPXDecoder : public IThumbnailDecoder {
public:
  DPXDecoder() {
    m_name = L"DPXDecoder";
    m_extensions = {L".dpx", L".cin"};
  }

  const wchar_t *GetName() const override { return m_name.c_str(); }

  bool CanDecode(const wchar_t *filePath) override {
    if (!filePath)
      return false;
    const wchar_t *ext = wcsrchr(filePath, L'.');
    if (!ext)
      return false;
    for (auto &e : m_extensions)
      if (_wcsicmp(e.c_str(), ext) == 0)
        return true;
    return false;
  }

  DecoderInfo GetInfo() const override {
    static const wchar_t *exts[] = {L".dpx", L".cin", nullptr};
    DecoderInfo info;
    info.name = L"DPXDecoder";
    info.version = L"1.0";
    info.supportedExtensions = exts;
    info.extensionCount = 2;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
  }

  HRESULT Decode(const ThumbnailRequest & /*request*/,
                 ThumbnailResult &result) override {
    result.status = E_NOTIMPL; // DPX decode requires file I/O — stub
    return E_NOTIMPL;
  }

  const wchar_t **GetSupportedExtensions() const override {
    static const wchar_t *exts[] = {L".dpx", L".cin", nullptr};
    return exts;
  }

  uint32_t GetExtensionCount() const override { return 2; }

  bool SupportsGPU() const override { return false; }

  bool IsArchiveDecoder() const override { return false; }

  /// Check DPX magic bytes
  static bool IsDPXFile(const uint8_t *data, size_t size) {
    if (size < 4)
      return false;
    uint32_t magic =
        (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return (magic == 0x53445058 || magic == 0x58504453);
  }

  /// Check Cineon magic bytes
  static bool IsCineonFile(const uint8_t *data, size_t size) {
    if (size < 4)
      return false;
    uint32_t magic =
        (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return (magic == 0x802A5FD7);
  }

  /// DPX transfer name
  static const wchar_t *TransferName(DPXTransfer t) {
    switch (t) {
    case DPXTransfer::Linear:
      return L"Linear";
    case DPXTransfer::LogFilm:
      return L"Log Film";
    case DPXTransfer::LogarithmicPrintingDensity:
      return L"Log Print Density";
    case DPXTransfer::ITU_R_709:
      return L"ITU-R BT.709";
    case DPXTransfer::SMPTE_ST_2084:
      return L"SMPTE ST2084 (PQ)";
    default:
      return L"Unknown";
    }
  }

  static constexpr uint32_t TransferTypeCount() { return 6; }

  /// Convert 10-bit log to 8-bit linear (simplified stub: 0→0, 1023→255)
  static uint8_t LogToLinear(uint16_t logValue) {
    return static_cast<uint8_t>((static_cast<uint32_t>(logValue) * 255u) /
                                1023u);
  }

private:
  std::wstring m_name;
  std::vector<std::wstring> m_extensions;
};

/// Alias for test compatibility
using DPXCineonDecoder = DPXDecoder;

} // namespace Engine
} // namespace ExplorerLens